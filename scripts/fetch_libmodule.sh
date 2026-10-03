#!/usr/bin/env bash
# 제조사(Hangsheng) 무선 모듈 라이브러리 libmodule.a 를 제조사 공개 저장소의 고정 커밋에서 받아 sha256 을 확인한다.
# 소스가 공개되지 않은 바이너리라 이 저장소에는 넣지 않는다.
set -euo pipefail

COMMIT=580665f77746c373ada7f79a4799f20a4f2f6c6d
URL="https://raw.githubusercontent.com/hangshengkeji/qmk_firmware/${COMMIT}/keyboards/wireless/libmodule.a"
SHA256=d150f5dca6f95ee7d3b8acb218827aacd92c0f711cb5e05ec655908560cb6f0b

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/keyboards/gats/wecat4/wireless/libmodule.a"

sha256_of() {
    if command -v sha256sum >/dev/null; then sha256sum "$1" | cut -d' ' -f1; else shasum -a 256 "$1" | cut -d' ' -f1; fi
}

if [ -f "$DEST" ] && [ "$(sha256_of "$DEST")" = "$SHA256" ]; then
    echo "이미 있음: $DEST"
    exit 0
fi

tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT
curl -fsSL -o "$tmp" "$URL"
got="$(sha256_of "$tmp")"
if [ "$got" != "$SHA256" ]; then
    echo "sha256 불일치: $got (기대값 $SHA256)" >&2
    exit 1
fi
mv "$tmp" "$DEST"
trap - EXIT
echo "받음: $DEST"
