// SPDX-License-Identifier: GPL-2.0-or-later
// 제조사 무선 모듈(libmodule.a) 연동 및 인디케이터 제어
// - 3단 스위치: BT / 유선 / 2.4G
// - QK_KB_0~2: BT1~3 (탭: 전환, 3초: 페어링)
// - QK_KB_5: 2.4G 페어링, QK_KB_7: 배터리 잔량, QK_KB_8: 충전 상태
// - QK_KB_9: 초절전, QK_KB_10: 공장 초기화(3초), QK_KB_11: 레인보우 효과
#include QMK_KEYBOARD_H
#include "wireless.h"
#include "usb_main.h"

// 하드웨어 모드 스위치 핀 (BT=0,1 / 2.4G=1,0 / USB=1,1)
#ifndef WLS_MODE_PIN_A
#    define WLS_MODE_PIN_A C14
#endif
#ifndef WLS_MODE_PIN_B
#    define WLS_MODE_PIN_B C15
#endif
#ifndef CHRG_FULL_PIN
#    define CHRG_FULL_PIN A15 // 완충 시 High
#endif

#ifndef LOWBAT_PERCENT
#    define LOWBAT_PERCENT 20
#endif
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
static int8_t          held_k = -1; // 누름 판정 중인 QK_KB_n의 n (-1: 없음)
static bool            long_done;
static uint32_t        show_bat_until, show_chrg_until;
// 배터리 잔량 표본 이동평균 필터
#define BAT_SAMPLE_MS 3000
#define BAT_AVG_N 8
static uint8_t  bat_hist[BAT_AVG_N], bat_hist_n, bat_hist_i;
static uint8_t  bat_avg, bat_last_raw, bat_valid_cnt, bat_sample_cnt;
static uint8_t  bat_used_n;
static uint32_t bat_sample_ms, bat_valid_ms;

static bool bat_known(void) {
    return bat_hist_n > 0;
}

static void bat_reset(void) {
    bat_hist_n = bat_hist_i = 0;
    bat_avg                 = 0;
    bat_used_n              = 0;
}

static void bat_track(void) {
    if (get_transport() == TRANSPORT_USB || lpwr_get_state() != LPWR_NORMAL) return;
    if (timer_elapsed32(bat_sample_ms) < BAT_SAMPLE_MS) return;
    bat_sample_ms = timer_read32();
    uint8_t raw   = *md_getp_bat();
    bat_last_raw  = raw;
    if (bat_sample_cnt < 255) bat_sample_cnt++;
    if (raw == 0 || raw > 100) return;
    if (bat_valid_cnt < 255) bat_valid_cnt++;
    bat_valid_ms         = bat_sample_ms;
    bat_hist[bat_hist_i] = raw;
    bat_hist_i           = (bat_hist_i + 1) % BAT_AVG_N;
    if (bat_hist_n < BAT_AVG_N) bat_hist_n++;
    // 최근 값의 중앙값에서 15 넘게 벗어난 값은 평균에서 뺀다. 값이 실제로 바뀌면 여러 개가 따라와 중앙값이 옮겨 가므로 반영된다.
    uint8_t sorted[BAT_AVG_N];
    for (uint8_t i = 0; i < bat_hist_n; i++) {
        uint8_t v = bat_hist[i], j = i;
        while (j > 0 && sorted[j - 1] > v) { sorted[j] = sorted[j - 1]; j--; }
        sorted[j] = v;
    }
    uint8_t  med = sorted[bat_hist_n / 2];
    uint16_t sum = 0;
    uint8_t  used = 0;
    for (uint8_t i = 0; i < bat_hist_n; i++) {
        uint8_t d = bat_hist[i] > med ? bat_hist[i] - med : med - bat_hist[i];
        if (d <= 15) { sum += bat_hist[i]; used++; }
    }
    bat_used_n = used;
    bat_avg    = used ? sum / used : med;
}

// 2.4G 동글 접속 대기 루프 (PAIR_WINDOW_MS 동안 주기적 재전송)
#define PAIR_WINDOW_MS 60000
#define PAIR_RETRY_MS 1500
static uint32_t pair_until;
static uint32_t pair_next;

// EEPROM 마지막 BT 채널 로드/저장
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

// 무선 모듈 호환: 무선은 6KRO, 유선은 NKRO 적용
static void set_nkro_for_mode(uint8_t mode) {
    bool want = (mode == MODE_USB);
    if (keymap_config.nkro != want) {
        clear_keyboard();
        keymap_config.nkro = want;
    }
}

// 무선 전환/복귀 직후 채널 재전송 태스크
static uint32_t reconn_at;
static uint8_t  reconn_left;

static void arm_reconnect(void) {
    reconn_left = (cur_mode == MODE_BT || cur_mode == MODE_2G4) ? 2 : 0;
    reconn_at   = timer_read32() + 1000;
}

static void reconnect_task(void) {
    if (!reconn_left || pair_until || (int32_t)(timer_read32() - reconn_at) < 0) return;
    uint8_t now   = wireless_get_devs().now;
    uint8_t state = *md_getp_state();
    if (now == DEVS_USB || state == MD_STATE_CONNECTED || state == MD_STATE_PAIRING) {
        reconn_left = 0;
        return;
    }
    if (lpwr_get_state() != LPWR_NORMAL) return;
    wireless_devs_change(now, now, false);
    reconn_left--;
    reconn_at = timer_read32() + 2000;
}

static void pair_start(void) {
    reconn_left = 0;
    pair_until  = timer_read32() + PAIR_WINDOW_MS;
    pair_next   = timer_read32() + PAIR_RETRY_MS;
    wireless_devs_change(wireless_get_devs().now, DEVS_2G4, true);
}

static void pair_task(void) {
    if (!pair_until) return;
    uint32_t t = timer_read32();
    if (cur_mode != MODE_2G4 || *md_getp_state() == MD_STATE_CONNECTED || (int32_t)(t - pair_until) >= 0) {
        pair_until = 0;
        return;
    }
    if ((int32_t)(t - pair_next) < 0 || lpwr_get_state() != LPWR_NORMAL) return;
    wireless_devs_change(wireless_get_devs().now, DEVS_2G4, true);
    pair_next = t + PAIR_RETRY_MS;
}

static void apply_mode(uint8_t mode) {
    uint8_t now = wireless_get_devs().now;
    if (mode != MODE_NONE) set_nkro_for_mode(mode);
    switch (mode) {
        case MODE_USB:
            // 유선 모드 PB1 LOW
            gpio_write_pin_low(B1);
            wireless_devs_change(now, DEVS_USB, false);
            break;
        case MODE_BT:
            // 무선 모드 PB1 HIGH
            gpio_write_pin_high(B1);
            wireless_devs_change(now, load_last_bt(), false);
            break;
        case MODE_2G4:
            // 무선 모드 PB1 HIGH
            gpio_write_pin_high(B1);
            wireless_devs_change(now, DEVS_2G4, false);
            break;
        default: break;
    }
    cur_mode = mode;
    arm_reconnect();
}

uint8_t wecat4_cur_mode(void) {
    return cur_mode;
}

void keyboard_post_init_kb(void) {
    // PB1 출력 설정 후 HIGH
    gpio_set_pin_output(B1);
    gpio_write_pin_high(B1);

    gpio_set_pin_input_high(WLS_MODE_PIN_A);
    gpio_set_pin_input_high(WLS_MODE_PIN_B);
    gpio_set_pin_input_high(CHRG_FULL_PIN);

    wireless_init(&devs);
    md_sleep_bt(true);  // 모듈 무선 절전 타이머 활성화
    md_sleep_2g4(true);

    // 드라이버 초기화 충돌 방지를 위해 첫 모드 적용은 pre_task에서 처리
    keyboard_post_init_user();
}

// 길게 누르기: BT 3초 새 페어링, RESET 3초 공장 초기화
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
        case 10:
            eeconfig_disable();
            soft_reset_keyboard();
            break;
        default:
            break;
    }
}

// 무선 연결 상태 인디케이터 및 미연결 절전 타이머
#define CONN_WARN_MS 10000
#define CONN_DONE_MS 1000
#define CONN_SLEEP_MS 30000
// 유선 모드 소등: 비활성 USB 또는 호스트 요청 시(0xD2)
static uint8_t lid;

bool wecat4_lid_raw_hid(uint8_t *data) {
    // 0xD3: 배터리 상태 진단
    if (data[0] == 0xD3) {
        uint32_t now_ms = timer_read32();
        memset(data + 1, 0, 31);
        data[1] = bat_known();
        data[2] = bat_avg;
        data[3] = bat_last_raw;
        data[4] = bat_valid_cnt;
        data[5] = bat_sample_cnt;
        for (uint8_t i = 0; i < 4; i++) {
            data[6 + i]  = bat_valid_ms >> (24 - 8 * i);
            data[10 + i] = now_ms >> (24 - 8 * i);
        }
        data[14] = bat_hist_n;
        memcpy(data + 15, bat_hist, BAT_AVG_N);
        data[23] = gpio_read_pin(CHRG_FULL_PIN);
        data[24] = bat_used_n;
        return true;
    }
    if (data[0] != 0xD2) return false;
    if (data[1] == 1 && wireless_get_devs().now == DEVS_USB) lid = data[2] & 1;
    data[2] = lid;
    return true;
}

static bool wired_dark(void) {
    return USB_DRIVER.state != USB_ACTIVE || (lid & 1);
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (!wecat4_lid_raw_hid(data)) data[0] = 0xFF;
}

static uint32_t disc_since;
static uint32_t done_until;

static void conn_track(void) {
    uint8_t  now   = wireless_get_devs().now;
    uint8_t  state = *md_getp_state();
    uint32_t t     = timer_read32();
    if (now != DEVS_USB) lid = 0;
    if (now == DEVS_USB) {
        disc_since = 0;
        done_until = 0;
    } else if (state == MD_STATE_CONNECTED) {
        if (disc_since) done_until = t + CONN_DONE_MS;
        disc_since = 0;
    } else if (!disc_since) {
        disc_since = t ? t : 1;
        done_until = 0;
    } else if (state != MD_STATE_PAIRING && !pair_until && held_k < 0 && lpwr_get_state() == LPWR_NORMAL &&
               timer_elapsed32(disc_since) >= CONN_SLEEP_MS && last_input_activity_elapsed() >= CONN_SLEEP_MS) {
        disc_since = 0;
        lpwr_set_manual_timeout(true);
    }
}

void wireless_pre_task(void) {
    static uint32_t t;
    static uint8_t  last;
    long_press_task();
    pair_task();
    reconnect_task();
    conn_track();
    bat_track();
    if (timer_elapsed32(t) < 50) return;
    t = timer_read32();
    uint8_t m = read_switch();
    if (m != MODE_NONE && m == last && m != cur_mode) apply_mode(m);
    last = m;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {
    if (old_devs != new_devs) bat_reset();
    // 유선 모드 PB1 LOW, 무선 모드 PB1 HIGH
    if (new_devs == DEVS_USB) {
        gpio_write_pin_low(B1);
    } else {
        gpio_write_pin_high(B1);
    }
    save_last_bt(new_devs);
}

// 절전 복귀 시 재연결 시도
void lpwr_wakeup_hook(void) {
    arm_reconnect();
}

// 유선 모드 절전 차단 (무선 모드만 LPWR_TIMEOUT 적용)
bool lpwr_is_allow_timeout_hook(void) {
    return wireless_get_devs().now != DEVS_USB;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode < QK_KB_0 || keycode > QK_KB_11) return process_record_user(keycode, record);

    uint8_t k = keycode - QK_KB_0;
    if (record->event.pressed) {
        press_time = timer_read();
        if (k <= 2 || k == 10) {
            held_k    = k;
            long_done = false;
        }
        if (k == 5 && cur_mode == MODE_2G4) pair_start();
        // 채널 전환: 즉시 전환 명령 전송
        if (k <= 2 && cur_mode == MODE_BT) {
            reconn_left = 0;
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

    // 릴리즈: 길게 누르기 판정 종료
    if (held_k == k) held_k = -1;
    return false;
}

// ---- 인디케이터 ----
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

// 레이어 내 현재 연결 채널 키 LED 점멸
#define CONN_KEY_BLINK_MS 500
static void blink_connected_key(uint8_t led_min, uint8_t led_max, uint8_t devs) {
    uint16_t kc;
    uint8_t  r = 0, g = 0, b = 0;
    switch (devs) {
        case DEVS_BT1: kc = QK_KB_0, r = 255; break;
        case DEVS_BT2: kc = QK_KB_1, g = 255; break;
        case DEVS_BT3: kc = QK_KB_2, b = 255; break;
        case DEVS_2G4: kc = QK_KB_5, g = 255, b = 255; break;
        default: return;
    }
    if (!blink(CONN_KEY_BLINK_MS)) r = g = b = 0;
    uint8_t layer = get_highest_layer(layer_state);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t i = g_led_config.matrix_co[row][col];
            if (i == NO_LED || i < led_min || i >= led_max) continue;
            keypos_t pos = {.row = row, .col = col};
            if (keymap_key_to_keycode(layer, pos) == kc) rgb_matrix_set_color(i, r, g, b);
        }
    }
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) return false;

    uint8_t now = wireless_get_devs().now;

    if (now == DEVS_USB && wired_dark()) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        return false;
    }

    // 배터리 잔량: Q~P 10단계 초록 (USB 모드는 노랑 점멸)
    if (timer_read32() < show_bat_until) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        if (now == DEVS_USB) {
            if (blink(250)) for (uint8_t c = 1; c <= 10; c++) set_rc(0, c, 200, 160, 0);
        } else if (!bat_known()) {
            if (blink(250)) for (uint8_t c = 1; c <= 10; c++) set_rc(0, c, 200, 200, 200);
        } else {
            uint8_t n = (bat_avg + 9) / 10;
            for (uint8_t c = 1; c <= 10 && c <= n; c++) set_rc(0, c, 0, 200, 0);
        }
        return false;
    }
    // 충전 상태: 빨강(충전 중) / 초록(완충)
    if (timer_read32() < show_chrg_until) {
        bool full = gpio_read_pin(CHRG_FULL_PIN);
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, full ? 0 : 200, full ? 200 : 0, 0);
        return false;
    }
    // 채널 키 인디케이터 (BT1 빨강, BT2 초록, BT3 파랑, 2.4G 청록)
    bool    chan_key = held_k >= 0 && (held_k <= 2 || held_k == 5);
    uint8_t held_dev = held_k == 5 ? DEVS_2G4 : DEVS_BT1 + held_k;
    bool    pairing  = now != DEVS_USB && (*md_getp_state() == MD_STATE_PAIRING || pair_until);
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
            // 연결 시도 인디케이터
            if (now != DEVS_2G4) {
                for (uint8_t d = DEVS_BT1; d <= DEVS_BT3; d++) {
                    if (d != now) channel_led(d, true);
                }
            }
            channel_led(now, blink(250));
        } else {
            // 연결 실패 경고 (어두운 빨강 점멸)
            if (blink(1000)) {
                for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 80, 0, 0);
            }
            channel_led(now, true);
        }
    } else if (now != DEVS_USB && timer_read32() < done_until) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        channel_led(now, true);
    } else if (now != DEVS_USB && get_highest_layer(layer_state) > 0) {
        blink_connected_key(led_min, led_max, now);
    }
    // 저배터리 인디케이터
    if (now != DEVS_USB && bat_known() && bat_avg <= LOWBAT_PERCENT && blink(500)) {
        set_rc(LOWBAT_ROW, LOWBAT_COL, 255, 0, 0);
    }
    return true;
}

// vial-qmk 심볼 호환용 더미 변수
matrix_row_t matrix_previous[MATRIX_ROWS];
