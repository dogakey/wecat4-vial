VIA_ENABLE     = yes
VIAL_ENABLE    = yes
VIALRGB_ENABLE = yes
DEBOUNCE_TYPE  = sym_eager_pk
TAP_DANCE_ENABLE    = yes
COMBO_ENABLE        = yes
KEY_OVERRIDE_ENABLE = yes
CAPS_WORD_ENABLE    = yes

# 무선: 제조사 keyboards/wireless(libmodule.a v0.2.6) 그대로. STOP 딥슬립은 제조사 기본값(켬) 유지 —
# lowpower.c가 STOP 함수를 무조건 호출해 끌 수 없음. USB 연결 중엔 wireless_glue.c 훅으로 절전 진입 안 함.
include keyboards/gats/wecat4/wireless/wireless.mk
SRC += wireless_glue.c highlight.c thumb.c

