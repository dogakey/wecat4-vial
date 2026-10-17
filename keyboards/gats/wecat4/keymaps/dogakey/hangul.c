// SPDX-License-Identifier: GPL-2.0-or-later
// 한/영 키(커스텀 키 13, vial.json "한/영"): 누르면 입력 언어 전환. 누른 채 다른 키를 쳤다가 그 키를 먼저 떼면
// "누른 채 치기"로 보고 한/영을 뗄 때 한 번 더 전환한다(누르고 있는 동안만 다른 언어). 다음 키를 누르고
// 한/영을 먼저 떼는 굴림은 톡으로 남는다.
//   macOS 모드(기본 레이어 1): CapsLock 100ms 누름 + 30ms 쉼. macOS 「CapsLock 으로 입력 소스 전환」과
//     2SetKorean 은 짧게 누른 CapsLock(약 70ms 이하)을 무시하고, 간격 없이 이어진 두 번째 전환을 버린다.
//   Windows 모드(기본 레이어 0): 한/영 키(Lang1) 10ms 누름 + 10ms 쉼.
// 전환하는 동안 들어온 키 이벤트는 대기열에 두었다가 전환이 끝나면 순서대로 처리한다. 그 자리에서 기다리면
// 무선 모드에서 누름·뗌 보고가 붙어 나가 100ms 누름이 사라지고, 기다리는 동안 눌렀다 뗀 키도 놓친다.
#include QMK_KEYBOARD_H
#include "wireless.h"
#include "hangul.h"

#define HANGUL_MAC_HOLD_MS 100
#define HANGUL_MAC_GAP_MS 30
#define HANGUL_WIN_HOLD_MS 10
#define HANGUL_WIN_GAP_MS 10

#define HG_QUEUE_LEN 16

enum { HG_IDLE, HG_PRESS, HG_GAP };

static uint8_t  hg_phase;
static uint8_t  hg_pending; // 보내야 할 전환 횟수
static uint16_t hg_kc, hg_hold, hg_gap, hg_timer;
static bool     hg_down;   // 한/영 키가 눌려 있다
static bool     hg_after;  // 한/영을 누른 뒤 다른 키를 눌렀다
static bool     hg_used;   // 그 키를 한/영보다 먼저 뗐다 = 누른 채 치기

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

void matrix_scan_user(void) {
    hangul_step();
    if (!hangul_busy() && hg_q_len) hangul_replay();
}

// 탭/홀드·콤보 판정보다 앞에서 받아 두어야 대기열의 순서와 시각이 그대로 다시 처리된다.
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (hg_replaying || record->event.type != KEY_EVENT) return true;
    if (!hangul_busy() && !hg_q_len) return true;
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

// 한/영 키 자신이면 처리하고 true(그 키는 더 넘기지 않는다). 톡이면 tapped 를 true 로 돌려준다.
bool hangul_record(uint16_t keycode, keyrecord_t *record, bool *tapped) {
    *tapped = false;
    if (keycode == HANGUL) {
        if (record->event.pressed) {
            hg_down  = true;
            hg_after = false;
            hg_used  = false;
            hg_pending++;
            hangul_step();
        } else {
            hg_down = false;
            if (hg_used) {
                hg_pending++;
                hangul_step();
            }
            *tapped = !hg_after;
        }
        return true;
    }
    if (record->event.pressed) {
        if (hg_down) hg_after = true;
    } else if (hg_down && hg_after) {
        hg_used = true;
    }
    return false;
}
