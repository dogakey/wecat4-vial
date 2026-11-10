# GUTS WECAT4 Vial Firmware

[한국어](README.md) | **English**

Unofficial QMK/Vial firmware for GUTS WECAT4 (WB32FQ95, 44-key, Wired/Bluetooth/2.4G).
With this you can customize key layouts/layers/macros/tap dance/combo/lightings directly
in [Vial](https://get.vial.today/). Changes saved to hardware.

> **Beta** - Wired/Bluetooth/Vial verified on real hardware, features marked `⚠️` (e.g.
> 2.4G/battery/charge indicator) have not been tested. Verify list: [docs/VERIFY.md](docs/VERIFY.md)

> [!WARNING]
>
> - This is not official and/or endorsed by the manufacturer/seller. GPL-2.0 as-is, liability falls under the user.
> - There is no public stock firmware file. **There is no going back** - you need to request the original firmware from the seller.
> - Firmware flashing deletes existing keybinds. If you were using VIA, back it up first.

## Features

| Feature | Status |
|---|---|
| Vial live keybind configuration (8-layer, macros, tap dance, combo, key overrides, Caps Word, lighting) | ✅ |
| Wired (USB) | ✅ |
| Bluetooth 3-channel (1-3), 3 seconds = new pairing | ✅ |
| 2.4G dongle | ⚠️ (untested) |
| Battery charge status/level indicator, power saving mode, 3 second press for factory reset | ⚠️ (untested) |
| Backlight off after 5 minutes of inactivity, sleep after 30 minutes of inactivity (wireless) | ⚠️ (untested) |
| Lighting off when computer sleeps or cable is unplugged | ⚠️ (untested) |

Wireless depends on the manufacturer's published wireless libraries. See [NOTICE.md](NOTICE.md).

## Two versions

| | `vial` (standard) | `dogakey` |
|---|---|---|
| For | Standard action | Additional feature(s) |
| Vial/Wireless/RAINBOW | same | same |
| Key layouts | same as original firmware | E/K(한영) key is configured differently - see below |
| E/K key | - | ✅ (⚠️ untested) |
| Status Emphasis | - | ✅ |
| Blink on lang change | - | ✅ |
| Anti-chattering (debounce) | standard defer 5ms | eager 30ms - input on press, filters jitter within 30ms |

### dogakey features

**E/K (한영) key** - tap to switch language, hold to transiently switch language. (hold down to
temporarily type Korean when in English mode)

| Mode | E/K key location | Keycode | Displaced Key |
|---|---|---|---|
| macOS | where capslock is | capslock 100ms (requires capslock to previous IME setting) | actual capslock is now in the function layer, same location |
| Windows | right alt | E/K (Lang1) | right alt (no replacement) |

- While E/K is held, the letters (A-Z) are highlighted in green. Tap to switch.
- If E/K is released while another key pressed after it is still held (a fast roll), it is treated as a tap - that key is typed in the new language and the switch stays.
- Vial custom key `HAN/ENG` (E/K) can be moved to another location. To restore its functionality to the original layout, replace it with its original key (capslock or right alt).

**Status Emphasis** - While a momentary layer or Shift/Ctrl/Win(Cmd) is held, keys which are
available in that layer are lit and the other keys are turned off. This visualizes key availability.
If the key is moved in Vial, a reflash is not required - it updates automatically.

| Held key | Lit up keys | Color |
|---|---|---|
| Fn(MO2) | keys assigned in that layer | violet |
| MO3 | keys assigned in that layer | purple |
| MO4 and above | keys assigned in that layer | pink |
| Windows mode - Ctrl | A C V X Z Y S F N T W R P O H Tab ← → | Red |
| Windows mode - Win | D E L R I S V Tab ← → ↑ ↓ | White |
| Windows mode - Alt | Tab F4 ← → | Teal (⚠️ untested) |
| macOS mode - command | A C V X Z S F N T W R P Q H M , \` Space Tab ← → | Red |
| macOS mode - control | Terminal/text editing(B D S C Z L R A E U K W P N ← → ↑ Tab) | White |
| macOS mode - option | Backspace ← → | Teal (⚠️ untested) |
| Shift | keys which have a shifted layer whose value is different from its unshifted layer | orange-ish yellow (주황빛 노랑) |

The keyboard does not know the host's operating system. The shortcuts are read off of the mode which
is selected. Primary shortcuts are red (Windows Ctrl, macOS command), and Secondary auxiliary shortcuts
are white (Windows Win, macOS control). Several keys held together are processed in the order of
layer -> red -> white -> teal -> Shift, where a left layer overrides the right layer.
The emphasis is lit up using the current brightness setting. (Fn shortcut to adjust lighting levels is reflected live)

**Blink on lang change** - Press a key which changes the input language, and the entire keyboard
fades smoothly into green, a light sky blue band goes through the keyboard left to right, displacing
the green, and it returns to its original lighting (~1.5 seconds). It is distinctly visible without
the need to look down on the keyboard - your eyes can remain on-screen.

- Keys which trigger the effect: capslock, right alt, Lang1(E/K), Lang2(Hanja), JIS かな・変換・無変換.
  macOS according to capslock, Windows according to Microsoft IME, JIS keys assume their original purpose.
- The keyboard is IME-stateless - it only knows that *a language key was pressed*. This does not affect the keycodes.
- Right option in macOS also triggers this. Win+Space, control+space, or Shift independently in Sino-IMEs are not supported, as they are already occupied by other roles.
- `is_lang_switch()` in `keymaps/dogakey/highlight.c` sets which keys should trigger this, and
  the color and/or duration settings can be set by the constants in the file such as `FLASH_*` or `SWEEP_*`.
- The blink follows the keyboard's brightness. If lighting is off, this is disabled as well.
- Battery/charge/Bluetooth lighting overrides this.

**Anti-chattering (eager debounce 30ms)** - Mechanical switches suffer from chattering (double input)
at the end of its life or when the contacts have been contaminated. The standard filters this
only within 5ms, but this version takes into account 30ms of margin. Chattering keys can be used
as normal for the most part.

- Press is processed instantly and only the identical key is ignored for 30ms, thus there is no input delay.
- Each key runs its own clock - this does not affect other switches.
- Cost: A same key cannot be pressed within 30ms of its release (at most 16 keypresses per second). Normal human repetition is 7-12 per second, and is unaffected.
- Certain game actions which depend on key release (negative edge) may be delayed up to 30ms.
  If this is problematic for you, rebuild with `DEBOUNCE` in `keymaps/dogakey/config.h` set to
  25-28 (20 keypresses per second @ 25ms). The 30ms figure was derived from various switches -
  lowering it may cause chattering.

## Stock key layout

Layer 0 (Windows) and Layer 1 (macOS) are stock. **Fn** is the centermost key in the bottommost
row, between the two spaces. Fn combinations are in Layer 2.

| Fn + Key | Action | Fn + Key | Action |
|---|---|---|---|
| Tab | Esc | Q-P | 1-0 |
| Backspace | Battery remaining (Q-P, 10%p per step, yellow in wired) | Enter | Power saving mode (wireless) / Turn off lighting (wired) |
| A S D | Bluetooth 1 2 3 | F | 2.4G |
| K L | `-` `=` | `/` | `` ` `` |
| `.` / App / right alt / right control | ↑ / ↓ / ← / → | Right Space | Charge status (red - charging, green - charged) |
| Z | Change lighting effect | X/C | Lighting speed -/+ |
| B/M | Color -/+ | N/H | Brightness -/+ |
| J/G | Saturation -/+ | Win | Win lock |
| left control for three seconds | factory reset | right shift | Windows ↔ macOS mode toggle |

**Windows / macOS mode** - Fn + right shift toggles the base layer between Layer 0 (Windows) and Layer 1 (macOS). For one second after the switch, all other lights are switched off, and either
**W** (Windows) or **M** (macOS) is illuminated in white. The mode setting persists after power off,
and the default after a reflash or factory reset is Windows mode.

While Fn is held, the current mode's key (W/M) flashes 2 times per second to indicate the current
mode (⚠️ untested). The two layers are independent, Layer 1 modification only affects macOS mode.
For example, the Alt key and Win key can be swapped in only the macOS mode.

Layers 3-7 are empty. Customize in Vial as you see fit.

### Vial-specific keys

In the **User** tab in Vial. Use in whatever position.

| Name | Action |
|---|---|
| BT1/BT2/BT3 | Bluetooth channel switch. Press for three seconds to enter pairing mode. |
| 2.4G | Start new pairing (retries for 60 seconds, stops on successful connection) |
| BAT | Show remaining battery charge |
| CHRG | Show charging status |
| SLEEP | Power saving mode |
| RESET | Press for three seconds to factory reset |
| RAINBOW | Switch lighting to rainbow effect (Cycle Left/Right) |
| WIN/MAC | Toggle Windows↔macOS mode (also in wired-only build) |
| HAN/ENG | E/K key (dogakey-only, see above) |

BT4/BT5/USB keys are unused (Bluetooth has only three channels, and USB is selected by a switch on the side).

## Wireless instructions

- Select the connection type using the switch on the side (BT / wired / 2.4G). Plugging in the cable does not automatically switch the mode.
- **First Bluetooth connection** - Set switch to BT, press Fn + A(or S/D) for three seconds. When the key blinks rapidly, look for `WECAT4 BT1` or `-BT2/BT3` on the host.
- **Change host** - Tap Fn + A/S/D. Avoid long-press - that will enter pairing mode.
- **Connection indicator** - While attempting to connect, the applicable key of A/S/D blinks. If connection fails after 10 seconds, the entire keyboard blinks in dark red. If left alone for 30 seconds without connection, it sleeps. Press any key to wake. On connection success, the corresponding key turns on for one second, then the keyboard returns to its normal lighting.
- **See current channel** - When in wireless, hold Fn(or momentary Layer 2 or higher), one of A/S/D/F will blink in the channel's color (A - red, S - green, D - blue, F - teal). This follows the key location set using Vial.
- **First 2.4G connection** - Move the switch to 2.4G and press Fn+F once. It will start pairing immediately, and will start blinking. It retries every 1.5 seconds for 60 seconds, and stops when it succeeds. **Plug in the dongle while the keyboard is blinking.** (replug if you have already had it plugged in)
- Press again to start a new pairing. Once paired, move the switch to 2.4G to automatically connect. Dongle appears to the host in its stock firmware name (`Hangsheng` / `TH40 PRO-2.4G  Dongle`) This has not been tested with this specific release (⚠️).
- **Low battery alert** - When in wireless and the battery charge is at most 20%, the Fn key blinks in red.
- **The keyboard only charges in wired mode.** When the switch is in BT/2.4G, the keyboard does not charge regardless of whether it is plugged in.

## Change keybinds with Vial

1. Set the switch to wired mode, and connect to a host with USB. Vial only runs over USB.
2. Use the [Vial app](https://get.vial.today/) or [vial.rocks](https://vial.rocks/) in Chrome/Edge. Keyboard definition is in hardware; you do not need to prepare a definitions file.
3. Press the key you want to change, and press the key you want it to be from below.
4. If a security notice pops up, hold **Tab+Backspace** to unlock.

## Flash the firmware

From [Releases](../../releases), download `gats_wecat4_vial.bin` (standard) or `gats_wecat4_dogakey.bin` (dogakey). If you do not use it wireless, `gats_wecat4_vial_wired.bin` also works. Check sha256 hash to validate file integrity.

**Enter bootloader**: Unplug the cable. Set the switch to wired. **While tab is pressed, plug it in.** The keyboard will appear as `342D:DFA0`(WB Device in DFU Mode). This is the same access method as the stock firmware. This will reset your existing keybinds.

**Flashing tool**: `wb32-dfu-updater_cli` from [wb32-dfu-updater](https://github.com/WestberryTech/wb32-dfu-updater). Use [QMK MSYS](https://msys.qmk.fm/) for Windows, and QMK CLI for macOS/Linux.

```bash
wb32-dfu-updater_cli -s 0x08000000 -D gats_wecat4_vial.bin
```

Once it is finished, replug or restart with `wb32-dfu-updater_cli -R`.
If Windows fails to find the device, use [Zadig](https://zadig.akeo.ie/) to install the WinUSB driver to `342D:DFA0`.

## Build from source

[vial-qmk](https://github.com/vial-kb/vial-qmk) and QMK build environment required. Verified vial-qmk commit is `dd43959ae5c08d8a28d38a1acf7b04e86b14a344`.

```bash
git clone https://github.com/vial-kb/vial-qmk.git
git -C vial-qmk checkout dd43959ae5c08d8a28d38a1acf7b04e86b14a344
git -C vial-qmk submodule update --init --recursive
QMK_HOME=$PWD/vial-qmk ./scripts/build.sh vial
```

- `scripts/build.sh vial`: wireless included. From the manufacturer's wireless libraries, `scripts/fetch_libmodule.sh` fetches, verifies, and builds.
- `scripts/build.sh dogakey`: dogakey-version wireless.
- `scripts/build.sh vial_wired`: wired-only. No manufacturer binaries, only GPL sources used in build.
- Artifacts output to `build/`.
- Wireless diagnostics build: `scripts/build.sh vial -e WECAT4_DIAG=yes`. Keeps USB alive even during wireless, and can be interfaced via raw HID `0xD1`.


## Hardware Information

| Key | Value |
|---|---|
| MCU | WB32FQ95RCT6(Cortex-M3), external 12MHz crystal |
| Matrix | 4×12, ROW2COL. Columns `C0 C1 C2 C3 A6 B10 B11 B12 B13 B14 A10 C6`, Rows `A2 A3 A4 C13` |
| LED | WS2812 x47(B15), Power control A5/A9 |
| Wireless chip connections | UART3 C10(TX)/C11(RX) |
| Mode switch | C14/C15(pull-up) |
| USB | VID `0x342D`, PID `0xE47E` (identical to stock firmware) |


## License

GPL-2.0-or-later ([LICENSE](LICENSE)). Original copyright holders and third-party components are listed in [NOTICE.md](NOTICE.md) (Korean).

---

*Human-translated by contributor - [@themrsung](https://github.com/themrsung)*
