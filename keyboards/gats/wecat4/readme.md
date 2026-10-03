# gats/wecat4

GUTS WECAT4 — 44-key 40% keyboard, WB32FQ95, wired / Bluetooth / 2.4 GHz.

* Keyboard Maintainer: dogakey
* Original board definition: [sdk66](https://github.com/sdk66) (`hangshengkeji/qmk_firmware`, `keyboards/leo/wecat4`)

Keymaps:

* `vial` — wired + wireless (requires the vendor `wireless/libmodule.a`, fetched by `scripts/fetch_libmodule.sh`)
* `vial_wired` — wired only, GPL sources only

Build (from a vial-qmk checkout after copying this folder to `keyboards/gats/wecat4`):

    qmk compile -kb gats/wecat4 -km vial

Bootloader: hold Tab (matrix 0,0) while plugging in the USB cable (also clears the stored keymap).
