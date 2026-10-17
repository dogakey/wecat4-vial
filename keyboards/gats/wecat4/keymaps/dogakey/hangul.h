// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#define HANGUL QK_KB_13 // 한/영: 톡 = 입력 언어 전환, 누른 채 치기 = 그동안만 다른 언어

bool hangul_held(void);
bool hangul_record(uint16_t keycode, keyrecord_t *record, bool *tapped);
