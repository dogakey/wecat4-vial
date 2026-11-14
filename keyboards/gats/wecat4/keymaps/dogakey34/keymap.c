// Copyright 2026 dogakey
// SPDX-License-Identifier: GPL-2.0-or-later
//
// 위캣4 34키 키맵(dogakey34).
//   0 맥 기본 / 1 윈도우 기본(키는 같고 한/영 전환 방식만 다름). 오른손 엄지 바깥(RAlt 자리) = Backspace
//   2 숫자(L1 엄지 누름, 34키 숫자 레이어와 같은 자리, Caps 자리·한/영 엄지 자리 Esc) / 3 기호 원샷(L1 엄지 톡, 34키 기호 레이어와 같은 자리)
//   4 편집(오른쪽 Space 누름: 왼손 Esc·Tab(A)·Shift+Tab(Z)·방향키, 오른손 검지 Y/H/N = Home/End/Backspace, U/J/M = PgUp/PgDn/Delete)
//   5 F키·미디어·조명·무선(숫자 2 + 편집 4 동시 누름). 34키 영역: F1~F10 숫자 자리·F11 G·F12 H, 미디어 J~L·M~., 조명 X~V·/ 자리 모드, 맥/윈 B·N, 무선 BT1~3·2.4G 는 A~F, 배터리는 Caps 자리
// 4×12 에 맞춘 차이: 둘째 줄에 1,10 이 없어 ] 는 Enter 자리, \ 는 F 자리. 셋째 줄은 "/" 가 Z 왼쪽이라 글자 기준으로 맞췄다
// (B·N·M 자리 = 위캣4 B·N·M).
// 키코드명은 최신 QMK 번호 체계로 해독한 것이라 RGB 계열(UG_*/RGB_M_*)은 원 펌웨어와 의미가 다를 수 있다.
// RGB_M_* 는 rgblight 모드 키라 RGB Matrix에서는 동작하지 않을 수 있음 — Vial에서 교체 권장.
// QK_KB_0/1/2/5/7/8/11 = BT1/BT2/BT3/2.4G/BAT/CHRG/RAINBOW(wireless_glue.c), HANGUL·THUMB = thumb.c.

#include QMK_KEYBOARD_H
#include "thumb.h"

// 홈로우 수식(A Ctrl·S Alt·D Gui·F Shift / J Shift·K Gui·L Alt·Enter): 같은 손 키와 겹치면 글자(chordal hold), 직전 글자 뒤 바로 누르면 글자(flow tap)
// 윈도우 레이어의 L 은 LAlt — RAlt 단독 입력이 윈도우 한국어 입력기에서 한/영 전환으로 먹지 않게.
// Enter 는 톡으로만 쓰므로 누르고 있으면 오른쪽 Shift.
// 오른쪽 Space 는 톡으로 누르면 Space, 누르고 있으면 편집 레이어.
#define A_CTL  MT(MOD_LCTL, KC_A)
#define S_ALT  MT(MOD_LALT, KC_S)
#define D_GUI  MT(MOD_LGUI, KC_D)
#define F_SFT  MT(MOD_LSFT, KC_F)
#define J_SFT  MT(MOD_RSFT, KC_J)
#define K_GUI  MT(MOD_RGUI, KC_K)
#define L_RALT MT(MOD_RALT, KC_L)
#define L_LALT MT(MOD_LALT, KC_L)
#define ENT_CTL MT(MOD_RCTL, KC_ENT)
#define Z_SFT  MT(MOD_LSFT, KC_Z)
#define SLSH_SFT MT(MOD_RSFT, KC_SLSH)
// SPC_NAV(QK_KB_14) 는 thumb.h 에 정의됨 (200ms normal mode)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_MAC] = LAYOUT(
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
        KC_CAPS, A_CTL, S_ALT, D_GUI, F_SFT, KC_G, KC_H, J_SFT, K_GUI, L_RALT, ENT_CTL,
        KC_LSFT, SLSH_SFT, Z_SFT, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT, KC_BSPC, THUMB, SPC_NAV, HANGUL, KC_APP, KC_RCTL
    ),
    [LAYER_WIN] = LAYOUT(
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
        KC_CAPS, A_CTL, S_ALT, D_GUI, F_SFT, KC_G, KC_H, J_SFT, K_GUI, L_LALT, ENT_CTL,
        KC_LSFT, SLSH_SFT, Z_SFT, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT, KC_BSPC, THUMB, SPC_NAV, HANGUL, KC_APP, KC_RCTL
    ),
    [LAYER_NUM] = LAYOUT(
        KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS,
        KC_ESC, KC_LSFT, KC_LALT, KC_LGUI, KC_LCTL, KC_GRV, KC_MINS, KC_EQL, KC_LBRC, KC_RBRC, KC_BSLS,
        KC_LSFT, KC_SLSH, S(KC_9), S(KC_0), S(KC_MINS), S(KC_EQL), S(KC_SCLN), KC_SCLN, KC_QUOT, KC_COMM, KC_DOT, _______,
        _______, _______, _______, KC_ESC, _______, _______, _______, _______, _______
    ),
    [LAYER_SYM] = LAYOUT(
        S(KC_GRV), S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5), S(KC_6), S(KC_7), S(KC_8), S(KC_9), S(KC_0), S(KC_MINS),
        _______, S(KC_SLSH), S(KC_SCLN), S(KC_QUOT), KC_QUOT, S(KC_GRV), S(KC_MINS), S(KC_EQL), S(KC_LBRC), S(KC_RBRC), S(KC_BSLS),
        _______, S(KC_SLSH), _______, _______, _______, _______, _______, S(KC_SCLN), S(KC_QUOT), S(KC_COMM), S(KC_DOT), _______,
        _______, _______, _______, _______, _______, _______, KC_LSFT /* 원샷 취소(thumb.c) */, _______, _______
    ),
    [LAYER_NAV] = LAYOUT(
        _______, KC_ESC, KC_NO, KC_UP, KC_NO, KC_NO, KC_HOME, KC_PGUP, KC_NO, KC_NO, KC_NO, _______,
        _______, KC_TAB, KC_LEFT, KC_DOWN, KC_RGHT, KC_NO, KC_END, KC_PGDN, KC_NO, KC_NO, KC_NO,
        _______, KC_NO, S(KC_TAB), KC_NO, KC_NO, KC_NO, KC_NO, KC_BSPC, KC_DEL, KC_NO, KC_NO, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [LAYER_FN] = LAYOUT(
        KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_NO,
        QK_KB_7 /* BAT */, QK_KB_0 /* BT1 */, QK_KB_1 /* BT2 */, QK_KB_2 /* BT3 */, QK_KB_5 /* 2.4G */, KC_F11, KC_F12, KC_MPLY, KC_VOLD, KC_VOLU, QK_KB_8 /* CHRG */,
        KC_NO, UG_NEXT, QK_BOOT, UG_TOGG, UG_VALD, UG_VALU, PDF(LAYER_MAC), PDF(LAYER_WIN), KC_MUTE, KC_MPRV, KC_MNXT, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, _______, _______, KC_NO, KC_NO, KC_NO
    ),
    [LAYER_SYM_WIN] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [7] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______
    )
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, LAYER_NUM, LAYER_NAV, LAYER_FN);
}
