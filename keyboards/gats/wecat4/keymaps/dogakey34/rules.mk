VIA_ENABLE     = yes
VIAL_ENABLE    = yes
VIALRGB_ENABLE = yes
DEBOUNCE_TYPE  = sym_eager_pk
TAP_DANCE_ENABLE    = yes
COMBO_ENABLE        = yes
KEY_OVERRIDE_ENABLE = yes
CAPS_WORD_ENABLE    = yes

# 제조사 무선 모듈 연동 (USB 연결 시 절전 억제는 wireless_glue.c에서 처리)
include keyboards/gats/wecat4/wireless/wireless.mk
SRC += wireless_glue.c highlight.c thumb.c

