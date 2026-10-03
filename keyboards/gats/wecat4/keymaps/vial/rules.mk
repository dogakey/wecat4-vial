VIA_ENABLE     = yes
VIAL_ENABLE    = yes
VIALRGB_ENABLE = yes
DEBOUNCE_TYPE  = sym_defer_pk
TAP_DANCE_ENABLE    = yes
COMBO_ENABLE        = yes
KEY_OVERRIDE_ENABLE = yes
CAPS_WORD_ENABLE    = yes

# 무선: 제조사 keyboards/wireless(libmodule.a v0.2.6) 그대로. 저장소에는 libmodule.a 가 없으므로
# 빌드 전에 scripts/fetch_libmodule.sh 로 받는다.
ifeq ($(wildcard keyboards/gats/wecat4/wireless/libmodule.a),)
    $(error libmodule.a 가 없습니다. scripts/fetch_libmodule.sh 를 먼저 실행하세요)
endif
# STOP 딥슬립은 제조사 기본값(켬) 유지 — lowpower.c 가 STOP 함수를 무조건 호출해 끌 수 없다.
# USB 연결 중엔 wireless_glue.c 훅으로 절전에 들어가지 않는다.
include keyboards/gats/wecat4/wireless/wireless.mk
SRC += wireless_glue.c

# 진단 빌드: qmk compile ... -e WECAT4_DIAG=yes
# 무선 모드에서도 USB 를 유지하고 raw HID 0xD1 로 무선·절전 상태를 읽는다(diag.c).
ifeq ($(strip $(WECAT4_DIAG)), yes)
    OPT_DEFS += -DWECAT4_DIAG -DKEEP_USB_CONNECTION_IN_WIRELESS_MODE
    SRC += diag.c
endif
