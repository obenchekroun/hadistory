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
#ifndef HADISTORY_LED_ONBOARD
#define HADISTORY_LED_ONBOARD 0   // -DSTATUS_LED=ONBOARD sets it to 1
#endif
#ifndef HADISTORY_DEEP_SLEEP
#define HADISTORY_DEEP_SLEEP 0    // -DDEEP_SLEEP=ON sets it to 1
#endif
#ifndef HADISTORY_BATTERY
#define HADISTORY_BATTERY 0       // -DBATTERY_GAUGE=ON sets it to 1
#endif
#ifndef HADISTORY_STORY_DIAL
#define HADISTORY_STORY_DIAL 0    // -DSTORY_DIAL=ON sets it to 1
#endif

// ---------------------------------------------------------------------------
// Display: Inky Impression 5.7" (UC8159, 600 x 448, 7 colours)
// ---------------------------------------------------------------------------
#define DISPLAY_WIDTH        600
#define DISPLAY_HEIGHT       448
// Panel orientation, done in hardware by the UC8159 scan directions: ROTATE_0 or
// ROTATE_180 (pimoroni::Rotation). 180 puts the HAT buttons at the top.
#define DISPLAY_ROTATION     ROTATE_180
// Pages: a JPEG illustration (any size, fitted into a 448 px square) and a text
#define IMAGE_MAX_BYTES      (160 * 1024)   // JPEG buffer: SD card pages and OpenAI images
#define PAGE_TEXT_MAX        1024

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
    #define LED_PIN              8
    // Story dial (STORY_DIAL=ON): KY-040 CLK / DT, and its push button, a second RESET
    #define DIAL_PIN_CLK         6
    #define DIAL_PIN_DT          7
    #define DIAL_PIN_SW          9
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
    #define LED_PIN              8
    #define DIAL_PIN_CLK         6
    #define DIAL_PIN_DT          7
    #define DIAL_PIN_SW          9
#else
    #error "Unsupported PICO_BOARD: add a pin mapping in src/hadistory_config.h"
#endif

// Status LED selection. ONBOARD overrides LED_PIN with the board's own LED:
//  - a plain GPIO on most boards (PICO_DEFAULT_LED_PIN, e.g. GP25 on the Pico Plus 2), PWM capable
//  - on the Pico W / Pico 2 W the LED sits on the wireless chip (CYW43_WL_GPIO_LED_PIN):
//    no PWM, so the "breathing" becomes a 1 Hz blink (HADISTORY_LED_CYW43)
#if HADISTORY_LED_ONBOARD
    #if defined(PICO_DEFAULT_LED_PIN)
        #undef LED_PIN
        #define LED_PIN PICO_DEFAULT_LED_PIN
        #define HADISTORY_LED_CYW43 0
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        #define HADISTORY_LED_CYW43 1
    #else
        #error "STATUS_LED=ONBOARD: this board has no known onboard LED"
    #endif
#else
    #define HADISTORY_LED_CYW43 0
#endif
// The wireless chip driver (cyw43) is initialised by main() when one of these needs it
#define HADISTORY_HAS_CYW43 (HADISTORY_WITH_GPT || HADISTORY_LED_CYW43)

// ---------------------------------------------------------------------------
// Behaviour
// ---------------------------------------------------------------------------
#define BUTTON_POLL_MS       100     // main loop period, as the 0.1 s sleep in hadistory.py
#define POST_ACTION_PAUSE_MS 3000    // pause after a page is shown, as in hadistory.py
#define IDLE_BEFORE_SLEEP_MS 20000   // DEEP_SLEEP: power the core off after this long without a button press
#define STORIES_DIR          "/stories"        // SDCARD backend
#define SETTINGS_FILE        "/currently.txt"  // SDCARD backend
#define MAX_STORIES          64
#define STORY_NAME_LEN       32

// ---------------------------------------------------------------------------
// Battery gauge (BATTERY_GAUGE=ON): the cell voltage estimated from VSYS, see battery.cpp.
// Display only: no action in the firmware depends on the level
// ---------------------------------------------------------------------------
#define BATTERY_DIODE_DROP_MV 580    // diode between the battery and VSYS, at the idle current. Measured 2026-10-21 on the
                                     // prototype: cell 4.24 V on B+/B-, VSYS 3.66 V, a silicon diode (a Schottky gives about
                                     // 250). Multimeter, USB unplugged, board idle. 0 on a board with its own charger.
// ADC scale: the ADC reads VSYS / 3 over the nominal 3.3 V reference, and the real reference and the divider
// are off by a few percent. Measured on USB: multimeter 4760 mV on pin 39 while the console said 4656 mV.
// The reading is multiplied by METER / CONSOLE; set both equal to disable the correction.
#define BATTERY_CAL_METER_MV   4760
#define BATTERY_CAL_CONSOLE_MV 4656
#define BATTERY_USB_MV        4300   // VSYS at or above this means USB power (4.7 V); a cell never gives more than 4.0 V after the diode
#define BATTERY_BARS          4
#define BATTERY_BAR_MV        { 4000, 3800, 3600, 3450 }   // cell voltage for 4, 3, 2 and 1 bars; below the last: empty

// ---------------------------------------------------------------------------
// AI mode (WITH_GPT=ON), same models as pi/hadistory.py
// ---------------------------------------------------------------------------
#define OPENAI_HOST          "api.openai.com"
#define GPT_MODEL            "gpt-4o-mini"
#define IMAGE_MODEL          "gpt-image-2"
#define IMAGE_QUALITY        "low"   // gpt-image quality: low, medium, high, auto (time, cost and size)
#define IMAGE_COMPRESSION    40      // JPEG compression 0-100 asked to the API, keeps the image in IMAGE_MAX_BYTES
#define WIFI_CONNECT_TIMEOUT_MS 30000 // give up joining the network after this
#define GPT_TIMEOUT_S        60      // idle timeout of the chat request
#define IMAGE_TIMEOUT_S      120     // idle timeout of the image request (127 s max, lwIP poll limit)
#define STORY_MAX_LEN        PAGE_TEXT_MAX
// ---------------------------------------------------------------------------
// Story dial (STORY_DIAL=ON)
// ---------------------------------------------------------------------------
#define DIAL_POSITIONS       12      // numbers on the printed wheel; the travel is DIAL_POSITIONS - 1 detents
#define DIAL_REVERSE         0       // 1 if the numbers go the wrong way for the wiring / mounting side
#define DIAL_BLIP_MS         20      // status LED off this long at each counted detent

#ifndef HADISTORY_TLS_NO_VERIFY
#define HADISTORY_TLS_NO_VERIFY 0    // -DTLS_NO_VERIFY=ON sets it to 1
#endif

#endif // HADISTORY_CONFIG_H
