// SPDX-License-Identifier: GPL-2.0-or-later
// 위캣4 ↔ 제조사 무선 모듈 라이브러리(libmodule.a) 연결부 — 원 펌웨어 기능 재현
// 동작 기준: 제품 설명서의 모드·Fn·절전 동작
//
// 모드: 3단 스위치(정면 기준 왼쪽 BT / 가운데 유선 / 오른쪽 2.4G)가 결정. 케이블 연결로 자동 전환하지 않음.
// 커스텀 키(vial.json customKeycodes 순서 = QK_KB_n):
//   0 BT1 / 1 BT2 / 2 BT3 : BT 모드에서 누르면 즉시 채널 전환, 3초 누르고 있으면 그 채널 새 페어링 (원: Fn+A/S/D)
//   3 BT4 / 4 BT5         : 사용 안 함(설명서상 BT 3채널)
//   5 2.4G                : 2.4G 모드에서 3초 누르고 있으면 동글 페어링, 짧게=재연결 (원: Fn+F 길게)
//   6 USB                 : 사용 안 함(스위치 가운데가 유선)
//   7 BAT                 : 무선 배터리 잔량을 Q~P 키 불빛으로 3초 표시            (원: Fn+Backspace)
//   8 CHRG                : 충전 상태 3초 표시(빨강 충전 중 / 초록 완료)         (원: Fn+오른쪽 Space)
//   9 SLEEP               : 초절전(무선: 즉시 절전, 유선: 조명 끔)                (원: Fn+Enter)
//  10 RESET               : 3초 누르고 있으면 공장 초기화(EEPROM 초기화 후 재시작) (원: Fn+L_Ctrl 3초)
//  11 RAINBOW             : 조명을 켜고 Cycle Left/Right 효과로 바로 전환(효과는 EEPROM에 저장)
#include QMK_KEYBOARD_H
#include "wireless.h"

// ---- 하드웨어 핀 ----
// 모드 스위치 C14/C15 는 원 펌웨어의 핀 설정과 같다. 충전 완료 핀 A15 는 같은 ODM 보드(dr6p)의 값을 따른 것이다.
#ifndef WLS_MODE_PIN_A
#    define WLS_MODE_PIN_A C14 // 스위치 핀1
#endif
#ifndef WLS_MODE_PIN_B
#    define WLS_MODE_PIN_B C15 // 스위치 핀2
#endif
// 핀 조합(A,B): BT=0,1 / 2.4G=1,0 / USB=1,1
#ifndef CHRG_FULL_PIN
#    define CHRG_FULL_PIN A15 // 충전 완료 (dr6p BAT_FULL_PIN, 완충 시 High)
#endif

#ifndef LOWBAT_PERCENT
#    define LOWBAT_PERCENT 20
#endif
// 저배터리 깜박임 위치 = 원 펌웨어의 Fn 자리(제조사 기본 키맵 MO(1) = 행렬 3,5)
#ifndef LOWBAT_ROW
#    define LOWBAT_ROW 3
#endif
#ifndef LOWBAT_COL
#    define LOWBAT_COL 5
#endif

enum { MODE_NONE = 0, MODE_BT, MODE_2G4, MODE_USB };

static wireless_devs_t devs = {.now = DEVS_USB, .bt = DEVS_BT1, .wl = DEVS_2G4};
static uint8_t         cur_mode;
static uint16_t        press_time;
static int8_t          held_k = -1; // 길게 누르기 판정 중인 커스텀 키(QK_KB_n 의 n), 없으면 -1
static bool            long_done;
static uint32_t        show_bat_until, show_chrg_until;

// EEPROM kb 영역 1바이트 = 마지막 BT 채널
static uint8_t load_last_bt(void) {
    uint8_t v = 0;
    eeconfig_read_kb_datablock(&v, 0, 1);
    return (v >= DEVS_BT1 && v <= DEVS_BT3) ? v : DEVS_BT1;
}
static void save_last_bt(uint8_t v) {
    if (v >= DEVS_BT1 && v <= DEVS_BT3) eeconfig_update_kb_datablock(&v, 0, 1);
}

static uint8_t read_switch(void) {
    uint8_t a = gpio_read_pin(WLS_MODE_PIN_A), b = gpio_read_pin(WLS_MODE_PIN_B);
    if (a && b) return MODE_USB;
    if (!a && b) return MODE_BT;
    if (a && !b) return MODE_2G4;
    return MODE_NONE;
}

// 무선 모듈은 NKRO 보고서를 받아 주지 않는다(연결은 되지만 키가 전달되지 않음).
// 무선에서는 6키 보고서로 보내고, 유선으로 돌아오면 NKRO 를 다시 켠다. EEPROM 에는 쓰지 않는다.
static void set_nkro_for_mode(uint8_t mode) {
    bool want = (mode == MODE_USB);
    if (keymap_config.nkro != want) {
        clear_keyboard();
        keymap_config.nkro = want;
    }
}

// 유선→무선 전환 직후 보낸 채널 전환 명령을 무선 칩이 놓치는 일이 있다(같은 채널 키를 다시 누르면 붙음).
// 무선 모드로 들어오거나 절전에서 깬 뒤 1초가 지나도 연결·페어링 중이 아니면 같은 채널로 다시 전환 명령을 보낸다.
// 1초·3초 뒤 최대 2번만 — 상대 컴퓨터가 꺼져 있을 때 계속 보내지 않는다.
static uint32_t reconn_at;
static uint8_t  reconn_left;

static void arm_reconnect(void) {
    reconn_left = (cur_mode == MODE_BT || cur_mode == MODE_2G4) ? 2 : 0;
    reconn_at   = timer_read32() + 1000;
}

static void reconnect_task(void) {
    if (!reconn_left || (int32_t)(timer_read32() - reconn_at) < 0) return;
    uint8_t now   = wireless_get_devs().now;
    uint8_t state = *md_getp_state();
    if (now == DEVS_USB || state == MD_STATE_CONNECTED || state == MD_STATE_PAIRING) {
        reconn_left = 0;
        return;
    }
    if (lpwr_get_state() != LPWR_NORMAL) return; // 절전 중에는 깨어난 뒤 다시 판단
    wireless_devs_change(now, now, false);
    reconn_left--;
    reconn_at = timer_read32() + 2000;
}

static void apply_mode(uint8_t mode) {
    uint8_t now = wireless_get_devs().now;
    if (mode != MODE_NONE) set_nkro_for_mode(mode);
    switch (mode) {
        case MODE_USB: wireless_devs_change(now, DEVS_USB, false); break;
        case MODE_BT: wireless_devs_change(now, load_last_bt(), false); break;
        case MODE_2G4: wireless_devs_change(now, DEVS_2G4, false); break;
        default: break;
    }
    cur_mode = mode;
    arm_reconnect();
}

uint8_t wecat4_cur_mode(void) {
    return cur_mode;
}

void keyboard_post_init_kb(void) {
    gpio_set_pin_input_high(WLS_MODE_PIN_A);
    gpio_set_pin_input_high(WLS_MODE_PIN_B);
    gpio_set_pin_input(CHRG_FULL_PIN);

    wireless_init(&devs);
    md_sleep_bt(true);  // 모듈 쪽 무선 수면 타이머 사용(설명서: 30분 미사용 수면) — 인자 의미는 라이브러리 문서가 없어 확인하지 못함
    md_sleep_2g4(true);

    // 스위치 위치는 여기서 적용하지 않는다. keyboard_init() 다음에 protocol_post_init()이 호스트 드라이버를
    // USB로 덮어쓰므로, 여기서 무선으로 바꾸면 BT/2.4G 위치로 켜졌을 때 키 입력이 USB로 나간다.
    // 첫 적용은 wireless_pre_task()에서 한다(cur_mode 가 MODE_NONE 이므로 50ms 간격 2회 일치 시 바로 적용).

    keyboard_post_init_user();
}

// 제조사 wireless.c의 housekeeping_task_kb → wireless_task → 이 훅을 매 주기 호출
// 3초 이상 누르고 있으면 떼기 전에 실행: BT1~3 = 그 채널 새 페어링, 2.4G = 동글 페어링, RESET = 공장 초기화
static void long_press_task(void) {
    if (held_k < 0 || long_done || timer_elapsed(press_time) < 3000) return;
    long_done   = true;
    uint8_t now = wireless_get_devs().now;
    switch (held_k) {
        case 0:
        case 1:
        case 2:
            if (cur_mode == MODE_BT) wireless_devs_change(now, DEVS_BT1 + held_k, true);
            break;
        case 5:
            if (cur_mode == MODE_2G4) wireless_devs_change(now, DEVS_2G4, true);
            break;
        case 10:
            eeconfig_disable();
            soft_reset_keyboard();
            break;
        default:
            break;
    }
}

// 무선 연결 표시 상태. 무선 칩 상태만 보고 판단하므로 스위치 전환·채널 전환·절전 복귀·사용 중 끊김을 같은 규칙으로 다룬다.
//   연결 안 됨 10초 미만 = 연결 시도 표시, 10초 이상 = 경고 표시(연결되거나 유선으로 바꿀 때까지),
//   연결되는 순간 = 대상 채널 키 1초 표시 후 평소 조명.
#define CONN_WARN_MS 10000
#define CONN_DONE_MS 1000
static uint32_t disc_since; // 0 = 연결됨(또는 유선)
static uint32_t done_until; // 연결 직후 표시 종료 시각

static void conn_track(void) {
    uint8_t  now   = wireless_get_devs().now;
    uint8_t  state = *md_getp_state();
    uint32_t t     = timer_read32();
    if (now == DEVS_USB) {
        disc_since = 0;
        done_until = 0;
    } else if (state == MD_STATE_CONNECTED) {
        if (disc_since) done_until = t + CONN_DONE_MS;
        disc_since = 0;
    } else if (!disc_since) {
        disc_since = t ? t : 1;
        done_until = 0;
    }
}

void wireless_pre_task(void) {
    static uint32_t t;
    static uint8_t  last;
    long_press_task();
    reconnect_task();
    conn_track();
    if (timer_elapsed32(t) < 50) return;
    t = timer_read32();
    uint8_t m = read_switch();
    if (m != MODE_NONE && m == last && m != cur_mode) apply_mode(m); // 50ms 간격 2회 일치로 채터링 방지
    last = m;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {
    save_last_bt(new_devs);
}

// 절전 진입 때 연결이 끊겨 있으면 제조사 코드(lpwr_stop_cb)가 무선 칩을 유선 모드로 돌려놓고, 깰 때 되돌리지 않는다.
// 깬 뒤에도 같은 재연결 확인을 건다.
void lpwr_wakeup_hook(void) {
    arm_reconnect();
}

// 유선 모드에선 절전(수면) 진입 안 함 — 무선 모드에서만 LPWR_TIMEOUT(30분) 적용
bool lpwr_is_allow_timeout_hook(void) {
    return wireless_get_devs().now != DEVS_USB;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode < QK_KB_0 || keycode > QK_KB_11) return process_record_user(keycode, record);

    uint8_t k = keycode - QK_KB_0;
    if (record->event.pressed) {
        press_time = timer_read();
        if (k <= 2 || k == 5 || k == 10) {
            held_k    = k;
            long_done = false;
        }
        // 채널 전환은 누르는 즉시. 같은 채널이어도 무선 칩에 전환 명령을 다시 보내 재연결을 시도한다
        if (k <= 2 && cur_mode == MODE_BT) {
            reconn_left = 0; // 직접 고른 채널에 막 붙은 연결을 남아 있던 재시도가 끊지 않게 한다
            wireless_devs_change(wireless_get_devs().now, DEVS_BT1 + k, false);
        }
        if (k == 11) {
            rgb_matrix_enable_noeeprom();
            rgb_matrix_mode(RGB_MATRIX_CYCLE_LEFT_RIGHT);
        }
        if (k == 7) show_bat_until = timer_read32() + 3000;
        if (k == 8) show_chrg_until = timer_read32() + 3000;
        if (k == 9) {
            if (wireless_get_devs().now == DEVS_USB) {
                rgb_matrix_disable_noeeprom();
            } else {
                lpwr_set_manual_timeout(true);
            }
        }
        return false;
    }

    // 뗄 때: 2.4G 를 짧게 눌렀으면 재연결, 길게 누르기 판정 종료
    if (k == 5 && !long_done && cur_mode == MODE_2G4) {
        reconn_left = 0;
        wireless_devs_change(wireless_get_devs().now, DEVS_2G4, false);
    }
    if (held_k == k) held_k = -1;
    return false;
}

// ---- 표시 ----
static bool blink(uint16_t period) {
    return (timer_read32() / period) % 2;
}
static void set_rc(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t i = g_led_config.matrix_co[row][col];
    if (i != NO_LED) rgb_matrix_set_color(i, r, g, b);
}

static void channel_led(uint8_t devs, bool on) {
    uint8_t v = on ? 255 : 0;
    switch (devs) {
        case DEVS_BT1: set_rc(1, 1, v, 0, 0); break;
        case DEVS_BT2: set_rc(1, 2, 0, v, 0); break;
        case DEVS_BT3: set_rc(1, 3, 0, 0, v); break;
        case DEVS_2G4: set_rc(1, 4, 0, v, v); break;
        default: break;
    }
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) return false;

    uint8_t now = wireless_get_devs().now;

    // 배터리 잔량: 다른 불 끄고 Q(0,1)~P(0,10) 중 10%당 1키 초록
    if (timer_read32() < show_bat_until) {
        uint8_t n = (*md_getp_bat() + 9) / 10;
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        for (uint8_t c = 1; c <= 10 && c <= n; c++) set_rc(0, c, 0, 200, 0);
        return false;
    }
    // 충전 상태: 전체를 빨강(충전 중) / 초록(완충)
    if (timer_read32() < show_chrg_until) {
        bool full = gpio_read_pin(CHRG_FULL_PIN);
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, full ? 0 : 200, full ? 200 : 0, 0);
        return false;
    }
    // 채널 키 색: A(BT1)빨강·S(BT2)초록·D(BT3)파랑·F(2.4G)청록
    // 흐르는 효과에 묻히지 않게 다른 불은 모두 끄고 채널 키만 표시한다.
    //   누르는 중(3초 전) = 켜 둠 / 3초 지나 새 페어링 = 손을 뗄 때까지·페어링 중인 동안 빠르게 깜박임.
    // 페어링이 곧바로 끝나도(호스트가 즉시 붙음) 손을 떼기 전까지는 깜박임이 보인다.
    bool    chan_key = held_k >= 0 && (held_k <= 2 || held_k == 5);
    uint8_t held_dev = held_k == 5 ? DEVS_2G4 : DEVS_BT1 + held_k;
    bool    pairing  = now != DEVS_USB && *md_getp_state() == MD_STATE_PAIRING;
    if (chan_key || pairing) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        if (chan_key && !long_done) {
            channel_led(held_dev, true);
        } else {
            channel_led(chan_key ? held_dev : now, blink(100));
        }
    } else if (now != DEVS_USB && disc_since) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        if (timer_elapsed32(disc_since) < CONN_WARN_MS) {
            // 연결 시도 중: BT 는 A·S·D 를 켜고 대상 채널만 깜박임, 2.4G 는 F 만 깜박임
            if (now != DEVS_2G4) {
                for (uint8_t d = DEVS_BT1; d <= DEVS_BT3; d++) {
                    if (d != now) channel_led(d, true);
                }
            }
            channel_led(now, blink(250));
        } else {
            // 경고: 연결 실패. 전체 어두운 빨강 1초 간격 깜박임, 대상 채널 키만 채널 색으로 켜 둠
            if (blink(1000)) {
                for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 80, 0, 0);
            }
            channel_led(now, true);
        }
    } else if (now != DEVS_USB && timer_read32() < done_until) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        channel_led(now, true);
    }
    // 저배터리: Fn 자리 빨강 연속 깜박임(무선 모드)
    if (now != DEVS_USB && *md_getp_bat() <= LOWBAT_PERCENT && blink(500)) {
        set_rc(LOWBAT_ROW, LOWBAT_COL, 255, 0, 0);
    }
    return true;
}

// 제조사 QMK는 keyboard.c의 matrix_previous를 전역으로 바꿔 두었고 lowpower.c가 깨어날 때 이를 0으로 지운다.
// vial-qmk 본체는 static이라 링크가 안 되므로, 이름만 같은 더미를 둔다(그 memset은 효과 없음 = 절전 복귀 직후
// 눌려 있던 키가 다시 눌림으로 인식되지 않을 수 있음). 본체(keyboard.c)는 수정하지 않는다.
matrix_row_t matrix_previous[MATRIX_ROWS];
