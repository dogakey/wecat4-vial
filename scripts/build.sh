#!/usr/bin/env bash
# vial-qmk 작업 사본에 이 저장소의 키보드 정의를 복사하고 빌드한다.
#   사용법: QMK_HOME=<vial-qmk 경로> scripts/build.sh [vial|vial_wired] [qmk compile 추가 인자...]
#   결과:   build/gats_wecat4_<keymap>.bin 과 SHA256SUMS
# 검증한 vial-qmk 커밋은 아래 VIAL_QMK_COMMIT 이다. 다른 커밋이면 경고만 하고 계속한다.
set -euo pipefail

VIAL_QMK_COMMIT=dd43959ae5c08d8a28d38a1acf7b04e86b14a344
KEYMAP="${1:-vial}"
shift || true

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
: "${QMK_HOME:?QMK_HOME 에 vial-qmk 경로를 지정하세요}"

head="$(git -C "$QMK_HOME" rev-parse HEAD)"
if [ "$head" != "$VIAL_QMK_COMMIT" ]; then
    echo "경고: vial-qmk 커밋이 $head 입니다(검증한 커밋 $VIAL_QMK_COMMIT)." >&2
fi

if [ "$KEYMAP" = "vial" ]; then
    "$ROOT/scripts/fetch_libmodule.sh"
fi

rm -rf "$QMK_HOME/keyboards/gats/wecat4"
mkdir -p "$QMK_HOME/keyboards/gats"
cp -R "$ROOT/keyboards/gats/wecat4" "$QMK_HOME/keyboards/gats/wecat4"

(cd "$QMK_HOME" && qmk compile -kb gats/wecat4 -km "$KEYMAP" "$@")

mkdir -p "$ROOT/build"
cp "$QMK_HOME/gats_wecat4_${KEYMAP}.bin" "$ROOT/build/"
cd "$ROOT/build"
if command -v sha256sum >/dev/null; then sha256sum ./*.bin > SHA256SUMS; else shasum -a 256 ./*.bin > SHA256SUMS; fi
cat SHA256SUMS
