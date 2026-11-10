# GUTS WECAT4 Vial Firmware

[한국어](README.md) | **English**

**Unofficial** QMK/Vial firmware for the GUTS WECAT4 40% keyboard (WB32FQ95, 44 keys, wired / Bluetooth / 2.4 GHz).
With this firmware you can change the key layout, layers, macros, tap dance, combos and lighting live in [Vial](https://get.vial.today/), and the changes are saved on the keyboard.

> **Beta** — Wired, Bluetooth and Vial have been confirmed in daily use. Items marked ⚠️, such as 2.4 GHz, battery and charging indicators, have not yet been tested on real hardware. Test checklist: [docs/VERIFY.md](docs/VERIFY.md) (Korean)

> **Warning**
> - This is not official firmware from the manufacturer or seller. Use it at your own risk (GPL-2.0, no warranty).
> - The original firmware file is not publicly available. To go back to the original firmware after flashing this one, you must ask the seller for the original file.
> - Flashing erases the key layout currently stored on the keyboard. If you were using VIA, save your keymap in VIA first.

## Features

| Feature | Status |
|---|---|
| Live key editing in Vial (8 layers, macros, tap dance, combos, key overrides, Caps Word, lighting) | ✅ |
| Wired (USB) | ✅ |
| Bluetooth, 3 channels (BT1–3), hold 3 s = new pairing | ✅ |
| 2.4 GHz dongle | ⚠️ Not yet tested on hardware |
| Battery level indicator, charging indicator, deep sleep, 3 s factory reset | ⚠️ Not yet tested on hardware |
| Backlight off after 5 min idle, sleep after 30 min idle (wireless) | ⚠️ Configured only, not yet tested on hardware |
| Lighting turns off when the computer sleeps or the cable is unplugged (wired) | ⚠️ Not yet tested with this release build |

Wireless features work by using the wireless module library published by the keyboard manufacturer (Hangsheng) as-is. See [NOTICE.md](NOTICE.md) (Korean) for details.

## Two versions

| | `vial` (standard) | `dogakey` |
|---|---|---|
| For | Standard behavior close to the original firmware | Anyone who wants the extra features below |
| Vial / wireless / RAINBOW | Same | Same |
| Key layout | Same as the original firmware | Differs by one key only, the Han/Eng key (below) |
| Han/Eng key | — | ✅ (⚠️ not yet tested on hardware) |
| State highlighting | — | ✅ |
| Language-switch flash | — | ✅ |
| Chatter prevention (debounce) | Standard defer 5 ms | Eager 30 ms: registers instantly on press, filters bounce within 30 ms |

### dogakey version features

**Han/Eng key** — Tap it to switch the input language. Hold it while typing other keys to type in the other language only while it is held (e.g. while typing Korean, hold Han/Eng, type one English word, release, and you are back in Korean).

| Mode | Han/Eng key position | Key sent to the computer | Displaced key moved to |
|---|---|---|---|
| macOS | CapsLock position | CapsLock for 100 ms (requires "Use Caps Lock to switch input sources") | Caps lock (uppercase) = Fn + that key |
| Windows | Right Alt position | Han/Eng key (Lang1) | No right Alt (left Alt unchanged) |

- While the Han/Eng key is held, the letter keys (A–Z) light up green. A tap triggers the language-switch flash described below.
- A roll, where you press the next key and then release Han/Eng first, counts as a tap. Keys typed during the switch are sent in order afterward.
- You can move it elsewhere with the Vial custom key `HAN/ENG`. To restore the original key, put CapsLock or Right Alt back in that position.

**State highlighting** — While a layer key or Shift / Ctrl / Win (Cmd) is held, only the keys that do something in that state stay lit and the rest go dark, so you can see right away which keys to press after switching layers. If you move keys in Vial, the highlighting follows without reflashing.

| Held key | Keys lit | Color |
|---|---|---|
| Fn (layer 2) | Keys assigned on that layer | Indigo |
| Layer 3 | Keys assigned on that layer | Purple |
| Layer 4 and up | Keys assigned on that layer | Pink |
| Windows mode Ctrl | A C V X Z Y S F N T W R P O H Tab ← → | Red |
| Windows mode Win | D E L R I S V Tab ← → ↑ ↓ | White |
| macOS mode Cmd | A C V X Z S F N T W R P Q H M , ` Space Tab ← → | Red |
| macOS mode Ctrl | Terminal / line editing (B D S C Z L R A E U K W P N ← → ↑ Tab) | White |
| Windows mode Alt | Tab F4 ← → | Cyan (⚠️ not yet tested on hardware) |
| macOS mode Option | Backspace ← → | Cyan (⚠️ not yet tested on hardware) |
| Shift | Keys whose output changes with Shift (letters, numbers, symbols) | Amber |

The keyboard can't tell whether the connected computer runs Windows or macOS, so it uses the shortcut list for the mode you have selected (Fn + Right Shift). The main shortcut modifier of each OS (Windows Ctrl, macOS Cmd) is red, and the secondary modifier is white.
When several are held together, the priority is layer > red > white > Alt (cyan) > Shift. ← → F4 are on the Fn layer, so on the base layer only Backspace (macOS) or Tab (Windows) lights up.
Highlight colors follow the current lighting brightness (if you change brightness while holding Fn, the highlighted keys update immediately).

**Language-switch flash** — When you press a language-switch key, the whole keyboard fades up to green, then a pale sky-blue band sweeps from left to right, pushing the green away, and the original lighting returns (about 1.5 s). You can see the switch happened without looking away from the screen.
- Keys that trigger it: CapsLock, Right Alt, Lang1 (Han/Eng) / Lang2 (Hanja), and the Japanese かな / 変換 / 無変換 keys.
  These are based on macOS (CapsLock to switch input sources), the Windows Korean IME (Right Alt), and the dedicated Korean/Japanese keys.
- The keyboard doesn't know which language is active, so it only signals that a switch key was pressed. The key sent to the computer is unchanged.
- Pressing Right Option on macOS also triggers it. Combinations like Win+Space or Ctrl+Space, and Shift alone in Chinese IMEs, overlap with other uses and do not trigger it.
- Change which keys trigger it in `is_lang_switch()` in `keymaps/dogakey/highlight.c`, and the timing and colors in the `FLASH_*` / `SWEEP_*` values at the top of the same file.
- Flash brightness also follows the current lighting brightness. If lighting is off, there is no flash. Battery, charging and Bluetooth connection indicators take priority over the flash.

**Chatter prevention (eager debounce, 30 ms)** — Mechanical switches that are old or have dirty contacts can chatter: one press registers twice. The standard version's default (5 ms) only filters bounce within 5 ms, while this version filters bounce up to 30 ms, so most keys that have started chattering become usable again.
- The key registers the moment it is pressed, and only further changes on that same key are ignored for the next 30 ms, so input latency is close to zero despite the long window (good for gaming too).
- Each key is handled separately, so quickly pressing different keys in a row is unaffected.
- Trade-off: pressing the same key again within 30 ms of releasing it is held back (single-key repeat limit about 16 per second). Fast human repeated tapping is usually 7–12 per second, so this rarely matters.
- Game moves triggered on button release (negative edge) can be up to 30 ms late for very short taps. If you mostly do rapid tapping, lower `DEBOUNCE` in `keymaps/dogakey/config.h` to 25–28 and build it yourself (25 ms gives a repeat limit of about 20 per second). 30 ms was chosen to cover the bounce intervals of different switches with margin, so lowering it may let some duplicate presses through.

## Default key layout

Layer 0 (Windows) and layer 1 (macOS) both start out matching the keycap legends. **Fn** is the key in the middle of the bottom row (between the two Space keys), and the Fn combinations are on layer 2.

| Fn + key | Action | Fn + key | Action |
|---|---|---|---|
| Tab | Esc | Q – P | 1 – 0 |
| Backspace | Battery level (lights on Q–P, one key per 10%; blinks yellow in wired mode) | Enter | Deep sleep (wireless) / lighting off (wired) |
| A / S / D | Bluetooth 1 / 2 / 3 | F | 2.4 GHz |
| K / L | `-` / `=` | `/` | `` ` `` |
| `.` / App / Right Alt / Right Ctrl | ↑ / ↓ / ← / → | Right Space | Charging status (red = charging, green = full) |
| Z | Change lighting effect | X / C | Lighting speed − / + |
| B / M | Hue − / + | N / H | Brightness − / + |
| J / G | Saturation − / + | Win | Win key lock |
| Left Ctrl **3 s** | Factory reset | Right Shift | Switch Windows ↔ macOS mode |

**Windows / macOS mode** — Each press of Fn + Right Shift switches the base layer between 0 (Windows) and 1 (macOS). For 1 second after switching, all other lights go off and only the W key (Windows) or M key (macOS) lights up white. The selected mode is kept across power cycles, and it is Windows mode after first flashing or a factory reset. While Fn is held, the current mode key (Windows W / macOS M) blinks white twice per second to show the current mode (⚠️ not yet tested on hardware). Both layers start out identical, so if you want to change keys only for the Mac, edit just layer 1 in Vial (e.g. swap Alt and Win).

Layers 3–7 are empty. Fill them however you like in Vial.

### Vial custom keys

These are in Vial's **User** tab. Put them wherever you like.

| Name | Action |
|---|---|
| BT1 / BT2 / BT3 | Switch Bluetooth channel. Hold 3 s to start a new pairing on that channel |
| 2.4G | In the 2.4 GHz switch position, starts a new pairing immediately (retries for 60 s, stops once connected) |
| BAT | Show battery level |
| CHRG | Show charging status |
| SLEEP | Deep sleep |
| RESET | Hold 3 s for factory reset |
| RAINBOW | Switch lighting straight to the rainbow (Cycle Left/Right) effect |
| WIN/MAC | Switch Windows ↔ macOS mode (also in the wired-only build) |
| HAN/ENG | Han/Eng key (dogakey version only, see above) |

The BT4, BT5 and USB keys are not used (Bluetooth has 3 channels, and wired mode is selected with the side switch).

## Wireless usage

- Select the connection with the **3-position side switch** (BT / wired / 2.4G). Plugging in a cable does not switch modes automatically.
- **First Bluetooth connection**: Set the switch to BT and hold Fn + A (or S, D) for **3 seconds**. When the key blinks rapidly, select `WECAT4 BT1` (BT2, BT3) from the Bluetooth list on your computer or phone.
- **Switching devices**: Press Fn + A / S / D **briefly**. Every 3-second hold creates a new pairing, so use a short press for devices you have already paired. If you need to pair again, remove the old `WECAT4` entry on the device first.
- **Connection indicator**: While connecting, the matching A / S / D key blinks. If it doesn't connect within 10 seconds, the whole keyboard blinks dark red. If it stays disconnected with no key presses for 30 seconds, it goes to sleep right away (press any key to wake it and retry; it doesn't sleep during a new pairing). Once connected, the key stays lit for 1 second and then normal lighting returns.
- **Seeing the connected channel**: While connected wirelessly, holding Fn (layer 2 or higher) makes the key for the currently connected channel on that layer (A / S / D or F) blink in the channel color (A red, S green, D blue, F cyan). It follows the channel key if you move it to another position or layer in Vial.
- **First 2.4 GHz connection**: Set the switch to 2.4G and press Fn + F **once**. Pairing starts immediately and the light blinks; it resends every 1.5 seconds for 60 seconds and stops once connected. **Plug in the dongle while it is blinking** (if it's already plugged in, unplug it and plug it back in). Pressing it while already connected starts a new pairing. Once paired, just set the switch to 2.4G and turn the keyboard on and it reconnects automatically. The dongle shows up on the computer with the same names as with the original firmware (`Hangsheng` / `TH40 PRO-2.4G  Dongle`). ⚠️ Not yet tested on hardware with this release build.
- **Low battery**: In wireless mode, the Fn key blinks red when the battery is at 20% or below.
- **Battery level is only shown in wireless mode**: The battery isn't read in wired mode, so Fn + Backspace blinks Q–P yellow instead of showing the level.
- **Charging only works with the switch set to wired**: If the switch is on BT or 2.4G, plugging in a USB cable does not charge the battery. Move the switch to wired to charge.

## Changing keys with Vial

1. Set the switch to **wired** and connect a USB cable. Vial only connects over a wired connection.
2. Run the [Vial app](https://get.vial.today/), or open [vial.rocks](https://vial.rocks/) in Chrome or Edge. The keyboard definition is built into the firmware, so there is no file to load.
3. Click a key and choose a new key from the list below; it is saved immediately.
4. If you are asked to unlock to change security-sensitive settings such as macros or bootloader entry, hold **Tab and Backspace together** to unlock.

## Flashing the firmware

Download `gats_wecat4_vial.bin` (standard) or `gats_wecat4_dogakey.bin` (dogakey version) from [Releases](../../releases). If you don't use wireless, `gats_wecat4_vial_wired.bin` works too.
Check that the downloaded file matches the sha256 listed on the Releases page.

**Entering the bootloader**: With the cable unplugged, set the switch to wired, then **hold the Tab key while** plugging in the cable. The keyboard appears as `342D:DFA0` (WB Device in DFU Mode). The same method works on the original firmware.
With this firmware, entering the bootloader this way resets the key layout stored on the keyboard.

**Flashing tool**: Use `wb32-dfu-updater_cli` from [wb32-dfu-updater](https://github.com/WestberryTech/wb32-dfu-updater). It is installed along with [QMK MSYS](https://msys.qmk.fm/) (Windows) or the QMK CLI (macOS / Linux).

```bash
wb32-dfu-updater_cli -s 0x08000000 -D gats_wecat4_vial.bin
```

When it finishes, replug the cable or restart with `wb32-dfu-updater_cli -R`.
If the device isn't found on Windows, use [Zadig](https://zadig.akeo.ie/) to install the WinUSB driver for `342D:DFA0`.

## Building it yourself

You need [vial-qmk](https://github.com/vial-kb/vial-qmk) and a QMK build environment. The tested vial-qmk commit is `dd43959ae5c08d8a28d38a1acf7b04e86b14a344`.

```bash
git clone https://github.com/vial-kb/vial-qmk.git
git -C vial-qmk checkout dd43959ae5c08d8a28d38a1acf7b04e86b14a344
git -C vial-qmk submodule update --init --recursive
QMK_HOME=$PWD/vial-qmk ./scripts/build.sh vial
```

- `scripts/build.sh vial`: With wireless. `scripts/fetch_libmodule.sh` downloads the manufacturer's wireless library from the manufacturer's repository and checks its sha256 before building.
- `scripts/build.sh dogakey`: dogakey version with wireless (state highlighting, language-switch flash, eager debounce).
- `scripts/build.sh vial_wired`: Wired only. Built from GPL sources only, without the manufacturer's binary.
- Output goes to `build/`.
- Wireless diagnostics build: `scripts/build.sh vial -e WECAT4_DIAG=yes`. Keeps USB active in wireless mode, and the status can be read with the raw HID command `0xD1`.

## Hardware

| Item | Value |
|---|---|
| MCU | WB32FQ95RCT6 (Cortex-M3), external 12 MHz crystal |
| Matrix | 4×12, ROW2COL. Columns `C0 C1 C2 C3 A6 B10 B11 B12 B13 B14 A10 C6`, rows `A2 A3 A4 C13` |
| LEDs | 47 × WS2812 (B15), power control A5 / A9 |
| Wireless chip | UART3 C10 (TX) / C11 (RX) |
| Mode switch | C14 / C15 (pull-up) |
| USB | VID `0x342D`, PID `0xE47E` (same as the original firmware) |

## License

GPL-2.0-or-later ([LICENSE](LICENSE)). Original authors and third-party components are listed in [NOTICE.md](NOTICE.md) (Korean).
