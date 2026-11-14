// SPDX-License-Identifier: GPL-2.0-or-later
// 상태 강조 및 언어 전환 애니메이션
// 활성 레이어 및 수식키에 따라 사용 가능한 키 LED 점등
#include QMK_KEYBOARD_H
#include "ws2812.h"
#include "thumb.h"

// WS2812 LED 프레임 버퍼
extern ws2812_led_t ws2812_leds[];

#define FLASH_RISE_MS 300
#define FLASH_HOLD_MS 150
#define FLASH_SWEEP_MS 1100
// 스윕 애니메이션 밴드 폭
#define SWEEP_EDGE 16
#define SWEEP_SOLID 70
#define SWEEP_TAIL 30
#define SWEEP_BAND (SWEEP_SOLID + SWEEP_TAIL)
#define SWEEP_R 110
#define SWEEP_G 200
#define SWEEP_B 255
#define FLASH_G 220
// Windows 모드 언어 전환 색상
#define FLASH_WIN_G 150
#define FLASH_WIN_B 220

static void highlight(uint8_t led_min, uint8_t led_max);

static bool     flash_on;
static uint32_t flash_at;

enum { HL_NONE, HL_LAYER, HL_HANGUL, HL_MAIN, HL_SUB, HL_ALT, HL_SHIFT };

#define MODE_BLINK_HALF_MS 250

// 수식 조합 키에서 기본 키코드 추출
static uint16_t basic_kc(uint16_t kc) {
    if (IS_QK_MOD_TAP(kc)) return QK_MOD_TAP_GET_TAP_KEYCODE(kc);
    if (IS_QK_LAYER_TAP(kc)) return QK_LAYER_TAP_GET_TAP_KEYCODE(kc);
    if (IS_QK_MODS(kc)) return QK_MODS_GET_BASIC_KEYCODE(kc);
    return kc;
}

static bool is_win_ctrl_key(uint16_t kc) {
    switch (kc) {
        case KC_A: case KC_C: case KC_V: case KC_X: case KC_Z: case KC_Y: case KC_S: case KC_F: case KC_N:
        case KC_T: case KC_W: case KC_R: case KC_P: case KC_O: case KC_H: case KC_TAB: case KC_LEFT: case KC_RGHT:
            return true;
        default:
            return false;
    }
}

static bool is_alt_key(uint16_t kc, bool win) {
    switch (kc) {
        case KC_LEFT: case KC_RGHT:
            return true;
        case KC_BSPC:
            return !win;
        case KC_TAB: case KC_F4:
            return win;
        default:
            return false;
    }
}

static bool is_win_gui_key(uint16_t kc) {
    switch (kc) {
        case KC_D: case KC_E: case KC_L: case KC_R: case KC_I: case KC_S: case KC_V: case KC_TAB:
        case KC_LEFT: case KC_RGHT: case KC_UP: case KC_DOWN:
            return true;
        default:
            return false;
    }
}

static bool is_cmd_key(uint16_t kc) {
    switch (kc) {
        case KC_A: case KC_C: case KC_V: case KC_X: case KC_Z: case KC_S: case KC_F: case KC_N:
        case KC_T: case KC_W: case KC_R: case KC_P: case KC_Q: case KC_H: case KC_M: case KC_COMM:
        case KC_GRV: case KC_SPC: case KC_TAB: case KC_LEFT: case KC_RGHT:
            return true;
        default:
            return false;
    }
}

static bool is_ctrl_key(uint16_t kc) {
    switch (kc) {
        case KC_B: case KC_D: case KC_S: case KC_C: case KC_Z: case KC_L: case KC_R: case KC_A:
        case KC_E: case KC_U: case KC_K: case KC_W: case KC_P: case KC_N: case KC_LEFT: case KC_RGHT:
        case KC_UP: case KC_TAB:
            return true;
        default:
            return false;
    }
}

static bool is_shiftable(uint16_t kc) {
    return (kc >= KC_A && kc <= KC_0) || (kc >= KC_MINS && kc <= KC_SLSH) || kc == KC_NUBS;
}

// 단일 언어 전환 키코드 판정 (맥 Option/Alt 제외)
static bool is_lang_switch(uint16_t kc) {
    switch (kc) {
        case KC_CAPS: case KC_RALT: case KC_LNG1: case KC_LNG2: case KC_INT2: case KC_INT4: case KC_INT5:
            return true;
        default:
            return false;
    }
}

void highlight_flash(void) {
    flash_on = true;
    flash_at = timer_read32();
}

void highlight_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_RALT && get_highest_layer(default_layer_state) != LAYER_WIN) return;
    if (is_lang_switch(keycode) && record->event.pressed) highlight_flash();
}

// smoothstep: 0~256 -> 0~256
static uint32_t smooth(uint32_t t) {
    if (t > 256) t = 256;
    return t * t * (768 - 2 * t) / 65536;
}

static uint8_t dim(uint8_t c, uint8_t v) {
    return (uint16_t)c * v / 255;
}

static uint8_t mix8(uint8_t a, uint8_t b, uint32_t k) {
    return (a * (256 - k) + b * k) / 256;
}

static void overlay_flash(uint8_t led_min, uint8_t led_max) {
    if (!flash_on) return;
    // 기본 레이어 외 진입 시 한/영 애니메이션 중단
    if (get_highest_layer(layer_state) >= LAYER_NUM) {
        flash_on = false;
        return;
    }
    uint32_t ms = timer_elapsed32(flash_at);
    if (ms >= FLASH_RISE_MS + FLASH_HOLD_MS + FLASH_SWEEP_MS) {
        flash_on = false;
        return;
    }
    uint8_t v  = rgb_matrix_get_val();
    bool    win_mode = get_highest_layer(default_layer_state) == LAYER_WIN;
    uint8_t fg = dim(win_mode ? FLASH_WIN_G : FLASH_G, v), fb = win_mode ? dim(FLASH_WIN_B, v) : 0, sr = dim(SWEEP_R, v), sg = dim(SWEEP_G, v), sb = dim(SWEEP_B, v);
    int32_t front = 0;
    uint32_t k    = 256;
    bool     sweep = ms >= FLASH_RISE_MS + FLASH_HOLD_MS;
    if (sweep) {
        // 밴드 앞 가장자리 이동
        uint32_t t    = (ms - FLASH_RISE_MS - FLASH_HOLD_MS) * 256 / FLASH_SWEEP_MS;
        int32_t  span = 224 + SWEEP_BAND + SWEEP_EDGE;
        front         = -SWEEP_EDGE + (int32_t)smooth(t) * span / 256;
    } else if (ms < FLASH_RISE_MS) {
        k = smooth(ms * 256 / FLASH_RISE_MS);
    }
    for (uint8_t i = led_min; i < led_max; i++) {
        uint8_t r0 = ws2812_leds[i].r, g0 = ws2812_leds[i].g, b0 = ws2812_leds[i].b;
        uint8_t r, g, b;
        if (!sweep) {
            r = mix8(r0, 0, k), g = mix8(g0, fg, k), b = mix8(b0, fb, k);
        } else {
            int32_t u = front - g_led_config.point[i].x;
            if (u < -SWEEP_EDGE) {
                r = 0, g = fg, b = fb;
            } else if (u < 0) {
                uint32_t e = smooth((u + SWEEP_EDGE) * 256 / SWEEP_EDGE);
                r = mix8(0, sr, e), g = mix8(fg, sg, e), b = mix8(fb, sb, e);
            } else if (u < SWEEP_SOLID) {
                r = sr, g = sg, b = sb;
            } else if (u < SWEEP_BAND) {
                uint32_t e = smooth((u - SWEEP_SOLID) * 256 / SWEEP_TAIL);
                r = mix8(sr, r0, e), g = mix8(sg, g0, e), b = mix8(sb, b0, e);
            } else {
                continue;
            }
        }
        rgb_matrix_set_color(i, r, g, b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    highlight(led_min, led_max);
    overlay_flash(led_min, led_max);
    return true;
}

static void highlight(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t mods  = get_mods();
    bool    win   = get_highest_layer(default_layer_state) == LAYER_WIN;
    uint8_t mainm = win ? MOD_MASK_CTRL : MOD_MASK_GUI;
    uint8_t subm  = win ? MOD_MASK_GUI : MOD_MASK_CTRL;
    uint8_t state = layer >= LAYER_NUM                         ? HL_LAYER
                    : hangul_held()                            ? HL_HANGUL
                    : (mods & mainm)                           ? HL_MAIN
                    : (mods & subm)                            ? HL_SUB
                    : (mods & MOD_MASK_ALT)                    ? HL_ALT
                    : (mods & MOD_MASK_SHIFT)                  ? HL_SHIFT
                                                               : HL_NONE;
    if (state == HL_NONE) return;

    uint8_t r, g, b;
    switch (state) {
        case HL_LAYER:
            if (layer == LAYER_NUM) {
                r = 70, g = 0, b = 255;
            } else if (layer == LAYER_SYM || layer == LAYER_SYM_WIN) {
                r = 162, g = 0, b = 255;
            } else {
                r = 255, g = 0, b = 116;
            }
            break;
        case HL_HANGUL:
            r = 0, g = win ? FLASH_WIN_G : FLASH_G, b = win ? FLASH_WIN_B : 0;
            break;
        case HL_MAIN:
            r = 255, g = 0, b = 0;
            break;
        case HL_SUB:
            r = 200, g = 200, b = 200;
            break;
        case HL_ALT:
            r = 0, g = 200, b = 160;
            break;
        default:
            r = 255, g = 100, b = 0;
            break;
    }

    uint8_t v = rgb_matrix_get_val();
    r = dim(r, v), g = dim(g, v), b = dim(b, v);

    for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t i = g_led_config.matrix_co[row][col];
            if (i == NO_LED || i < led_min || i >= led_max) continue;
            keypos_t pos = {.row = row, .col = col};
            bool     on;
            if (state == HL_LAYER) {
                uint16_t kc = keymap_key_to_keycode(layer, pos);
                on          = kc != KC_TRNS && kc != KC_NO;
                // 현재 기본 모드 전환 키 점멸
                if (on && IS_QK_PERSISTENT_DEF_LAYER(kc) && QK_PERSISTENT_DEF_LAYER_GET_LAYER(kc) == get_highest_layer(default_layer_state) && (timer_read() / MODE_BLINK_HALF_MS) % 2) on = false;
            } else {
                // 하위 레이어 투명 키 처리
                uint16_t kc = basic_kc(keymap_key_to_keycode(layer_switch_get_layer(pos), pos));
                if (state == HL_HANGUL) {
                    on = kc >= KC_A && kc <= KC_Z;
                } else if (state == HL_SHIFT) {
                    on = is_shiftable(kc);
                } else if (state == HL_ALT) {
                    on = is_alt_key(kc, win);
                } else if (state == HL_MAIN) {
                    on = win ? is_win_ctrl_key(kc) : is_cmd_key(kc);
                } else {
                    on = win ? is_win_gui_key(kc) : is_ctrl_key(kc);
                }
            }
            if (on) rgb_matrix_set_color(i, r, g, b);
        }
    }
}
