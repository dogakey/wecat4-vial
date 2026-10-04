// SPDX-License-Identifier: GPL-2.0-or-later
// 진단 빌드 전용(WECAT4_DIAG): 무선 연결 상태를 USB raw HID 로 읽는다.
// KEEP_USB_CONNECTION_IN_WIRELESS_MODE 와 함께 써서 무선 모드에서도 USB 를 유지하고,
// md_raw.c 가 응답을 USB 로 보내게 한다. 명령 0xD1 → 아래 형식으로 응답.
//   [1] md 연결 상태  [2] md 표시등  [3] devs.now  [4] cur_mode  [5] lpwr 상태  [6] lpwr 모드
//   [7] 전송 경로(0 USB / 1 무선)  [8] nkro  [9] md 배터리  [10..13] 가동 ms
//   [14] host 잠듦 알림 횟수  [15] host 깸 알림 횟수  [16..19] 마지막 host 알림 ms
//   [20] 기록 개수(최대 3)  [21..29] 최근 상태 변화 3건 × (md상태, lpwr상태, lpwr모드)
#include QMK_KEYBOARD_H
#include "wireless.h"

uint8_t wecat4_cur_mode(void);

static uint8_t  host_sleep_cnt, host_wake_cnt;
static uint32_t host_last_ms;
static uint8_t  hist[3][3], hist_n;

void host_state_cb_user(bool resume) {
    if (resume) {
        host_wake_cnt++;
    } else {
        host_sleep_cnt++;
    }
    host_last_ms = timer_read32();
}

// 연결 상태·절전 상태가 바뀔 때마다 최근 3건을 남긴다.
void wireless_post_task(void) {
    static uint8_t last[3] = {0xFF, 0xFF, 0xFF};
    uint8_t        cur[3]  = {*md_getp_state(), lpwr_get_state(), lpwr_get_mode()};
    if (memcmp(cur, last, 3) == 0) return;
    memcpy(last, cur, 3);
    memmove(hist[1], hist[0], sizeof(hist) - sizeof(hist[0]));
    memcpy(hist[0], cur, 3);
    if (hist_n < 3) hist_n++;
}

static void put32(uint8_t *p, uint32_t v) {
    p[0] = v >> 24, p[1] = v >> 16, p[2] = v >> 8, p[3] = v;
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (data[0] != 0xD1) {
        data[0] = 0xFF; // id_unhandled
        return;
    }
    memset(data + 1, 0, length - 1);
    data[1] = *md_getp_state();
    data[2] = *md_getp_indicator();
    data[3] = wireless_get_devs().now;
    data[4] = wecat4_cur_mode();
    data[5] = lpwr_get_state();
    data[6] = lpwr_get_mode();
    data[7] = get_transport() == TRANSPORT_USB ? 0 : 1;
    data[8] = keymap_config.nkro;
    data[9] = *md_getp_bat();
    put32(data + 10, timer_read32());
    data[14] = host_sleep_cnt;
    data[15] = host_wake_cnt;
    put32(data + 16, host_last_ms);
    data[20] = hist_n;
    for (uint8_t i = 0; i < 3; i++) memcpy(data + 21 + i * 3, hist[i], 3);
}
