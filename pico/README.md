# hadistory — Raspberry Pi Pico version

Port of hadistory to the RP2350 (Raspberry Pi Pico 2 W, Pimoroni Pico Plus 2 / Pico LiPo 2) driving the Inky Impression 5.7" (600 x 448, 7 colours, UC8159). Written in C++ on the Pico SDK with the Pimoroni libraries: `pico_graphics` and the `uc8159` display driver, `sdcard` and `fatfs` for the microSD variant.

Scope compared to the Pi version:
- **Story mode**: pre-rendered pages, either on a microSD card (`STORAGE=SDCARD`) or embedded in the firmware for boards with a big flash such as the Pico Plus 2 / Pico LiPo 2 (`STORAGE=FLASH`). Pages are produced by `tools/render_pages.py`, see `tools/README.md`.
- **AI mode** (optional, `WITH_GPT=ON`): OpenAI text and image generation over WiFi. No embedded models. Not implemented yet, the module is a stub.
- 4 buttons (execute, reset, change mode, go back) and a status LED, same behaviour as the Pi version.

## Layout

```
pico/
├── CMakeLists.txt          build configuration and options
├── main.cpp                button loop and mode switching (port of the Python main loop)
├── secrets.cmake.example   WiFi / OpenAI credentials template (copy to secrets.cmake, ignored by git)
└── src/
    ├── hadistory_config.h  pins per board, display geometry, behaviour constants
    ├── led.cpp/.h          status LED: solid = waiting, breathing = busy (timer driven), bursts = mode / error
    ├── buttons.cpp/.h      4 active-low buttons, edge detection
    ├── display.cpp/.h      PicoGraphics 3-bit frame buffer + Pimoroni UC8159 driver; the only path to the panel
    ├── storage.h           storage interface (stories catalogue, pages delivered row by row, persisted state)
    ├── storage_sdcard.cpp  STORAGE=SDCARD backend: FAT microSD, state in /currently.txt
    ├── storage_flash.cpp   STORAGE=FLASH backend: pages embedded in the image, state in the last flash sector
    ├── stories_embedded.h  catalogue type for the generated stories_table.c (FLASH)
    ├── story.cpp/.h        story mode logic
    ├── gpt.cpp/.h          AI mode (built only with WITH_GPT=ON)
    └── lwipopts.h          lwIP configuration for the AI mode
```

## Display path

Everything that reaches the panel goes through `display.cpp`: a `PicoGraphics_Pen3Bit` frame buffer (100 KB, allocated once) and the Pimoroni `UC8159` driver, which converts it to the panel format on the fly. Pages from the storage are 4 bpp colour indices (the BMP pixel data, see `tools/README.md`) and are pushed one row at a time with `set_pen(index)` / `set_pixel()`, so no second full-frame buffer exists. The AI mode will draw into the same PicoGraphics object (jpegdec, fonts). Changing to another Pimoroni panel means swapping the driver class and the geometry in `display.cpp` / `hadistory_config.h`, and re-rendering the pages for it.

## Build

Requirements: Pico SDK 2.x with `PICO_SDK_PATH` set, [`pimoroni-pico`](https://github.com/pimoroni/pimoroni-pico) checked out next to it (the CMake finds it there), `cmake`, `arm-none-eabi-gcc`, `picotool`.

```bash
# Story mode, pages on microSD, Pico 2 W
cmake -S pico -B pico/build -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD
make -C pico/build -j8

# Story mode, pages embedded in flash, Pico Plus 2 (needs python3 + Pillow + numpy at build time)
cmake -S pico -B pico/build-flash -DPICO_BOARD=pimoroni_pico_plus2_rp2350 -DSTORAGE=FLASH
make -C pico/build-flash -j8

# Story + AI mode (needs a board with wireless)
cp pico/secrets.cmake.example pico/secrets.cmake   # then fill it in
cmake -S pico -B pico/build-gpt -DPICO_BOARD=pico2_w -DSTORAGE=SDCARD -DWITH_GPT=ON
make -C pico/build-gpt -j8
```

Flash `hadistory.uf2` by holding BOOTSEL while plugging the board, or with `picotool load -f pico/build/hadistory.uf2`. Console output is on USB serial.

Build-time options:

| CMake variable | Effect |
|---|---|
| `PICO_BOARD` | Selects the pin mapping in `hadistory_config.h`. Known: `pico2_w`, `pico2`, `pimoroni_pico_plus2_rp2350`, `pimoroni_pico_plus2_w_rp2350`. |
| `STORAGE` | `SDCARD` (default): pages on a FAT microSD, `HADISTORY_STORAGE_SDCARD=1`. `FLASH`: pages rendered at build time and embedded in the firmware, `HADISTORY_STORAGE_FLASH=1`. |
| `WITH_GPT` | `ON` builds the AI mode (`HADISTORY_WITH_GPT=1` in the code). Default `OFF`. |
| `WIFI_SSID`, `WIFI_PASSWORD`, `OPENAI_API_KEY` | Credentials for the AI mode, via `-D` or `secrets.cmake`. |

## Wiring (default mapping in `hadistory_config.h`, Pico 2 W)

Inky Impression 5.7" HAT, signals taken from its 40-pin header:

| Signal | HAT pin (Pi GPIO) | Pico GPIO |
|---|---|---|
| SCLK | GPIO 11 | GP18 |
| MOSI | GPIO 10 | GP19 |
| CS | GPIO 8 | GP17 |
| DC | GPIO 22 | GP20 |
| RESET | GPIO 27 | GP21 |
| BUSY | GPIO 17 | GP22 |
| 3V3, GND | 3V3, GND | 3V3, GND |

Whether the HAT also needs 5V is still to be confirmed on the Pimoroni schematic.

microSD breakout (SPI, 3.3 V):

| Signal | Pico GPIO |
|---|---|
| SCK | GP10 |
| MOSI | GP11 |
| MISO | GP12 |
| CS | GP13 |

Buttons (to GND, internal pull-ups) and LED (through a 220 ohm resistor):

| Function | HAT button | Pico GPIO |
|---|---|---|
| execute / next page | A (GPIO 5) | GP2 |
| reset story | B (GPIO 6) | GP3 |
| change mode | C (GPIO 16) | GP4 |
| go back | D (GPIO 24) | GP5 |
| status LED | — | GP6 |

## Stories in flash (`STORAGE=FLASH`)

`make` runs `tools/render_pages.py --embed`, which renders the pages into the build folder and writes an assembly file that includes the pixel data of each BMP with `.incbin` (the 118-byte header is skipped), plus a C table of stories. The pages are read straight from the flash by XIP. The current state is saved in the last 4 KB sector of the flash.

The 12 current stories (97 pages) make a 13.1 MB image, which fits the 16 MB flash of the Pico Plus 2 / Pico LiPo 2 with about 3.6 MB to spare before the state sector. The `.uf2` file is twice that size, that is normal. Changing a story means rebuilding and reflashing.

## SD card content (`STORAGE=SDCARD`)

Copy `tools/out/stories/` to the root of a FAT32 card:

```
/stories/story_1/p1.bmp ... pN.bmp
/stories/story_2/...
/currently.txt       written by the firmware: current story, page and mode
```
