#pragma once
// ChibiOS HAL 설정 (시리얼 통신 및 콜백 사용)
#define HAL_USE_SERIAL    TRUE
#define PAL_USE_CALLBACKS TRUE
#include_next <halconf.h>
