# hadistory — Raspberry Pi Pico version

Port of hadistory to the RP2350 (Raspberry Pi Pico 2 W, Pimoroni Pico Plus 2 / Pico LiPo 2) driving the Inky Impression 5.7" (600 x 448, 7 colours, UC8159). Written in C++ on the Pico SDK with the Pimoroni libraries: `pico_graphics` and the `uc8159` display driver, `sdcard` and `fatfs` for the microSD variant.

Scope compared to the Pi version:
- **Story mode**: pages made of a JPEG illustration and a text, either on a microSD card (`STORAGE=SDCARD`) or embedded in the firmware (`STORAGE=FLASH`). The firmware composes the page with its bitmap font. Pages are prepared by `tools/prepare_pages.py`, see `tools/README.md`.
- **AI mode** (optional, `WITH_GPT=ON`): OpenAI text and image generation over WiFi, no embedded models. The WiFi is joined on demand and the switch to AI mode is refused without network.
- 4 buttons (execute, reset, change mode, go back) and a status LED, same behaviour as the Pi version, plus a robot badge on the AI pages.
- **Battery gauge** (`BATTERY_GAUGE=ON`, the default): battery bars in the top-right corner of each page. Display only, nothing in the firmware depends on the level.

## Status

| Part | State |
|---|---|
| Buttons, status LED, display | Working on a Pico Plus 2 with the Inky Impression 5.7" display HAT wired on a breadboard |
| Story mode from flash | Working with JPEG + text pages composed on the board (bitmap font, page counter) |
| Story mode from microSD | Written, not yet tested on hardware |
| Onboard LED (`STATUS_LED=ONBOARD`) | Working on the Pico 2 W (wireless-chip LED, 1 Hz blink when busy); GP25 of the Pico Plus 2 not tried yet |
| AI mode | Working end to end on a Pico 2 W: prompt, chat request, image request, JPEG decoding and display, about 35 s plus the 30 s refresh |
| Deep sleep between pages (`DEEP_SLEEP=ON`) | Working on the Pico 2 W: RP2350 power manager, wake-up and action on the button that woke the board |
| Battery operation | To be tested, see Power |
| Battery gauge (`BATTERY_GAUGE=ON`) | Calibrated on the prototype (ADC scale and diode drop measured, 2026-10-21); bars to check against the cell voltage |
| Mode LED and robot badge | Written, to be tested: green sphere on in AI mode, bursts at a mode change, robot top-left of the AI pages |
| Story dial (`STORY_DIAL=ON`) | Written, to be tested: KY-040 on GP6 / GP7 / GP9, story number in the page counter |

## Use

At boot the firmware prints on the USB serial console (115200 baud), announces the current mode with the LED and waits with the LED on:

```
Welcome to hadistory !                 (or "Welcome back to hadistory !" after a deep sleep, with the button that woke it)
Flash storage: 2 embedded stories      (or "SD card: mounted")
Stories available: 2
Starting on local story mode.
Waiting for next button press...
```

| Button | Story mode | AI mode (`WITH_GPT=ON`) |
|---|---|---|
| A, execute | Next page; when none is running or the last page was shown, starts a story: at random, or the one under the dial with `STORY_DIAL=ON` | Generates and shows a new page |
| D, go back | Previous page | — |
| B, reset | Forgets the current story, the next A starts a new one. With `STORY_DIAL=ON` the push button of the dial does the same, and both re-arm the dial at 1 | Same |
| C, change mode | Starts the WiFi connection if needed (status LED breathing, mode LED blinking), then switches to AI mode: 3 long bursts on the mode LED, which then stays on; if the network cannot be joined within 30 s the switch is refused with the error blink and the mode LED goes off | Switches to story mode: 5 short bursts on the mode LED, which then stays off |

Two sphere LEDs, one question each. Status LED (white, left of the chin): solid = waiting for a press, breathing (or 1 Hz blink on the Pico 2 W onboard LED) = working, including the 30 s refresh of the panel, 19 rapid blinks = error, off = asleep. Mode LED (green, right of the chin): on = AI mode, off = story mode, blinking = joining the WiFi after a press on C; it goes off again if the network cannot be joined (with the error blink on the status LED). At a mode change, and at a cold boot, it first plays the bursts of the Pi version, 3 long (0.8 s) for AI or 5 short (0.25 s) for story, then settles; the status LED never bursts for the mode. On the page itself, the story mode shows its page counter top-left and the AI mode a blue robot at the same place, so the mode is readable even with the board asleep. In story mode the core is powered off 20 seconds after the last press (`DEEP_SLEEP=ON`, the default, `IDLE_BEFORE_SLEEP_MS`); any button powers it up again, which is a reboot of about a second, and the firmware then acts on that button without the start-up delay and announcement. The AI mode stays awake to keep its WiFi link. The current story, page and mode are saved after each action, in `/currently.txt` on the card or in the last flash sector, and restored at boot. Without `WITH_GPT`, button C only prints "AI mode not available in this build".

Story dial (`STORY_DIAL=ON`): a KY-040 rotary encoder on the left edge of the enclosure, a thumbwheel with the numbers 1 to 12 on its rim (`DIAL_POSITIONS`) and an arrow on the case, see `case/README.md`. Printed end stops limit the wheel to 11 detents; the firmware counts the detents by interrupt while awake and saturates at 1 and 12, so the stops are the reference of an otherwise incremental encoder. Nothing counts during the deep sleep, by choice (battery life), so the ritual re-arms it: turn the wheel down to the stop, press the wheel (or B), which wakes the board, forgets the story and sets the dial to 1, turn up to the number under the arrow, press A: that story starts at page 1, and its number is shown before the page counter top-left (`3: 1/9`). While a story is being read the wheel changes nothing; at the end of a story, A starts the one under the arrow again (no more random pick: the AI mode is the random one). Each counted detent blinks the status LED off for 20 ms (`DIAL_BLIP_MS`): no blink means the wheel is past a stop in the firmware's count, i.e. desynchronised, and the ritual fixes it. Stories are ordered by the number at the end of their folder name (`story_2` before `story_10`); a wheel position beyond the catalogue gives the last story. The push button of the dial takes the wake-up slot of button B, which then only works awake (the RP2350 has four wake-up slots). In AI mode the dial is ignored for now.

Battery (`BATTERY_GAUGE=ON`): each page carries a battery symbol in its top-right corner, 4 bars when full, red with one bar or none when low, a lightning bolt when the USB is plugged. The gauge is informative only: the firmware never refuses an action, powers off or changes mode because of the level; when the cell is really empty the protection circuit of the battery module cuts the power. See "Battery gauge" under Power and battery.

## Layout

```
pico/
├── CMakeLists.txt          build configuration and options
├── main.cpp                button loop and mode switching (port of the Python main loop)
├── secrets.cmake.example   WiFi / OpenAI credentials template (copy to secrets.cmake, ignored by git)
├── img/                    wiring diagram (wiring.py generates wiring.svg, rendered to wiring.png)
└── src/
    ├── hadistory_config.h  pins per board, display geometry, behaviour constants
    ├── led.cpp/.h          status LED: solid = waiting, breathing = busy (timer driven), rapid blink = error,
    │                       external GPIO (PWM) or onboard LED, including the cyw43-driven one of the Pico 2 W;
    │                       mode LED: plain GPIO, on in AI mode, bursts at a mode change, blinking while the WiFi is joined
    ├── buttons.cpp/.h      4 active-low buttons, edge detection (the dial push button counts as RESET)
    ├── dial.cpp/.h         STORY_DIAL: KY-040 quadrature decoding by interrupt, position 1..12 saturated at the printed stops
    ├── display.cpp/.h      PicoGraphics 3-bit frame buffer + Pimoroni UC8159 driver; the only path to the panel
    ├── render.cpp/.h       page composition: rotated text layout, JPEG decoded and dithered into the square
    ├── battery.cpp/.h      BATTERY_GAUGE: cell voltage from VSYS, bars
    ├── storage.h           storage interface (stories catalogue, page = JPEG + text, persisted state)
    ├── storage_sdcard.cpp  STORAGE=SDCARD backend: FAT microSD, state in /currently.txt
    ├── storage_flash.cpp   STORAGE=FLASH backend: pages embedded in the image, state in the last flash sector
    ├── stories_embedded.h  catalogue type for the generated stories_table.c (FLASH)
    ├── story.cpp/.h        story mode logic
    ├── power.cpp/.h        deep sleep: core powered off until a button is pressed (built with DEEP_SLEEP=ON)
    ├── gpt.cpp/.h          AI mode pipeline (built only with WITH_GPT=ON)
    ├── https.cpp/.h        blocking HTTPS POST on lwIP altcp_tls + mbedtls, streamed and de-chunked body
    ├── json.cpp/.h         JSON escaping and string extraction, enough for the OpenAI API
    ├── base64.cpp/.h       streaming base64 decoder for the image response
    ├── prompt.cpp/.h       story prompt from prompts/prompts.txt (weights, bracket choices), embedded at build time
    ├── openai_roots.h      root certificates of api.openai.com (GTS Root R4, GlobalSign Root CA)
    ├── mbedtls_config.h    mbedtls configuration (TLS 1.2 client)
    └── lwipopts.h          lwIP configuration for the AI mode
```

## Display path

Everything that reaches the panel goes through `display.cpp`: a `PicoGraphics_Pen3Bit` frame buffer (100 KB, allocated once) and the Pimoroni `UC8159` driver, which converts it to the panel format on the fly. Pages, from the storage or from the AI mode, are composed by `render.cpp` into that frame buffer: the text is wrapped and drawn with the `bitmap8` font at scale 2 (accents supported), the JPEG illustration is decoded by Pimoroni's `jpegdec`, resampled into the square left above the text and dithered to the 7 colours with Floyd-Steinberg error diffusion, strip by strip as the decoder delivers the blocks, with the inky palette blend used by the Pi version (the ordered dithering of PicoGraphics was tried first and gave a woven look). The layout is the one of `pi/hadistory.py`: portrait page, illustration on top, text below, rotated 90 degrees onto the landscape panel; the font is drawn rotated by PicoGraphics so no intermediate buffer exists. One 160 KB buffer holds the current JPEG (`IMAGE_MAX_BYTES`). Changing to another Pimoroni panel means swapping the driver class and the geometry in `display.cpp` / `hadistory_config.h`.

Orientation: `DISPLAY_ROTATION` in `hadistory_config.h` (`ROTATE_0` or `ROTATE_180`) is passed to the driver, which flips the scan directions of the UC8159 in hardware. `ROTATE_180` is the default: it puts the display HAT buttons at the top with the pages as rendered by the tool. Nothing is rotated in software.

## Build

Requirements: Pico SDK 2.x with `PICO_SDK_PATH` set, [`pimoroni-pico`](https://github.com/pimoroni/pimoroni-pico) checked out next to it (the CMake finds it there), `cmake`, `arm-none-eabi-gcc`, `picotool`.

```bash
# Story mode, pages on microSD, Pico 2 W
cmake -S pico -B pico/build -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD
make -C pico/build -j8

# Story mode, pages embedded in flash, Pico Plus 2 (needs python3 + Pillow at build time)
cmake -S pico -B pico/build-flash -DPICO_BOARD=pimoroni_pico_plus2_rp2350 -DSTORAGE=FLASH
make -C pico/build-flash -j8

# Story + AI mode (needs a board with wireless)
cp pico/secrets.cmake.example pico/secrets.cmake   # then fill it in
cmake -S pico -B pico/build-gpt -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD -DWITH_GPT=ON
make -C pico/build-gpt -j8
```

Flashing: put the board in BOOTSEL mode (hold BOOTSEL while plugging the USB, or on the Pico Plus 2 hold BOOT, press RESET, release both) and copy `hadistory.uf2` to the `RP2350` volume, or use `picotool load -f -x pico/build/hadistory.uf2` from application mode. The `picotool` reboot over USB does not work while a serial monitor holds the port: close minicom first.

Console: USB serial, 115200 baud, e.g. `minicom -b 115200 -o -D /dev/tty.usbmodem*`. Messages printed before the monitor is attached are lost: the boot does not wait for it (the 2 s wait of the bring-up was removed), so attach the monitor before resetting the board, or build with `-DDEEP_SLEEP=OFF` and read the lines printed at each press.

Build-time options:

| CMake variable | Effect |
|---|---|
| `PICO_BOARD` | Selects the pin mapping in `hadistory_config.h`. Known: `pico2_w`, `pico2`, `pimoroni_pico_plus2_rp2350`, `pimoroni_pico_plus2_w_rp2350`. |
| `STORAGE` | `SDCARD` (default): pages on a FAT microSD, `HADISTORY_STORAGE_SDCARD=1`. `FLASH`: pages prepared at build time and embedded in the firmware, `HADISTORY_STORAGE_FLASH=1`. |
| `EMBED_STORIES` | `STORAGE=FLASH` only: semicolon-separated list of the stories to embed, e.g. `"story_1;story_2"`. Empty (default) embeds everything in `stories/`. |
| `EMBED_QUALITY` | `STORAGE=FLASH` only: JPEG quality of the embedded illustrations, 1 to 95, default 85. See the size table in `tools/README.md`. |
| `WITH_GPT` | `ON` builds the AI mode (`HADISTORY_WITH_GPT=1` in the code). Default `OFF`. |
| `DEEP_SLEEP` | `ON` (default): 20 seconds after the last press in story mode (`IDLE_BEFORE_SLEEP_MS`) the RP2350 power manager switches the core off (state P1.7), any button powers it up, which reboots the firmware (`HADISTORY_DEEP_SLEEP=1`). `OFF` keeps the board awake and the USB console alive, for debugging. |
| `BATTERY_GAUGE` | `ON` (default): battery bars on each page from the VSYS reading, display only (`HADISTORY_BATTERY=1`). `OFF` drops the module and the ADC. |
| `STORY_DIAL` | `ON`: KY-040 story dial on `DIAL_PIN_CLK` / `DIAL_PIN_DT` / `DIAL_PIN_SW` (GP6 / GP7 / GP9, "+" from GP14), RESET re-arms it at 1, A starts the story under the arrow (`HADISTORY_STORY_DIAL=1`). Default `OFF`: stories picked at random. |
| `STATUS_LED` | `EXTERNAL` (default): LED on `LED_PIN` (GP8). `ONBOARD`: the board's own LED, GP25 on the Pico Plus 2 with the same breathing effect; on the Pico 2 W the LED is on the wireless chip, no PWM, so busy is shown as a 1 Hz blink. |
| `WIFI_SSID`, `WIFI_PASSWORD`, `OPENAI_API_KEY` | Credentials for the AI mode, via `-D` or `secrets.cmake`. |
| `TLS_NO_VERIFY` | AI mode: `ON` skips the certificate check of `api.openai.com`. Debug only, it exposes the API key to anyone on the path. Default `OFF`: the server must chain to one of the roots in `src/openai_roots.h`. |

## Wiring (default mapping in `hadistory_config.h`, Pico 2 W and Pico Plus 2)

![Wiring of the Pico version](img/wiring.png)

*Everything on one picture: display HAT header, buttons, status and mode LEDs, story dial (option), TP4056, battery, switch and diode (`img/wiring.svg`, generated by `img/wiring.py`). The HAT header is drawn as seen from the back of the HAT, the side the wires plug into: pin 1 (3V3) top right, odd pins on the right column. The microSD breakout of `STORAGE=SDCARD` is not drawn.*

Physical pin numbers are those of the 40-pin headers: the Raspberry Pi header of the display HAT (the Inky Impression), and the Pico header (the Pico Plus 2 keeps the Pico layout for pins 1 to 40). Any Pico GND pin does for any ground; the ones below keep one wire per pin: 33 for the HAT, 38 for the TP4056, 13 for the status LED, 23 for the mode LED, 8 for the dial. The "Display HAT GPIO" column is the Raspberry Pi GPIO number the HAT expects on that pin, as in the Pimoroni pinout; it is only there to find your way on the HAT, nothing on the Pico uses it.

Power. The display HAT runs from 3V3 alone: its 5V pin only feeds its small expansion header (Pimoroni schematic), so it can stay unconnected. That is what makes battery operation simple.

| Display HAT | Display HAT pin | Pico | Pico pin |
|---|---|---|---|
| 3V3 | 1 | 3V3 OUT | 36 |
| GND | 6 | GND | 33 |
| 5V | 2 | not connected | |

Inky Impression 5.7" display, on the Pico SPI0:

| Signal | Display HAT GPIO | Display HAT pin | Pico GPIO | Pico pin |
|---|---|---|---|---|
| SCLK | GPIO 11 | 23 | GP18 | 24 |
| MOSI | GPIO 10 | 19 | GP19 | 25 |
| CS | GPIO 8 | 24 | GP17 | 22 |
| DC | GPIO 22 | 15 | GP20 | 26 |
| RESET | GPIO 27 | 13 | GP21 | 27 |
| BUSY | GPIO 17 | 11 | GP22 | 29 |

The 4 display HAT buttons (active low, internal pull-ups on the Pico; they close to the display HAT ground, already shared):

| Function | Button | Display HAT GPIO | Display HAT pin | Pico GPIO | Pico pin |
|---|---|---|---|---|---|
| execute / next page | A | GPIO 5 | 29 | GP2 | 4 |
| reset story | B | GPIO 6 | 31 | GP3 | 5 |
| change mode | C | GPIO 16 | 36 | GP4 | 6 |
| go back | D | GPIO 24 | 18 | GP5 | 7 |

Status LED, external as on the Pi version (`STATUS_LED=EXTERNAL`, the default). With `STATUS_LED=ONBOARD` nothing is wired and the board LED is used instead: GP25 on the Pico Plus 2, the wireless-chip LED on the Pico 2 W.

| Element | Pico | Pico pin |
|---|---|---|
| Status LED anode through a 220 ohm resistor | GP8 | 11 |
| Status LED cathode | GND | 13 |
| Mode LED (green sphere) anode through a 220 ohm resistor | GP16 | 21 |
| Mode LED cathode | GND | 23 |

The mode LED is a plain on/off output (`MODE_LED_PIN`); apart from the bursts at a mode change it is lit in AI mode only, when the board is awake with the WiFi anyway, so it costs nothing in story mode.

Story dial, `STORY_DIAL=ON` only (KY-040 module, its pins in the order GND, +, SW, DT, CLK):

| KY-040 pin | Pico | Pico pin |
|---|---|---|
| CLK | GP6 | 9 |
| DT | GP7 | 10 |
| SW (push button, acts as RESET) | GP9 | 12 |
| + (the module's two 10 k pull-ups, 0.3 mA) | GP14, driven high by the firmware (`DIAL_PIN_VCC`) | 19 |
| GND | GND | 8 |

The five dial wires thus sit on neighbouring pins of the left column, and 3V3 OUT keeps its single wire to the HAT. Set `DIAL_PIN_VCC` to -1 to feed "+" from a real 3.3 V instead (HAT pin 17 is a free 3V3).

microSD breakout (SPI, 3.3 V), `STORAGE=SDCARD` only:

| Signal | Pico GPIO | Pico pin |
|---|---|---|
| SCK | GP10 | 14 |
| MOSI | GP11 | 15 |
| MISO | GP12 | 16 |
| CS | GP13 | 17 |
| VCC | 3V3 OUT | 36 |
| GND | GND | 13 |

Display HAT pins 27 and 28 (ID EEPROM) stay unconnected.

## Power and battery

Consumption, order of magnitude (to be measured): a page costs about 10 to 18 J, i.e. around 1 mAh of a single LiPo cell, so even a 600 mAh battery is good for several hundred pages. What actually sets the autonomy is the time between pages: awake, the board draws 30 to 40 mA (60 to 80 mA with WiFi), about 15 h on 600 mAh, 50 h on the 2000 mAh 103450 cell of the enclosure. With the core powered off between pages (`DEEP_SLEEP=ON`, after 20 s idle) the RP2350 itself drops to about 150 uA; the floor is then the quiescent current of the charger board and of the SD card.

On USB, the Pico powers everything from its 3V3 OUT: display HAT and microSD. The display HAT's 5V pin is not needed: on the Inky Impression 5.7" it only feeds the small 10-pin expansion header, the panel voltages are made from 3V3 (checked on the Pimoroni schematic), so battery operation only needs VSYS. Two options for a rechargeable battery:

- **TP4056 module + single LiPo cell** (the enclosure in `case/` is built around it, see below).
- **Pimoroni Pico LiPo 2** (RP2350B, 16 MB flash, onboard charger and battery connector). Same build as the Pico Plus 2 (`PICO_BOARD=pimoroni_pico_plus2_rp2350`, the SDK 2.1.0 has no board file for it). No wireless unless an RM2 breakout is added.

### TP4056 wiring

The Pico has no charger. Its VBUS pin (40) is the 5V of its own USB socket, unregulated; it feeds the TP4056, which charges the cell. Plugging the Pico's USB therefore charges the battery, and the TP4056's own micro-USB stays unused.

| From | To | Role |
|---|---|---|
| Pico VBUS (pin 40) | TP4056 IN+ | USB 5V to the charger |
| Pico GND (pin 38) | TP4056 IN- and B- | Common ground |
| Battery + / - | TP4056 B+ / B- | |
| TP4056 B+ (or OUT+ on a module with protection) | Slide switch, centre pin | Battery output |
| Slide switch, one side pin | Diode anode (the end without the band) | |
| Diode cathode (banded end) | Pico VSYS (pin 39) | Powers the Pico |

```
Pico USB ──► VBUS (40) ──► TP4056 IN+ ──► charger ──► B+ ──► battery
                                                      │
                                   slide switch ◄─────┘
                                         │
                                       ──┤►──  1 A Schottky diode (1N5817, 1N5819, SS14; a 1N4007 also works)
                                         │
                                       VSYS (39) ──► Pico regulator ──► 3V3 ──► Pico, display HAT
```

The diode: inside the Pico, VBUS already reaches VSYS through a Schottky diode, so VSYS sits at about 4.7V when the USB is plugged. Without the external diode those 4.7V would reach the battery through the switch, uncontrolled. Oriented battery to VSYS, the diode lets the battery power the Pico but never the reverse. A 1N5711 is not suitable (15 mA signal diode).

| Situation | What happens |
|---|---|
| USB plugged, switch on | VSYS gets 4.7V from the USB and only 3.9V from the battery through the diode: the Pico runs on USB, the battery charges without any load, the TP4056 terminates properly |
| USB plugged, switch off | Charging only, Pico off |
| USB unplugged, switch on | Battery minus the diode drop into VSYS; the Pico regulator accepts down to 1.8V |

The TP4056 ships set to 1 A (Rprog 1.2 kΩ): charge from a mains USB adapter rather than a computer port, or fit 2 kΩ for about 580 mA. A module without B+/B- protection relies on the protection circuit of the cell itself (most 103450 pouch cells have one).

### Battery gauge

No extra component: the Pico has a 200 k / 100 k divider between VSYS and GP29 (ADC 3), so the firmware reads VSYS / 3 with the 12-bit ADC over its 3.3 V reference, averaged over 8 samples, then scaled by `BATTERY_CAL_METER_MV / BATTERY_CAL_CONSOLE_MV` (4760 / 4656 on the prototype: the reference and the divider read 2.2 % low). On battery, VSYS is the cell voltage minus the drop of the diode of the wiring above, a constant at the idle current of the board; `BATTERY_DIODE_DROP_MV` adds it back: 580 mV measured on the prototype, a silicon diode (a Schottky would give about 250). On a board with its own charger (Pico LiPo 2) set it to 0.

Six states are shown: 4, 3, 2, 1 or 0 bars, and the lightning bolt on USB. The outline turns red at one bar or none.

| Bars | Estimated cell voltage |
|---|---|
| 4 | 4.00 V and above |
| 3 | 3.80 V and above |
| 2 | 3.60 V and above |
| 1 | 3.45 V and above |
| 0 | below 3.45 V |

**Calibration**, two multimeter readings, nothing to flash in between:

1. *ADC chain, on USB.* The console prints `Battery: VSYS n mV` before each page. Compare with the multimeter between pin 39 (VSYS) and a ground pin, and put the two numbers in `BATTERY_CAL_METER_MV` and `BATTERY_CAL_CONSOLE_MV` (with the correction already active, multiply the existing ratio by the new one, or set both equal first).
2. *Diode drop, USB unplugged, switch on, board idle* (not during a refresh). Measure the cell on B+ / B- of the TP4056, then pin 39: the difference is the real drop. Put it in `BATTERY_DIODE_DROP_MV`: typically 150 to 250 mV for a 1N5817 / SS14 at 40 mA, 550 to 700 mV for a silicon 1N4007 (580 measured on the prototype).

The firmware's own reading on battery cannot be watched: without USB there is no console. Hence the multimeter-only method; the bars on the page are the only feedback once unplugged.

On USB, VSYS is the USB voltage through the Pico's own diode, about 4.7 V, whatever the cell: a reading above `BATTERY_USB_MV` (4.3 V) means USB power, and the symbol shows the lightning bolt. A cell never gives more than 4.0 V to VSYS through the diode, so the two cases cannot be confused.

On the Pico W family GP29 is also the clock of the SPI link to the wireless chip, so the reading holds the cyw43 driver for the millisecond it takes; the driver sets the pin back at its next transfer. The measurement is taken before each page, with the board idle at 30 to 40 mA, never during the panel refresh, where the current peaks and the voltage sags.

The thresholds (`BATTERY_BAR_MV`) are 150 to 200 mV apart and a LiPo spends most of its charge between 3.6 and 4.0 V, so the bars are honest to about one bar; a percentage would not be. The symbol turns red at one bar or none, time to recharge. That is all the firmware does with the reading: no action depends on the level, the protection circuit of the battery module (about 2.4 V) is what cuts the power on a flat cell.

## AI mode

Port of the online mode of `pi/hadistory.py`, with `gpt-4o-mini` for the text and `gpt-image-2` for the illustration (models in `hadistory_config.h`). The prompt is built like the Pi version from `prompts/prompts.txt` (only that file; `prompts_en.txt` and `prompts_examples.txt` are not used), which is embedded in the firmware at build time, so editing it means rebuilding. Each line is a theme drawn at random with equal chances; a `3:` prefix gives a line three chances, and `[a|b|c]`, `(a|b)` or `{a|b}` groups pick one option at random inside a line. The French incipit and excipit around the theme are in `src/prompt.cpp`.

The WiFi is only joined when needed: at boot if the saved mode is the AI mode, or when button C asks for it. The join runs in the background in the cyw43 driver and the main loop polls the link, so the buttons stay responsive meanwhile and the story mode never waits for the network. In AI mode a lost link is re-joined automatically; a generation requested while the link is down fails with the error blink.

Requests go to `api.openai.com` over TLS 1.2 with the server certificate verified against the two roots in `src/openai_roots.h`. If OpenAI changes its certificate chain one day, update that file (the roots were taken from the macOS system keychain) or build with `TLS_NO_VERIFY=ON` temporarily. The board has no clock, so certificate dates are not checked.

The illustration is requested from the image API as a 1024 x 1024 JPEG (`IMAGE_QUALITY`, `IMAGE_COMPRESSION` in `hadistory_config.h`), returned base64 encoded inside the JSON; it is decoded on the fly into the 160 KB image buffer while the response streams in, then decoded by `jpegdec` at half size and dithered into the page like a story page (see Display path). If the image step fails, the text alone is shown and the LED blinks the error pattern.

On button A in AI mode, the console shows the prompt, the generated story, the image download size and the decoding, then the page is displayed, with the robot badge top-left where a story page carries its counter.

Deep sleep stays off in AI mode: the power-down would drop the WiFi link. Switching back to story mode re-enables it.

## Stories in flash (`STORAGE=FLASH`)

`make` runs `tools/prepare_pages.py --embed`, which writes the JPEG and text of each page into the build folder and an assembly file that includes them with `.incbin`, plus a C table of stories. The JPEGs are decoded straight from the flash by XIP. The current state is saved in the last 4 KB sector of the flash.

The 12 current stories (97 pages) take about 4.5 MB at the default JPEG quality, which fits the 16 MB flash of the Pico Plus 2 / Pico LiPo 2 with room for many more. The `.uf2` file is twice that size, that is normal. Changing a story means rebuilding and reflashing.

On a 4 MB board such as the Pico 2 W, about 3 MB are left for pages once the firmware is in: a selection of stories with `EMBED_STORIES`, or all of them at a lower JPEG quality with `EMBED_QUALITY` (60 gives 2.6 MB for the 12 stories, see `tools/README.md`). For example:

```bash
cmake -S pico -B pico/build-flash -DPICO_BOARD=pico2_w -DSTORAGE=FLASH -DEMBED_STORIES="story_1;story_2"
cmake -S pico -B pico/build-flash -DPICO_BOARD=pico2_w -DSTORAGE=FLASH -DEMBED_QUALITY=60
```

Changing either variable re-runs the page preparation at the next `make`.

The linker refuses an image that does not fit, so an oversized selection fails at build time rather than on the board.

## SD card content (`STORAGE=SDCARD`)

Copy `tools/out/stories/` to the root of a FAT32 card:

```
/stories/story_1/p1.jpg + p1.txt ... pN.jpg + pN.txt
/stories/story_2/...
/currently.txt       written by the firmware: current story, page and mode
```

## Troubleshooting

Lessons from the first bring-up, in the order they bit:

- **Count the display HAT header pins from the right end and row.** Seen from the display HAT's female connector the numbering is mirrored. The even pins (2, 4, ..., 40) are the row along the board edge, pin 1 is at the end marked with a square pad. Check with a continuity tester before powering: pins 2 and 4 (5V) are adjacent and connected, pins 1 and 17 (3V3) are connected, and the grounds 6, 9, 14, 20, 25, 30, 34, 39 are all connected. A button line that reads 0 at rest is on a ground pin (29 and 30 are neighbours).
- **Buttons**: at rest a button line reads 1 (internal pull-up), pressed it reads 0, whether or not the display HAT is powered. The display HAT ground must be connected to the Pico ground.
- **`Test pattern: done in 605 ms`** instead of about 30 s: the driver never saw BUSY go low, so the panel did not answer. In practice: BUSY wire (display HAT pin 11), then 3V3 on display HAT pin 1, then RESET (pin 13).
- **`main.cpp` has a commented DEBUG block** after `display_init()`: an endless loop that prints each button press and the BUSY level every second, and on button A draws 7 colour bars, refreshes the panel, times it and reports how long BUSY was low. Uncomment it to check a new wiring step by step.
- **Flashing over USB fails** ("no accessible RP-series devices in BOOTSEL mode"): a serial monitor holds the port. Close it, or use the BOOTSEL button.
- **The USB serial console drops at each sleep**: the core is powered off, so the host sees the device go away and come back on the wake-up reboot (reopen the serial monitor). Build with `-DDEEP_SLEEP=OFF` while debugging, or press a button within the 20 second idle window.
- **Deep sleep history**: the first implementation used the XOSC dormant mode of pico-extras and never woke on the RP2350. The power manager path (P1.7, GPIO wake-up slots, reboot on wake) is the one Raspberry Pi documents for the RP2350. Three things were needed to make it work: a wake-up state that lists the SRAM banks in the same state as the sleep state, waiting for the power manager's WAITING bit beyond the SDK's short timeout, and the SLEEPDEEP bit of the Cortex-M33 before the wait-for-interrupt. The console prints the wake-up slot registers before sleeping and after waking. If the board wakes immediately or refuses to power down, a button line is probably low: check the pull-ups, the internal ones are kept in sleep, the internal pull-downs are not (erratum E9).
- **Long lines on the USB console are cut in the middle**, the end of the line being kept: an artefact of the console path, the data itself is complete (the firmware prints the character counts). Raising `PICO_STDIO_USB_STDOUT_TIMEOUT_US` did not remove it; try another terminal program if it matters.
- **`SubSample mode: 0x22`** on the console comes from the JPEG decoder library itself, it is harmless.
- **The FLASH variant uf2 is tiny** (a few hundred KB): nothing references the stories, typically because a debug loop never returns, so the linker dropped them. Remove the loop.

## Roadmap

- Dithering: Floyd-Steinberg on the board; Atkinson or Stucki could be tried if a lighter grain is wanted.
- Measure the real consumption awake, refreshing and dormant.
