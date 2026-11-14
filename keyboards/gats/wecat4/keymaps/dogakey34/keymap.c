// Copyright 2026 dogakey
// SPDX-License-Identifier: GPL-2.0-or-later
// 위캣4 34키 키맵 (0:macOS, 1:Windows, 2:숫자, 3:기호, 4:편집, 5:기능)
// QK_KB_0~11: wireless_glue.c, HANGUL/THUMB/SPC_NAV: thumb.c
// RGB_M_* 는 RGB Matrix에서 동작하지 않을 수 있음(Vial 교체 권장)

#include QMK_KEYBOARD_H
#include "thumb.h"

// 홈로우 수식키 (A Ctrl, S Alt, D Gui, F Shift / J Shift, K Gui, L Alt, Enter Ctrl)
// 윈도우 L은 RAlt 한/영 간섭 방지를 위해 LAlt 지정
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
        _______, _______, _______, _______, _______, _______, KC_LSFT /* 원샷 취소 */, _______, _______
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
