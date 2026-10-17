// Copyright 2026 dogakey
// SPDX-License-Identifier: GPL-2.0-or-later
//
// 위캣4 기본 키맵(유선 전용). 무선 빌드(keymaps/vial)와 같은 배치에서 무선·배터리 키만 뺐다.
// 공장 초기화는 Fn+LCtrl(EE_CLR, 누르는 즉시).
// 레이어 0(Windows)·1(macOS) 은 처음에는 같고, Fn+오른쪽 Shift 로 기본 레이어를 고른다(EEPROM 저장).

#define WINMAC  QK_KB_0 // Windows ↔ macOS 기본 레이어 전환(vial.json customKeycodes 0번)

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_WIN] = LAYOUT(
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,             KC_ENT,
        KC_LSFT, KC_SLSH, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT,          KC_SPC,  MO(LAYER_FN),     KC_SPC,           KC_RALT, KC_APP,  KC_RCTL
    ),
    [LAYER_MAC] = LAYOUT(
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,             KC_ENT,
        KC_LSFT, KC_SLSH, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT,          KC_SPC,  MO(LAYER_FN),     KC_SPC,           KC_RALT, KC_APP,  KC_RCTL
    ),
    // Fn: 숫자·기호, 방향키(. / App / RAlt / RCtl), 조명(Z 효과, X·C 속도, B·M 색상, N·H 밝기, J·G 채도),
    //     LCtrl 공장 초기화, Win 잠금
    [LAYER_FN] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    _______,
        _______, _______, _______, _______, _______, RM_SATU, RM_VALU, RM_SATD, KC_MINS, KC_EQL,           _______,
        _______, KC_GRV,  RM_NEXT, RM_SPDD, RM_SPDU, _______, RM_HUED, RM_VALD, RM_HUEU, _______, KC_UP,   WINMAC,
        EE_CLR,  GU_TOGG, _______,          _______, _______,          _______,          KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [3] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______,          _______, _______,          _______,          _______, _______, _______
    ),
    [4] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______,          _______, _______,          _______,          _______, _______, _______
    ),
    [5] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______,          _______, _______,          _______,          _______, _______, _______
    ),
    [6] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______,          _______, _______,          _______,          _______, _______, _______
    ),
    [7] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______,          _______, _______,          _______,          _______, _______, _______
    )
};
// clang-format on

#include "usb_main.h"

// 전환 직후 이 시간 동안 다른 불을 끄고 W(Windows) 또는 M(macOS) 키 하나만 켜서 바뀐 모드를 알린다.
#define OS_SHOW_MS 1000
// Fn 을 누르고 있는 동안 현재 모드 키 깜박임 반주기(켜짐·꺼짐 각각), 초당 2회
#define MODE_BLINK_HALF_MS 250
static bool     os_show;
static uint32_t os_shown_at;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == WINMAC) {
        if (record->event.pressed) {
            set_single_persistent_default_layer(get_highest_layer(default_layer_state) == LAYER_MAC ? LAYER_WIN : LAYER_MAC);
            os_show     = true;
            os_shown_at = timer_read32();
        }
        return false;
    }
    return true;
}

static void set_key_led(uint8_t row, uint8_t col, uint8_t led_min, uint8_t led_max) {
    uint8_t i = g_led_config.matrix_co[row][col];
    if (i != NO_LED && i >= led_min && i < led_max) rgb_matrix_set_color(i, 255, 255, 255);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // 컴퓨터가 키보드를 쓰고 있지 않으면(충전기·잠든 컴퓨터) 조명을 끈다.
    if (USB_DRIVER.state != USB_ACTIVE) {
        for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
        return false;
    }
    if (os_show) {
        if (timer_elapsed32(os_shown_at) < OS_SHOW_MS) {
            for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
            if (get_highest_layer(default_layer_state) == LAYER_MAC) {
                set_key_led(2, 8, led_min, led_max); // M
            } else {
                set_key_led(0, 2, led_min, led_max); // W
            }
            return false;
        }
        os_show = false;
    }
    // Fn 을 누르고 있는 동안 지금 모드 키(Windows W / macOS M)를 초당 2회 흰색으로 깜박여 현재 모드를 알린다.
    if (get_highest_layer(layer_state) >= LAYER_FN) {
        bool    mac = get_highest_layer(default_layer_state) == LAYER_MAC;
        uint8_t i   = g_led_config.matrix_co[mac ? 2 : 0][mac ? 8 : 2];
        if (i != NO_LED && i >= led_min && i < led_max) {
            uint8_t c = (timer_read32() / MODE_BLINK_HALF_MS) % 2 ? 255 : 0;
            rgb_matrix_set_color(i, c, c, c);
        }
    }
    return true;
}
