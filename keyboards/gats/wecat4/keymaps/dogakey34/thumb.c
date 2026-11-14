// SPDX-License-Identifier: GPL-2.0-or-later
// 34키 엄지 키 체계 (THUMB, HANGUL, SPC_NAV)
// - THUMB: 탭 기호(L3 원샷), 홀드 숫자(L2)
// - HANGUL: 탭 전환, 홀드 타이핑 영문, 250ms 이상 단독 홀드 시 취소
// - SPC_NAV: 탭 Space, 200ms 이상 홀드 시 편집(L4), 탭 직후 반복(quick tap)
#include QMK_KEYBOARD_H
#include "wireless.h"
#include "qmk_settings.h"
#include "thumb.h"

#define THUMB_TAP_MS 200

#define HANGUL_MAC_HOLD_MS 100
#define HANGUL_MAC_GAP_MS 30
#define HANGUL_WIN_HOLD_MS 10
#define HANGUL_WIN_GAP_MS 10

// quantum/qmk_settings.c 설정 ID
#define QSID_PERMISSIVE_HOLD 22
#define QSID_CHORDAL_HOLD 26
#define QSID_FLOW_TAP_TERM 27
#define DEFAULT_FLOW_TAP_TERM 150

// chordal hold 좌우 손 구분
// clang-format off
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
    {'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R'},
    {'L', 'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R'},
    {'*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*'},
};
// clang-format on

// Vial QMK Settings 기본값
static bool qs_defaults_due;

// BUILD_ID 변경 시 기본값 재설정 플래그
void via_init_kb(void) {
    if (!via_eeprom_is_valid()) qs_defaults_due = true;
}

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

// THUMB 키 처리
static bool     th_down;
static bool     th_other;
static uint16_t th_time;

static void thumb_record(keyrecord_t *record) {
    if (record->event.pressed) {
        if (is_oneshot_layer_active()) clear_oneshot_layer_state(ONESHOT_START);
        layer_on(LAYER_NUM);
        th_down  = true;
        th_other = false;
        th_time  = record->event.time;
    } else {
        layer_off(LAYER_NUM);
        th_down = false;
        if (!th_other && TIMER_DIFF_16(record->event.time, th_time) < THUMB_TAP_MS) {
            bool mac = get_highest_layer(default_layer_state) == LAYER_MAC;
            set_oneshot_layer(mac ? LAYER_SYM : LAYER_SYM_WIN, ONESHOT_START);
            clear_oneshot_layer_state(ONESHOT_PRESSED);
        }
    }
}

// 한/영 키 처리
enum { HG_IDLE, HG_PRESS, HG_GAP };

#define HG_QUEUE_LEN 16

static uint8_t  hg_phase;
static uint8_t  hg_pending;
static uint16_t hg_kc, hg_hold, hg_gap, hg_timer;
static bool     hg_down;
static bool     hg_after;
static bool     hg_used;
static bool     hg_typing;
static bool     hg_is_korean = false;
static bool     hg_was_korean = false;
static bool     hg_tap_valid;
static uint16_t hg_last_tap;
static bool     hg_double_tapped;
static uint16_t hg_press_time;
#define HANGUL_HOLD_CANCEL_MS 250 // 250ms 이상 단독 홀드 시 취소

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

// 대기열 초과 시 즉시 동기 완료
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
        // 350ms 이내 더블탭 시 한글 설정
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
            hg_pending++;
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
            // 홀드 타이핑 복귀
            if (hg_was_korean) {
                hg_pending++;
                hangul_step();
                hg_is_korean = true;
            } else {
                hg_is_korean = false;
            }
            hg_tap_valid = false;
        } else if (!hg_after && timer_elapsed(hg_press_time) >= HANGUL_HOLD_CANCEL_MS) {
            // 250ms 이상 단독 홀드 시 전환 취소
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
                hg_is_korean = false;
            } else {
                hg_pending++;
                hangul_step();
                hg_is_korean = true;
            }
            hg_tap_valid = true;
            hg_last_tap  = timer_read();
        }
        if (!hg_typing && !hg_cancelled) highlight_flash();
        hg_typing = false;
    }
}

// 오른쪽 Space (SPC_NAV)
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

// 탭/홀드 판정 전 키 이벤트 대기열 처리
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (hg_replaying || spc_replaying || record->event.type != KEY_EVENT) return true;

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
