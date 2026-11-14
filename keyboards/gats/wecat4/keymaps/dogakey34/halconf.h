#pragma once
// 제조사 dr6p halconf와 같은 값(SPI는 위캣4 정의에서 안 쓰므로 제외)
#define HAL_USE_SERIAL    TRUE
#define PAL_USE_CALLBACKS TRUE
#include_next <halconf.h>
