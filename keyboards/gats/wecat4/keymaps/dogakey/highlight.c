// SPDX-License-Identifier: GPL-2.0-or-later
// 상태 강조: 레이어·Ctrl/Cmd·Shift 가 눌려 있는 동안 그 상태에서 쓸 수 있는 키만 켜고 나머지는 끈다.
// 키보드는 OS 를 모르므로 기본 레이어(Windows 0 / macOS 1, Fn+오른쪽 Shift 로 전환)로 목록을 고른다.
//   레이어 Fn/3/4+ : 그 레이어에 배정된 키(투명·KC_NO 제외)       남보라 / 보라 / 분홍
//   주 단축키 수식(Windows Ctrl, macOS Cmd)                          빨강
//     Windows Ctrl : A C V X Z Y S F N T W R P O H Tab ← →
//     macOS Cmd    : A C V X Z S F N T W R P Q H M , ` Space Tab ← →
//   보조 수식(Windows Win, macOS Ctrl)                               흰색
//     Windows Win  : D E L R I S V Tab ← → ↑ ↓
//     macOS Ctrl   : tmux·셸·줄 편집 B D S C Z L R A E U K W P N ← → ↑ Tab
//   Shift          : Shift 로 결과가 바뀌는 키(글자·숫자·기호)     노랑(주황빛) — 거의 모든 키가 켜지므로 덜 밝게
// 우선순위 레이어 > 주 수식 > 보조 수식 > Shift. Alt 는 강조하지 않는다.
// 판정은 매 프레임 현재 키맵(Vial 편집 반영)을 읽어서 하므로 키를 옮겨도 다시 구울 필요가 없다.
//
// 한/영 번쩍임: 언어 전환 키(CapsLock·RAlt·Lang1/2·일본어 키, is_lang_switch)를 누르면 원래 조명 → 초록으로 smoothstep 교차 전환(FLASH_RISE_MS)해 FLASH_HOLD_MS 유지한 뒤,
// 연한 하늘색 띠가 왼쪽에서 오른쪽으로 FLASH_SWEEP_MS 동안 흘러 초록을 오른쪽으로 밀어내고 띠 뒤로 원래 조명이
// 돌아온다(순간 점멸·즉시 복귀는 눈이 피로함).
// 강조색·번쩍임 색은 현재 RGB 밝기에 비례해 줄인다(효과와 같은 기준) — 고정값으로 칠하면 강조 중에는 밝기 키가 듣지 않는다.
// 배터리·연결 표시는 경고라 밝기와 무관하게 그대로 둔다.
// 강조 위에 덮고, 배터리·연결 표시(wireless_glue.c)는 그보다 위에 그린다. 키보드는 IME 상태를 모르므로
// "눌렀다"만 알린다. 컴퓨터로 보내는 CapsLock 은 그대로.
#include QMK_KEYBOARD_H
#include "ws2812.h"

// 드라이버(platforms/chibios/drivers/ws2812_*.c)의 프레임 버퍼. 헤더에 선언이 없어 직접 선언한다.
extern ws2812_led_t ws2812_leds[];

#define FLASH_RISE_MS 300
#define FLASH_HOLD_MS 150
#define FLASH_SWEEP_MS 1100
// 밀어내는 띠: 앞 가장자리(초록→하늘색), 진한 하늘색 구간, 꼬리(하늘색→원래 조명) 폭, LED x 좌표 단위(0~224).
// 띠 전체를 꼬리로 녹이면 진하게 보이는 부분이 앞쪽 1/3 뿐이라 진한 구간을 따로 둔다.
#define SWEEP_EDGE 16
#define SWEEP_SOLID 70
#define SWEEP_TAIL 30
#define SWEEP_BAND (SWEEP_SOLID + SWEEP_TAIL)
#define SWEEP_R 110
#define SWEEP_G 200
#define SWEEP_B 255
// 채널 하나만 쓰는 초록이 같은 전류에서 체감 밝기가 가장 높다. 47개를 한꺼번에 켜므로 전류를 고려해 최대값보다 조금 낮춘다.
#define FLASH_G 220

static void highlight(uint8_t led_min, uint8_t led_max);

static bool     flash_on;
static uint32_t flash_at;

enum { HL_NONE, HL_LAYER, HL_MAIN, HL_SUB, HL_SHIFT };

// 탭홀드·수식 조합 키코드는 탭 쪽 기본 키코드로 판정한다.
static uint16_t basic_kc(uint16_t kc) {
    if (IS_QK_MOD_TAP(kc)) return QK_MOD_TAP_GET_TAP_KEYCODE(kc);
    if (IS_QK_LAYER_TAP(kc)) return QK_LAYER_TAP_GET_TAP_KEYCODE(kc);
    if (IS_QK_MODS(kc)) return QK_MODS_GET_BASIC_KEYCODE(kc);
    return kc;
}

// Windows Ctrl: 복사·붙여넣기·실행 취소·다시 실행·저장·찾기·새로 만들기·탭·창 닫기·새로 고침·인쇄·열기·바꾸기, 단어 이동
static bool is_win_ctrl_key(uint16_t kc) {
    switch (kc) {
        case KC_A: case KC_C: case KC_V: case KC_X: case KC_Z: case KC_Y: case KC_S: case KC_F: case KC_N:
        case KC_T: case KC_W: case KC_R: case KC_P: case KC_O: case KC_H: case KC_TAB: case KC_LEFT: case KC_RGHT:
            return true;
        default:
            return false;
    }
}

// Windows Win: 바탕 화면·탐색기·잠금·실행·설정·검색·클립보드 기록, 작업 보기, 창 배치(←→↑↓)
static bool is_win_gui_key(uint16_t kc) {
    switch (kc) {
        case KC_D: case KC_E: case KC_L: case KC_R: case KC_I: case KC_S: case KC_V: case KC_TAB:
        case KC_LEFT: case KC_RGHT: case KC_UP: case KC_DOWN:
            return true;
        default:
            return false;
    }
}

// Cmd(Gui): macOS 공통 단축키
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

// Ctrl(macOS 기준): tmux(B 접두키, D 분리·EOF, S 세션), 셸(C 중지, Z 일시정지, L 지우기, R 기록 검색),
// 줄 편집(A/E 처음·끝, U/K/W 지우기, P/N 이전·다음 명령), 시스템(←→ 데스크톱, ↑ Mission Control, Tab 탭 전환)
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

// 언어 전환으로 쓰이는 단일 키: macOS CapsLock(짧게), Windows 한국어 입력기 RAlt, 한/영·한자(LNG1/LNG2),
// 일본어 かな(INT2)·変換(INT4)·無変換(INT5). 키보드는 OS 를 모르므로 모두 번쩍인다(macOS 에서 RAlt 를 눌러도 번쩍임).
// LAlt 는 조합키로 자주 써서, Win+Space·Ctrl+Space 같은 조합은 다른 용도와 겹쳐서 제외.
static bool is_lang_switch(uint16_t kc) {
    switch (kc) {
        case KC_CAPS: case KC_RALT: case KC_LNG1: case KC_LNG2: case KC_INT2: case KC_INT4: case KC_INT5:
            return true;
        default:
            return false;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (is_lang_switch(keycode) && record->event.pressed) {
        flash_on = true;
        flash_at = timer_read32();
    }
    return true;
}

// smoothstep: 0~256 → 0~256, 양 끝에서 변화가 0 이라 전환이 부드럽다
static uint32_t smooth(uint32_t t) {
    if (t > 256) t = 256;
    return t * t * (768 - 2 * t) / 65536;
}

// 고정 색 한 채널을 현재 밝기 비율로 줄인다
static uint8_t dim(uint8_t c, uint8_t v) {
    return (uint16_t)c * v / 255;
}

static uint8_t mix8(uint8_t a, uint8_t b, uint32_t k) {
    return (a * (256 - k) + b * k) / 256;
}

// 번쩍임 진행 중이면 지금 프레임(효과 + 강조)에 시간표대로 초록·하늘색 띠를 섞는다
static void overlay_flash(uint8_t led_min, uint8_t led_max) {
    if (!flash_on) return;
    uint32_t ms = timer_elapsed32(flash_at);
    if (ms >= FLASH_RISE_MS + FLASH_HOLD_MS + FLASH_SWEEP_MS) {
        flash_on = false;
        return;
    }
    uint8_t v  = rgb_matrix_get_val();
    uint8_t fg = dim(FLASH_G, v), sr = dim(SWEEP_R, v), sg = dim(SWEEP_G, v), sb = dim(SWEEP_B, v);
    int32_t front = 0;
    uint32_t k    = 256;
    bool     sweep = ms >= FLASH_RISE_MS + FLASH_HOLD_MS;
    if (sweep) {
        // 띠 앞 가장자리: 왼쪽 밖(-EDGE)에서 띠 꼬리가 오른쪽 밖으로 나갈 때까지, 양 끝은 천천히
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
            r = mix8(r0, 0, k), g = mix8(g0, fg, k), b = mix8(b0, 0, k);
        } else {
            // u: 앞 가장자리에서 뒤로 떨어진 거리(음수 = 아직 밀리지 않은 초록 쪽)
            int32_t u = front - g_led_config.point[i].x;
            if (u < -SWEEP_EDGE) {
                r = 0, g = fg, b = 0;
            } else if (u < 0) {
                uint32_t e = smooth((u + SWEEP_EDGE) * 256 / SWEEP_EDGE);
                r = mix8(0, sr, e), g = mix8(fg, sg, e), b = mix8(0, sb, e);
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
    bool    mac   = get_highest_layer(default_layer_state) == LAYER_MAC;
    uint8_t mainm = mac ? MOD_MASK_GUI : MOD_MASK_CTRL;
    uint8_t subm  = mac ? MOD_MASK_CTRL : MOD_MASK_GUI;
    uint8_t state = layer >= LAYER_FN                          ? HL_LAYER
                    : (mods & mainm)                           ? HL_MAIN
                    : (mods & subm)                            ? HL_SUB
                    : (mods & MOD_MASK_SHIFT)                  ? HL_SHIFT
                                                               : HL_NONE;
    if (state == HL_NONE) return;

    uint8_t r, g, b;
    switch (state) {
        case HL_LAYER: // 보라 계열 한 가지로 묶고 단계만 바꾼다(언어 전환 번쩍임의 초록·하늘색과 겹치지 않게)
            if (layer == LAYER_FN) {
                r = 70, g = 0, b = 255; // 남보라
            } else if (layer == LAYER_FN + 1) {
                r = 162, g = 0, b = 255; // 보라
            } else {
                r = 255, g = 0, b = 116; // 분홍
            }
            break;
        case HL_MAIN: // Windows Ctrl / macOS Cmd
            r = 255, g = 0, b = 0;
            break;
        case HL_SUB: // Windows Win / macOS Ctrl, 흰색(밝게 보여 조금 낮춤)
            r = 200, g = 200, b = 200;
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
            } else {
                // 투명 키는 아래 레이어로 내려가 실제로 입력될 키를 본다.
                uint16_t kc = basic_kc(keymap_key_to_keycode(layer_switch_get_layer(pos), pos));
                if (state == HL_SHIFT) {
                    on = is_shiftable(kc);
                } else if (state == HL_MAIN) {
                    on = mac ? is_cmd_key(kc) : is_win_ctrl_key(kc);
                } else {
                    on = mac ? is_ctrl_key(kc) : is_win_gui_key(kc);
                }
            }
            if (on) rgb_matrix_set_color(i, r, g, b);
        }
    }
}
