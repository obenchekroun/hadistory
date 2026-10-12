/********************************************************************************
 * hadistory - Pico version
 * hadistory_config.h - pins, display geometry and compile-time options
 *
 * Board-specific pins are selected with the macros the Pico SDK derives from
 * PICO_BOARD (RASPBERRYPI_PICO2_W, PIMORONI_PICO_PLUS2_RP2350, ...).
 * This file is force-included in every source file (see CMakeLists.txt).
 ********************************************************************************/
#ifndef HADISTORY_CONFIG_H
#define HADISTORY_CONFIG_H

// Pulls in the SDK board header so that the board macros used below are defined,
// whatever the include order (this file is force-included first, see CMakeLists.txt).
#include "pico/config.h"

// ---------------------------------------------------------------------------
// Features (set from CMake: -DWITH_GPT=ON, -DSTORAGE=SDCARD|FLASH)
// ---------------------------------------------------------------------------
#ifndef HADISTORY_WITH_GPT
#define HADISTORY_WITH_GPT 0
#endif
#if !defined(HADISTORY_STORAGE_SDCARD) && !defined(HADISTORY_STORAGE_FLASH)
#define HADISTORY_STORAGE_SDCARD 1
#endif

// ---------------------------------------------------------------------------
// Display: Inky Impression 5.7" (UC8159, 600 x 448, 7 colours)
// ---------------------------------------------------------------------------
#define DISPLAY_WIDTH        600
#define DISPLAY_HEIGHT       448
#define FRAME_BYTES          (DISPLAY_WIDTH * DISPLAY_HEIGHT / 2)   // 4 bpp: 134400
#define PAGE_BMP_PIXEL_OFFSET 118   // see tools/README.md: pixel data offset in pN.bmp
#define PAGE_BMP_SIZE        (PAGE_BMP_PIXEL_OFFSET + FRAME_BYTES)

// ---------------------------------------------------------------------------
// Pins
// ---------------------------------------------------------------------------
#if defined(RASPBERRYPI_PICO2_W) || defined(RASPBERRYPI_PICO2)
    // Display on hardware SPI0
    #define DISPLAY_SPI          spi0
    #define DISPLAY_PIN_SCK      18
    #define DISPLAY_PIN_MOSI     19
    #define DISPLAY_PIN_CS       17
    #define DISPLAY_PIN_DC       20
    #define DISPLAY_PIN_RESET    21
    #define DISPLAY_PIN_BUSY     22
    // SD card on PIO SPI (pimoroni sdcard driver)
    #define SDCARD_PIN_SPI0_SCK  10
    #define SDCARD_PIN_SPI0_MOSI 11
    #define SDCARD_PIN_SPI0_MISO 12
    #define SDCARD_PIN_SPI0_CS   13
    // Buttons, active low, internal pull-ups (same roles as the Inky Impression HAT)
    #define BUTTON_PIN_EXECUTE   2
    #define BUTTON_PIN_RESET     3
    #define BUTTON_PIN_MODE      4
    #define BUTTON_PIN_BACK      5
    // Status LED (PWM capable)
    #define LED_PIN              6
#elif defined(PIMORONI_PICO_PLUS2_RP2350) || defined(PIMORONI_PICO_PLUS2_W_RP2350)
    // Same mapping for now; adjust once the LiPo 2 / Plus 2 wiring is decided
    #define DISPLAY_SPI          spi0
    #define DISPLAY_PIN_SCK      18
    #define DISPLAY_PIN_MOSI     19
    #define DISPLAY_PIN_CS       17
    #define DISPLAY_PIN_DC       20
    #define DISPLAY_PIN_RESET    21
    #define DISPLAY_PIN_BUSY     22
    #define SDCARD_PIN_SPI0_SCK  10
    #define SDCARD_PIN_SPI0_MOSI 11
    #define SDCARD_PIN_SPI0_MISO 12
    #define SDCARD_PIN_SPI0_CS   13
    #define BUTTON_PIN_EXECUTE   2
    #define BUTTON_PIN_RESET     3
    #define BUTTON_PIN_MODE      4
    #define BUTTON_PIN_BACK      5
    #define LED_PIN              6
#else
    #error "Unsupported PICO_BOARD: add a pin mapping in src/hadistory_config.h"
#endif

// ---------------------------------------------------------------------------
// Behaviour
// ---------------------------------------------------------------------------
#define BUTTON_POLL_MS       100     // main loop period, as the 0.1 s sleep in hadistory.py
#define POST_ACTION_PAUSE_MS 3000    // pause after a page is shown, as in hadistory.py
#define STORIES_DIR          "/stories"        // SDCARD backend
#define SETTINGS_FILE        "/currently.txt"  // SDCARD backend
#define MAX_STORIES          64
#define STORY_NAME_LEN       32

#endif // HADISTORY_CONFIG_H
