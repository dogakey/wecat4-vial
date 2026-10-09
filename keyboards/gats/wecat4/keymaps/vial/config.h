// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Vial 키보드 고유 ID(무작위 생성값)
#define VIAL_KEYBOARD_UID {0xFD, 0x30, 0x32, 0xA4, 0xDC, 0x75, 0x92, 0x80}

// Vial 잠금 해제: Tab(0,0) + Backspace(0,11) 를 함께 누른다
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 11 }

// 원 펌웨어와 같은 8레이어
#define DYNAMIC_KEYMAP_LAYER_COUNT 8

// 디바운스 5ms (알고리즘은 rules.mk 의 DEBOUNCE_TYPE)
#define DEBOUNCE 5

// ---- 무선(제조사 libmodule.a 그대로 사용) ----
// UART3 = C10(TX)/C11(RX), AF7. 원 펌웨어의 핀 설정과 같다.
#define UART_DRIVER SD3
#define UART_TX_PIN C10
#define UART_RX_PIN C11
#define UART_TX_PAL_MODE 7
#define UART_RX_PAL_MODE 7
// 블루투스 이름·동글 문자열
#define BT1_NAME "WECAT4 BT1"
#define BT2_NAME "WECAT4 BT2"
#define BT3_NAME "WECAT4 BT3"
#define BT4_NAME "WECAT4 BT4"
#define BT5_NAME "WECAT4 BT5"
#define DONGLE_PRODUCT "WECAT4 Dongle"
// 무선 리포트 키 수(제조사 wecat7 설정과 동일)
#define WLS_KEYBOARD_REPORT_KEYS 5
// 설정 저장: 마지막 연결 BT 채널 1바이트
#define EECONFIG_KB_DATA_SIZE 1

// ---- 절전(설명서): 5분 미사용 백라이트 끔 / 30분 미사용 수면(무선) ----
#define RGB_MATRIX_TIMEOUT 300000
// 컴퓨터가 잠들어 USB 가 절전에 들어가면 조명을 끈다(유선). 무선 절전에서 깰 때는 lowpower.c 가 suspend_wakeup_init() 으로 되살린다.
#define RGB_MATRIX_SLEEP
#define LPWR_TIMEOUT 1800000
