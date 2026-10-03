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

// 5분 미사용 백라이트 끔(설명서)
#define RGB_MATRIX_TIMEOUT 300000
