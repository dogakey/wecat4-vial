// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Vial 키보드 고유 ID(무작위 생성값)
#define VIAL_KEYBOARD_UID {0xFD, 0x30, 0x32, 0xA4, 0xDC, 0x75, 0x92, 0x80}

// 잠금해제: Tab(0,0) + Backspace(0,11)
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 11 }

// 원 펌웨어와 같은 8레이어
#define DYNAMIC_KEYMAP_LAYER_COUNT 8

// 탭/홀드(F/J 홈로우 Shift). vial-qmk 는 키별 값을 받지 않고 Vial QMK Settings 전역값 하나를 쓴다.
// L1 엄지는 thumb.c 에서 따로 200ms.
#define TAPPING_TERM 220
// 기호 원샷은 다음 키를 칠 때까지 사실상 기다린다
#define ONESHOT_TIMEOUT 60000

// 디바운스 28ms (알고리즘은 rules.mk의 DEBOUNCE_TYPE = eager). 누르는 즉시 입력하고 이후 28ms 동안의 접점 변화를 무시한다.
// B 키에서 26ms 간격 중복 입력이 관측돼 타이머 해상도(1ms)까지 2ms 여유를 둔 값. defer 방식은 눌림이 그만큼 늦어져 eager로 바꿨다
#define DEBOUNCE 40

// ---- 무선(제조사 libmodule.a 그대로 사용) ----
// UART3 = C10(TX)/C11(RX), AF7 — 같은 ODM(HS) 무선 보드 dr6p·epomaker_split70의 배선을 따른 값(위캣4 실제 배선과는 대조 전)
#define UART_DRIVER SD3
#define UART_TX_PIN C10
#define UART_RX_PIN C11
#define UART_TX_PAL_MODE 7   // 데이터시트 AF7 = UART3
#define UART_RX_PAL_MODE 7
// 블루투스 이름·동글 문자열
#define BT1_NAME "WECAT4 BT1"
#define BT2_NAME "WECAT4 BT2"
#define BT3_NAME "WECAT4 BT3"
#define BT4_NAME "WECAT4 BT4"
#define BT5_NAME "WECAT4 BT5"
// 원본 펌웨어와 동일한 2.4G 동글 식별자 및 UTF-16LE 전송 플래그 설정
#define DONGLE_MANUFACTURER "Hangsheng"
#define DONGLE_PRODUCT "TH40 PRO-2.4G  Dongle"
#define MD_XX_ENABLE true
// 무선 리포트 키 수(제조사 wecat7 설정과 동일)
#define WLS_KEYBOARD_REPORT_KEYS 5
// 설정 저장: 마지막 연결 장치 1바이트
#define EECONFIG_KB_DATA_SIZE 1

// ---- 절전(설명서): 5분 미사용 백라이트 끔 / 30분 미사용 수면(무선) ----
#define RGB_MATRIX_TIMEOUT 300000
// 호스트가 잠들어 USB 가 절전에 들어가면 조명을 끈다(유선). 무선 절전 복귀는 lowpower.c 가 suspend_wakeup_init() 으로 되살린다.
#define RGB_MATRIX_SLEEP
#define LPWR_TIMEOUT 1800000
