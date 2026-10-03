# 원저작자·외부 구성 요소

이 저장소는 GPL-2.0-or-later 입니다. 아래 구성 요소의 저작권은 각 원저작자에게 있습니다.

| 경로 | 출처 | 라이선스 |
|---|---|---|
| `keyboards/gats/wecat4/keyboard.json`, `wecat4.c` | Hangsheng(sdk66) [`hangshengkeji/qmk_firmware`](https://github.com/hangshengkeji/qmk_firmware) master `keyboards/leo/wecat4` | GPL-2.0-or-later |
| `keyboards/gats/wecat4/wireless/*.c`, `*.h`, `wireless.mk` | Su(@isuua), JoyLee(@itarze), QMK — 같은 저장소 tri-mode 브랜치 `580665f` `keyboards/wireless`. 최신 QMK 에 맞게 일부 수정 | GPL-2.0-or-later |
| `keyboards/gats/wecat4/keymaps/*` | 이 저장소 | GPL-2.0-or-later |
| `libmodule.a`(저장소에 없음) | Hangsheng 무선 모듈 라이브러리 v0.2.6, 같은 커밋 `keyboards/wireless/libmodule.a`, sha256 `d150f5dc…6f0b` | 소스 미공개, 라이선스 표기 없음 |

## 무선 라이브러리(libmodule.a)

- 무선 칩과 통신하는 부분은 제조사가 소스 없이 공개한 바이너리입니다. 이 저장소에는 넣지 않고, `scripts/fetch_libmodule.sh`가 제조사 저장소의 고정 커밋에서 받아 sha256을 확인합니다.
- Releases의 무선 포함 펌웨어(`gats_wecat4_vial.bin`)에는 이 라이브러리가 링크돼 있습니다. 라이브러리 없이 GPL 소스만으로 만든 펌웨어가 필요하면 유선 전용 `gats_wecat4_vial_wired.bin`을 쓰세요.

## 상표

GUTS·WECAT·가츠·위캣은 각 권리자의 상표이며, 이 저장소는 권리자와 관계가 없습니다. USB VID/PID(`0x342D`/`0xE47E`)는 원래 펌웨어와 같은 값으로, 기존 장치 드라이버·도구와 호환되도록 그대로 두었습니다.
