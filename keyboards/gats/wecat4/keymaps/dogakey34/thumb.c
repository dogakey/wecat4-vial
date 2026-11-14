// SPDX-License-Identifier: GPL-2.0-or-later
// 엄지 키 체계 — 34키 엄지 동작.
//
// L1 엄지(THUMB): 누르는 즉시 숫자 레이어. 다른 키 없이 THUMB_TAP_MS 안에 떼면 기호 레이어 원샷.
//   vial-qmk 는 permissive hold·hold on other key press·tapping term 을 Vial QMK Settings 의 전역값 하나로만 받는다
//   (get_* 함수를 qmk_settings.c 가 강한 심볼로 정의). F/J 홈로우 Shift 는 다른 키 누름만으로 Shift 가 되면 안 되고
//   이 키는 반대로 다른 키를 누르는 즉시 숫자여야 하므로, LT 대신 여기서 직접 처리한다.
//
// 한/영(HANGUL): 누르면 입력 언어 전환. 누른 뒤 다른 키를 눌렀다가 한/영보다 먼저 떼면 "누른 채 치기"로 보고
//   한/영을 뗄 때 다시 전환한다(그동안 영문). 다음 키를 누르고 한/영을 먼저 떼는 굴림은 톡으로 남는다.
//   맥(기본 레이어 0): CapsLock 100ms 누름 + 30ms 쉼 — 2SetKorean 은 70ms 이하로 누른 CapsLock 을 무시하고
//   간격 없이 이어진 두 번째 전환을 버린다. 윈도우(기본 레이어 1): Lang1 10ms 누름 + 10ms 쉼.
//   전환하는 동안 들어온 키 이벤트는 대기열에 두었다가 전환이 끝나면 순서대로 처리한다. 그 자리에서 기다리면
//   무선 모드에서 누름·뗌 보고가 붙어서 나가 100ms 누름이 사라지고, 기다리는 동안 눌렀다 뗀 키도 놓친다.
//
// 기호 원샷 중 기호 레이어에 직접 둔 수식키(한/영 자리 Shift) = 아무것도 입력하지 않고 원샷 취소.
//   QMK 원샷 레이어는 수식키를 눌러도 풀리지 않으므로 여기서 푼다.
//
// 오른쪽 Space(SPC_NAV): 200ms를 넘겨 누르고 있을 때만 편집 레이어(4), 그 전에 떼면 Space 탭.
//   200ms 안에 다른 키를 눌렀다 떼도 홀드로 보지 않고 대기열에 두었다가(normal mode), Space를 뗄 때
//   Space 탭 뒤에 그 키들을 순서대로 실행한다.
//   탭 후 200ms 안에 다시 누르면 홀드 판정 없이 Space를 즉시 등록해 누르고 있는 동안 Space가 반복된다(quick tap).
//
// Vial QMK Settings 기본값: permissive hold 켬, chordal hold 켬, flow tap 150ms.
//   새 펌웨어 첫 부팅·EEPROM 초기화 때만 넣고, 그 뒤 Vial 에서 바꾼 값은 건드리지 않는다.
//   tapping term 250ms·원샷 60초는 config.h(Vial 초기화 기본값이 그 값을 쓴다).
#include QMK_KEYBOARD_H
#include "wireless.h"
#include "qmk_settings.h"
#include "thumb.h"

#define THUMB_TAP_MS 200

#define HANGUL_MAC_HOLD_MS 100
#define HANGUL_MAC_GAP_MS 30
#define HANGUL_WIN_HOLD_MS 10
#define HANGUL_WIN_GAP_MS 10

// quantum/qmk_settings.c protos 의 설정 번호
#define QSID_PERMISSIVE_HOLD 22
#define QSID_CHORDAL_HOLD 26
#define QSID_FLOW_TAP_TERM 27
#define DEFAULT_FLOW_TAP_TERM 150

// chordal hold 손 구분(행렬 = LAYOUT 위치). 아랫줄 엄지 키는 양손 어느 쪽과도 누름 허용.
// 셋째 줄은 "/" 가 Z 왼쪽에 있어 B(2,6)까지 왼손이다.
// clang-format off
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    {'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R'},
    {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*'},
};
// clang-format on

// ---- Vial QMK Settings 기본값 ----

static bool qs_defaults_due;

// 새 펌웨어(BUILD_ID 가 다름)면 via_init 이 키맵·QMK Settings 를 초기화한다. 그 판정 직전에 불린다.
void via_init_kb(void) {
    if (!via_eeprom_is_valid()) qs_defaults_due = true;
}

// EE_CLR·공장 초기화 경로(eeconfig_init → dynamic_keymap_reset 뒤에 불린다)
void eeconfig_init_user(void) {
    qs_defaults_due = true;
}

void keyboard_post_init_user(void) {
    if (!qs_defaults_due) return;
    qs_defaults_due = false;
    uint8_t  on   = 1;
    uint16_t flow = DEFAULT_FLOW_TAP_TERM;
    qmk_settings_set(QSID_PERMISSIVE_HOLD, &on, sizeof(on));
    qmk_settings_set(QSID_CHORDAL_HOLD, &on, sizeof(on));
    qmk_settings_set(QSID_FLOW_TAP_TERM, &flow, sizeof(flow));
}

// ---- L1 엄지 ----

static bool     th_down;
static bool     th_other; // 엄지를 누른 동안 다른 키를 눌렀다 = 숫자 레이어로 썼다
static uint16_t th_time;

static void thumb_record(keyrecord_t *record) {
    if (record->event.pressed) {
        // 기호 원샷 대기 중에 다시 누르면 대기를 버리고 새로 판정한다(누름 = 숫자, 톡 = 기호 다시 대기)
        if (is_oneshot_layer_active()) clear_oneshot_layer_state(ONESHOT_START);
        layer_on(LAYER_NUM);
        th_down  = true;
        th_other = false;
        th_time  = record->event.time;
    } else {
        layer_off(LAYER_NUM);
        th_down = false;
        if (!th_other && TIMER_DIFF_16(record->event.time, th_time) < THUMB_TAP_MS) {
            // OSL 키를 눌렀다 뗀 것과 같은 상태: 다음 키 하나에만 적용, 원샷 시간은 QMK Settings(ONESHOT_TIMEOUT)
            bool mac = get_highest_layer(default_layer_state) == LAYER_MAC;
            set_oneshot_layer(mac ? LAYER_SYM : LAYER_SYM_WIN, ONESHOT_START);
            clear_oneshot_layer_state(ONESHOT_PRESSED);
        }
    }
}

// ---- 한/영 ----

enum { HG_IDLE, HG_PRESS, HG_GAP };

#define HG_QUEUE_LEN 16

static uint8_t  hg_phase;
static uint8_t  hg_pending; // 보내야 할 전환 횟수
static uint16_t hg_kc, hg_hold, hg_gap, hg_timer;
static bool     hg_down;   // 한/영 키가 눌려 있다
static bool     hg_after;  // 한/영을 누른 뒤 다른 키를 눌렀다
static bool     hg_used;   // 그 키를 한/영보다 먼저 뗐다 = 누른 채 치기
static bool     hg_typing; // 조명용: 누른 채 다른 키를 눌렀다 = 뗄 때 번쩍임 생략(누름으로 판정)
static bool hg_is_korean = false;  // 부팅 가정: 영문
static bool hg_was_korean = false; // 누른 순간의 상태
static bool     hg_tap_valid;        // 직전에 탭(홀드 타이핑 아님)으로 뗐다
static uint16_t hg_last_tap;         // 그 탭을 뗀 시각
static bool     hg_double_tapped;    // 이번 누름이 더블탭이다
static uint16_t hg_press_time;       // 한/영 키를 누른 시각
#define HANGUL_HOLD_CANCEL_MS 250    // 이 시간 이상 눌렀고 다른 키가 없었으면 전환 취소

static keyevent_t hg_queue[HG_QUEUE_LEN];
static uint8_t    hg_q_head, hg_q_len;
static bool       hg_replaying;

bool hangul_held(void) {
    return hg_down;
}

static bool hangul_busy(void) {
    return hg_phase != HG_IDLE || hg_pending;
}

static void hangul_step(void) {
    switch (hg_phase) {
        case HG_IDLE: {
            if (!hg_pending) return;
            hg_pending--;
            bool mac = get_highest_layer(default_layer_state) == LAYER_MAC;
            hg_kc    = mac ? KC_CAPS : KC_LNG1;
            hg_hold  = mac ? HANGUL_MAC_HOLD_MS : HANGUL_WIN_HOLD_MS;
            hg_gap   = mac ? HANGUL_MAC_GAP_MS : HANGUL_WIN_GAP_MS;
            register_code(hg_kc);
            hg_timer = timer_read();
            hg_phase = HG_PRESS;
            break;
        }
        case HG_PRESS:
            if (timer_elapsed(hg_timer) < hg_hold) return;
            unregister_code(hg_kc);
            hg_timer = timer_read();
            hg_phase = HG_GAP;
            break;
        case HG_GAP:
            if (timer_elapsed(hg_timer) < hg_gap) return;
            hg_phase = HG_IDLE;
            break;
    }
}

// 대기열이 넘칠 때만 쓴다: 그 자리에서 전환을 끝낸다. 무선이면 보고마다 모듈 전송이 끝나길 기다려
// 누름 시간이 호스트에 그대로 보이게 한다.
static void hangul_finish_now(void) {
    while (hangul_busy()) {
        hangul_step();
        if (get_transport() != TRANSPORT_USB) wireless_wait_md_free();
        wait_ms(1);
    }
}

static void hangul_replay(void) {
    hg_replaying = true;
    while (hg_q_len && !hangul_busy()) {
        keyevent_t e = hg_queue[hg_q_head];
        hg_q_head    = (hg_q_head + 1) % HG_QUEUE_LEN;
        hg_q_len--;
        action_exec(e);
    }
    hg_replaying = false;
}

static void hangul_record(keyrecord_t *record) {
    if (record->event.pressed) {
        hg_down          = true;
        hg_after         = false;
        hg_used          = false;
        hg_typing        = false;
        hg_double_tapped = false;
        // 탭 뒤 350ms 안에 다시 누르면 더블탭: 한글로 초기화
        if (hg_tap_valid && timer_elapsed(hg_last_tap) <= 350) {
            hg_tap_valid     = false;
            hg_double_tapped = true;
            hg_is_korean     = true;
            hg_pending++;
            hangul_step();
            return;
        }
        hg_press_time = timer_read();
        hg_tap_valid  = false;
        hg_was_korean = hg_is_korean;
        if (hg_was_korean) {
            hg_pending++;      // 한글일 때만 누를 때 영문으로 토글
            hangul_step();
        }
    } else {
        hg_down = false;
        bool hg_cancelled = false;
        if (hg_double_tapped) {
            hg_double_tapped = false;
            hg_typing        = false;
            highlight_flash();
            return;
        }
        if (hg_used) {
            // 홀드 타이핑: 시작이 한글이었으면 되돌리고, 영문이었으면 영문 유지
            if (hg_was_korean) {
                hg_pending++;
                hangul_step();
                hg_is_korean = true;
            } else {
                hg_is_korean = false;
            }
            hg_tap_valid = false;
        } else if (!hg_after && timer_elapsed(hg_press_time) >= HANGUL_HOLD_CANCEL_MS) {
            // 길게 눌렀고 다른 키가 없었다: 누를 때 보낸 전환을 되돌려 아무 변화 없음
            if (hg_was_korean) {
                hg_pending++;
                hangul_step();
            }
            hg_is_korean = hg_was_korean;
            hg_tap_valid = false;
            hg_cancelled = true;
        } else {
            // 탭: 상태 반전
            if (hg_was_korean) {
                hg_is_korean = false;       // 누를 때 이미 토글됨
            } else {
                hg_pending++;               // 영문 시작이면 뗄 때 한글로 토글
                hangul_step();
                hg_is_korean = true;
            }
            hg_tap_valid = true;
            hg_last_tap  = timer_read();
        }
        // 톡이면 뗄 때 초록 번쩍임, 누른 채 쳤으면 떼는 즉시 원래 조명
        if (!hg_typing && !hg_cancelled) highlight_flash();
        hg_typing = false;
    }
}

// ---- 오른쪽 Space (SPC_NAV) ----

#define SPACE_TAP_MS 200
#define SPACE_QUICK_TAP_MS SPACE_TAP_MS
#define SPC_QUEUE_LEN 16

static bool       spc_down;
static bool       spc_held;
static bool       spc_repeating;
static uint16_t   spc_time;
static uint16_t   spc_tap_time;
static bool       spc_can_quick_tap;

static keyevent_t spc_queue[SPC_QUEUE_LEN];
static uint8_t    spc_q_head, spc_q_len;
static bool       spc_replaying;

static void spc_step(void) {
    if (spc_down && !spc_held && !spc_repeating && timer_elapsed(spc_time) >= SPACE_TAP_MS) {
        spc_held = true;
        layer_on(LAYER_NAV);
        spc_replaying = true;
        while (spc_q_len) {
            keyevent_t e = spc_queue[spc_q_head];
            spc_q_head   = (spc_q_head + 1) % SPC_QUEUE_LEN;
            spc_q_len--;
            action_exec(e);
        }
        spc_replaying = false;
    }
}

static void spc_replay_tapped(uint16_t tap_time) {
    spc_tap_time      = tap_time;
    spc_can_quick_tap = true;
    spc_replaying     = true;
    tap_code(KC_SPC);
    while (spc_q_len) {
        keyevent_t e = spc_queue[spc_q_head];
        spc_q_head   = (spc_q_head + 1) % SPC_QUEUE_LEN;
        spc_q_len--;
        action_exec(e);
    }
    spc_replaying = false;
}

static void space_record(keyrecord_t *record) {
    if (record->event.pressed) {
        if (spc_can_quick_tap && timer_elapsed(spc_tap_time) < SPACE_QUICK_TAP_MS) {
            spc_down          = true;
            spc_repeating     = true;
            spc_held          = false;
            spc_can_quick_tap = false;
            register_code(KC_SPC);
            return;
        }
        spc_down          = true;
        spc_held          = false;
        spc_repeating     = false;
        spc_can_quick_tap = false;
        spc_time          = record->event.time;
        spc_q_head        = 0;
        spc_q_len         = 0;
    } else {
        if (spc_repeating) {
            unregister_code(KC_SPC);
            spc_repeating = false;
        } else if (spc_held) {
            layer_off(LAYER_NAV);
            spc_held = false;
        } else {
            spc_replay_tapped(record->event.time);
        }
        spc_down = false;
    }
}

void matrix_scan_user(void) {
    hangul_step();
    if (!hangul_busy() && hg_q_len) hangul_replay();
    spc_step();
}

// 탭/홀드·콤보 판정보다 앞에서 받아 두어야 대기열의 순서와 시각이 그대로 다시 처리된다.
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (hg_replaying || spc_replaying || record->event.type != KEY_EVENT) return true;

    // 한/영 전환 중 키 대기
    if (hangul_busy() || hg_q_len) {
        if (hg_q_len == HG_QUEUE_LEN) {
            while (hg_q_len || hangul_busy()) {
                hangul_finish_now();
                hangul_replay();
            }
            return true;
        }
        hg_queue[(hg_q_head + hg_q_len) % HG_QUEUE_LEN] = record->event;
        hg_q_len++;
        return false;
    }

    // Space 판정 대기 중 키 대기 (200ms normal mode)
    if (spc_down && !spc_held && !spc_repeating) {
        if (keycode == SPC_NAV || (record->event.key.row == 3 && record->event.key.col == 7)) {
            return true;
        }
        if (spc_q_len == SPC_QUEUE_LEN) {
            spc_replay_tapped(record->event.time);
            return true;
        }
        spc_queue[(spc_q_head + spc_q_len) % SPC_QUEUE_LEN] = record->event;
        spc_q_len++;
        return false;
    }

    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    highlight_record(keycode, record);
    if (keycode == HANGUL) {
        hangul_record(record);
        return false;
    }
    if (keycode == THUMB) {
        thumb_record(record);
        return false;
    }
    if (keycode == SPC_NAV) {
        space_record(record);
        return false;
    }
    if (record->event.pressed) {
        if (th_down) th_other = true;
        if (hg_down) hg_after = hg_typing = true;
        uint8_t osl = get_oneshot_layer();
        if (IS_MODIFIER_KEYCODE(keycode) && is_oneshot_layer_active() && (osl == LAYER_SYM || osl == LAYER_SYM_WIN) && keymap_key_to_keycode(osl, record->event.key) == keycode) {
            clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        }
    } else if (hg_down && hg_after) {
        hg_used = true;
    }
    return true;
}
