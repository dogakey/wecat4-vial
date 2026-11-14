// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Vial 키보드 UID
#define VIAL_KEYBOARD_UID {0xFD, 0x30, 0x32, 0xA4, 0xDC, 0x75, 0x92, 0x80}

// 잠금해제: Tab(0,0) + Backspace(0,11)
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 11 }

#define DYNAMIC_KEYMAP_LAYER_COUNT 8

// 홈로우 Shift 탭/홀드 (엄지 타이밍은 thumb.c에서 별도 처리)
#define TAPPING_TERM 220
#define ONESHOT_TIMEOUT 60000

// 채터링 방지 디바운스 (sym_eager_pk)
#define DEBOUNCE 40

// 무선 통신 설정 (UART3)
#define UART_DRIVER SD3
#define UART_TX_PIN C10
#define UART_RX_PIN C11
#define UART_TX_PAL_MODE 7
#define UART_RX_PAL_MODE 7

// 블루투스 기기명
#define BT1_NAME "WECAT4 BT1"
#define BT2_NAME "WECAT4 BT2"
#define BT3_NAME "WECAT4 BT3"
#define BT4_NAME "WECAT4 BT4"
#define BT5_NAME "WECAT4 BT5"

// 2.4G 동글 식별자
#define DONGLE_MANUFACTURER "Hangsheng"
#define DONGLE_PRODUCT "TH40 PRO-2.4G  Dongle"
#define MD_XX_ENABLE true

#define WLS_KEYBOARD_REPORT_KEYS 5
#define EECONFIG_KB_DATA_SIZE 1

// 절전: 5분 백라이트 차단, 30분 수면
#define RGB_MATRIX_TIMEOUT 300000
#define RGB_MATRIX_SLEEP
#define LPWR_TIMEOUT 1800000
