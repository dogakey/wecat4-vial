// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include QMK_KEYBOARD_H

// QK_KB_12~14 (0~11은 wireless_glue.c에서 처리)
#define HANGUL QK_KB_12 // 탭: 한/영 전환, 홀드: 영문 타이핑
#define THUMB QK_KB_13  // 탭: 기호(L3 원샷), 홀드: 숫자(L2)
#define SPC_NAV QK_KB_14 // 탭: Space, 200ms 홀드: 편집(L4)

#define LAYER_MAC 0
#define LAYER_WIN 1
#define LAYER_NUM 2
#define LAYER_SYM 3
#define LAYER_NAV 4
#define LAYER_FN 5
#define LAYER_SYM_WIN 6

bool hangul_held(void);

void highlight_record(uint16_t keycode, keyrecord_t *record);
void highlight_flash(void);
