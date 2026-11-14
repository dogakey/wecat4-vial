// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include QMK_KEYBOARD_H

// vial.json customKeycodes 순서(QK_KB_n). 0~11 은 wireless_glue.c 가 처리한다.
#define HANGUL QK_KB_12 // 한/영: 톡 = 전환, 누른 채 치면 그동안 영문
#define THUMB QK_KB_13  // L1 엄지: 톡 = 기호 원샷(레이어 3), 누름 = 숫자(레이어 2)
#define SPC_NAV QK_KB_14 // 오른쪽 Space: 톡 = Space, 200ms 누름 = 편집 레이어(4)

#define LAYER_MAC 0
#define LAYER_WIN 1
#define LAYER_NUM 2
#define LAYER_SYM 3
#define LAYER_NAV 4
#define LAYER_FN 5
#define LAYER_SYM_WIN 6

// 한/영 키를 누른 채 다른 키를 치는 중(그동안 영문)
bool hangul_held(void);

// highlight.c
void highlight_record(uint16_t keycode, keyrecord_t *record);
void highlight_flash(void);
