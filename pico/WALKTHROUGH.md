# hadistory, Pico version: a walkthrough of the code

One path through the firmware, from the build to the deep sleep, with the code that carries each idea and the reasons behind the odd lines. The snippets are abridged copies of the sources (a `...` marks a cut), the names are the real ones, so a search in `src/` lands on the full version. `README.md` says what the device does and how to build it; this file says how the code does it, and is meant to be enough on its own to understand the subtleties without opening the sources.

Reading order: the build first, because every file is shaped by its `#if` blocks; then the boot and the loop, which is the spine; then a page of a story, down through the storage and the rendering; then the parts the loop talks to, LEDs, sleep, the two dials, the AI mode, the battery.

1. [Options become macros](#1-options-become-macros)
2. [Boot](#2-boot)
3. [The loop](#3-the-loop)
4. [A page of a story](#4-a-page-of-a-story)
5. [Where the pages come from](#5-where-the-pages-come-from)
6. [Rendering a portrait page on a landscape panel](#6-rendering-a-portrait-page-on-a-landscape-panel)
7. [LEDs on timers](#7-leds-on-timers)
8. [Deep sleep is a reboot](#8-deep-sleep-is-a-reboot)
9. [Two dials behind one interface](#9-two-dials-behind-one-interface)
10. [AI mode: WiFi, HTTPS, OpenAI](#10-ai-mode-wifi-https-openai)
11. [Battery](#11-battery)
12. [Memory map and sizes](#12-memory-map-and-sizes)

## 1. Options become macros

### The options

Every build option is decided in `CMakeLists.txt` and reaches the code as a `HADISTORY_*` macro that is always defined, 0 or 1. The code never tests `#ifdef`, only `#if HADISTORY_X`, so a typo in a macro name is a compile error (`#if` on an undefined name is 0 in C, but `-Wundef` is not on; the defaults at the top of `hadistory_config.h` are the real safety net):

```c
#ifndef HADISTORY_WITH_GPT
#define HADISTORY_WITH_GPT 0
#endif
#if !defined(HADISTORY_STORAGE_SDCARD) && !defined(HADISTORY_STORAGE_FLASH)
#define HADISTORY_STORAGE_SDCARD 1
#endif
#ifndef HADISTORY_DEEP_SLEEP
#define HADISTORY_DEEP_SLEEP 0    // -DDEEP_SLEEP=ON sets it to 1
#endif
```

An option does two things in CMake: it adds the source files of the feature to the build, and it sets the macros. The dial is the typical case, with a third value:

```cmake
string(TOUPPER "${STORY_DIAL}" STORY_DIAL_KIND)
if(STORY_DIAL_KIND MATCHES "^(ON|TRUE|YES|Y|1|KY040|KY-040)$")
    set(STORY_DIAL_KIND KY040)
    list(APPEND hadistory_SRC src/dial.cpp)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=1 HADISTORY_DIAL_KY040=1 HADISTORY_DIAL_AS5600=0)
elseif(STORY_DIAL_KIND STREQUAL "AS5600")
    list(APPEND hadistory_SRC src/dial_as5600.cpp)
    list(APPEND my_lib hardware_i2c)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=1 HADISTORY_DIAL_KY040=0 HADISTORY_DIAL_AS5600=1)
elseif(STORY_DIAL_KIND MATCHES "^(OFF|FALSE|NO|N|0|)$")
    set(STORY_DIAL_KIND OFF)
    set(DIAL_DEFINES HADISTORY_STORY_DIAL=0 HADISTORY_DIAL_KY040=0 HADISTORY_DIAL_AS5600=0)
else()
    message(FATAL_ERROR "STORY_DIAL must be OFF, KY040 (or ON) or AS5600 (got '${STORY_DIAL}')")
endif()
```

`HADISTORY_STORY_DIAL` is what the common code tests (`main.cpp`, `story.cpp`: "is there a dial"), the two others pick the driver where the drivers differ (the wake-up slots in `power.cpp`, the push button in `buttons.cpp`, the calibration hold in `main.cpp`). A build without the AI mode has no trace of lwIP or mbedtls: `wifi.cpp`, `gpt.cpp`, `https.cpp`, `json.cpp`, `prompt.cpp`, `base64.cpp` are simply not in the source list, and `pico_cyw43_arch_lwip_threadsafe_background`, `pico_lwip_mbedtls`, `pico_mbedtls` are not linked.

### The force-included header

`src/hadistory_config.h` holds the defaults, the pins of each board and every behaviour constant. It is force-included in front of every source file of the target, the Pimoroni drivers included:

```cmake
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:-include ${CMAKE_CURRENT_LIST_DIR}/src/hadistory_config.h>
    -Wall
)
```

The reason is the SD card driver: it reads its pins from macros named `SDCARD_PIN_SPI0_SCK` and so on, and the force-include is how those names reach a file this project does not own. The side effect is that the header must be harmless in C as well as C++, and must not drag the SDK's platform layer in front of third-party sources. That is what the odd lines at its top are about:

```c
#ifndef pico_board_cmake_set
#define pico_board_cmake_set(x, y)
#endif
#ifndef pico_board_cmake_set_default
#define pico_board_cmake_set_default(x, y)
#endif
#include "pico/config.h"
```

The pin mapping below needs the board macros (`RASPBERRYPI_PICO2_W`, ...), which the board header defines, and `pico/config.h` is the SDK's way to include the board header. Since SDK 2.2 the board headers also carry `pico_board_cmake_set(...)` lines, meant for CMake to read with a regular expression, that only `pico.h` defines away before including the config. This header includes `pico/config.h` directly, not `pico.h`, so it defines the same two no-op macros first, spelt exactly as in `pico.h` so that the SDK's own definitions, met later, are identical repeats and not redefinitions. Without them the first build with SDK 2.3.1 failed in every file, on line 17 of `pico2_w.h`.

### The pin mapping

One `#if defined(...)` block per board family, and an `#error` for an unknown board, so a wrong `PICO_BOARD` cannot build with random pins:

```c
#if defined(RASPBERRYPI_PICO2_W) || defined(RASPBERRYPI_PICO2) || defined(WAVESHARE_RP2350B_PLUS_W)
    #define DISPLAY_SPI          spi0
    #define DISPLAY_PIN_SCK      18
    ...
    #define BUTTON_PIN_EXECUTE   2
    #define BUTTON_PIN_RESET     3
    #define BUTTON_PIN_MODE      4
    #define BUTTON_PIN_BACK      5
    #define LED_PIN              15
    #define MODE_LED_PIN         16
    #define DIAL_PIN_CLK         6      // KY-040: CLK DT SW + on GP6 GP7 GP8 GP9, GND on pin 13
    #define DIAL_PIN_DT          7
    #define DIAL_PIN_SW          8
    #define DIAL_PIN_VCC         9
    #define DIAL_I2C             i2c1   // AS5600 on the same five pins: SDA SCL DIR VCC GND
    #define DIAL_PIN_SDA         6
    #define DIAL_PIN_SCL         7
    #define DIAL_PIN_DIR         8
#elif defined(PIMORONI_PICO_PLUS2_RP2350) || defined(PIMORONI_PICO_PLUS2_W_RP2350)
    ...
#else
    #error "Unsupported PICO_BOARD: add a pin mapping in src/hadistory_config.h"
#endif
```

Two subtleties. The dial's five pins are the same for both sensors, so the ribbon does not change with the firmware; GP6 and GP7 are CLK / DT for the encoder and SDA / SCL of `i2c1` for the sensor (I2C1 is on GP6 / GP7 on the RP2350, I2C0 is not: that is why `i2c1`). And `DIAL_PIN_VCC` is a GPIO, not a 3V3 pin: the dial driver drives it high and the sleep code drives it low, so a sensor fed from it draws nothing while the board sleeps; `-1` means "wired to 3V3, do not touch it".

The status LED selection comes after the mapping, because `STATUS_LED=ONBOARD` has to override `LED_PIN` with the board's own LED, and on the Pico W family that LED is not a GPIO at all:

```c
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
#define HADISTORY_HAS_CYW43 (HADISTORY_WITH_GPT || HADISTORY_LED_CYW43)
```

`HADISTORY_HAS_CYW43` is "the wireless chip driver is initialised": true with the AI mode, true with the onboard LED of a Pico W. It decides whether `main()` calls `cyw43_arch_init()`, whether `power_sleep()` deinitialises it, and whether the battery reading has to borrow the pin from the driver (section 11). CMake has the same logic for the link line: `pico_cyw43_arch_none` (the driver without a network stack) is linked for the LED alone.

### Generated files

Three files are generated at configure or build time and included like sources:

- `generated/wifi_networks.h` (AI mode): CMake turns the `WIFI_NETWORKS` list of `secrets.cmake` into the initialiser of a C array. A CMake list is semicolon separated, and so is the `ssid;password;ssid;password` form, so a semicolon inside a name or a password is written `\;` and is swapped for `@SEMI@` while the list is split, then put back. `WIFI_SSID` / `WIFI_PASSWORD`, the older single-network form, are put in front. Backslashes and double quotes are escaped for C. An empty list gives one dummy `{ "", "" }` entry so that the array is never empty, with `WIFI_NETWORKS_COUNT` 0.
- `prompts_data.S` (AI mode): an assembly file with `.incbin "prompts/prompts.txt"` and a `.byte 0`, which makes the prompts file a NUL-terminated string in flash named `hadistory_prompts_txt`, with no copy at run time. The file is a dependency of the object, so editing the prompts rebuilds.
- `embedded_stories/stories_data.S` and `stories_table.c` (`STORAGE=FLASH`): the same `.incbin` trick for every page, see section 5. The custom command depends on the story sources, the script and a stamp file holding the selection and the quality, so changing `-DEMBED_STORIES` or `-DEMBED_QUALITY` re-runs the preparation.

`configure_file(... COPYONLY)` is used for `wifi_networks.h` rather than `file(WRITE)` alone so that the header's timestamp only changes when its content does, and a reconfigure does not rebuild `wifi.cpp` for nothing.

### The summary

The configure step ends with a summary of the build (board and SDK version, storage and what is embedded, dial, sleep, gauge, LED, and for the AI mode whether the secrets were found, whether the key is set and the networks in their order, names only). It exists because several build directories live side by side (`build`, `build-flash`, `build-gpt`, `build-as`) and the one mistake that costs an hour is flashing the wrong one.

## 2. Boot

### Cold boot or wake-up

`main()` starts by asking whether this boot is a wake-up from the deep sleep, because then the reader has already pressed a button and nothing must be printed or waited for:

```cpp
int main(void) {
    stdio_init_all();
#if HADISTORY_DEEP_SLEEP
    bool woke = power_woke_from_sleep();
    button_t wake_button = woke ? power_wake_button() : BUTTON_COUNT;
#else
    bool woke = false;
#endif
    puts(woke ? "Welcome back to hadistory !" : "Welcome to hadistory !");
```

`power_wake_button()` is called before anything else because it reads the power manager's registers, which keep the status of the slot that fired until something clears them, and as a fallback the level of the button pins, which the reader is probably still holding (section 8). `BUTTON_COUNT` is the "none" value of the `button_t` enum, used throughout as a sentinel.

### Initialisation order

```cpp
#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
    if (cyw43_arch_init()) puts("cyw43: init failed");
#endif
    led_init();
    led_mode_init();
    buttons_init();
    display_init();
#if HADISTORY_BATTERY
    battery_init();
#endif
    sd_ok = storage_init();
    if (sd_ok && storage_load_settings(&settings)) {
        puts("Currently on loaded !");
    } else {
        puts("No saved state, starting on local story mode.");
        memset(&settings, 0, sizeof settings);
        settings.dial_zero = -1;
    }
    story_init(&settings);
#if HADISTORY_STORY_DIAL
    dial_init(settings.dial, settings.dial_detents, settings.dial_zero);
    dial_set_moved(settings.dial_moved);   // a turn before the sleep still counts after the wake-up
    bool dial_dirty = false;
#if HADISTORY_DIAL_AS5600
    if (dial_position() != settings.dial) { dial_set_moved(true); dial_dirty = true; }
    absolute_time_t reset_down_since = get_absolute_time();
    bool reset_hold_done = true;
#endif
#endif
#if HADISTORY_WITH_GPT
    wifi_init();
    if (settings.ai_mode) wifi_connect();   // resume in AI mode: join in the background
    bool ai_switch_pending = false;
#else
    settings.ai_mode = false;   // no AI mode in this build
#endif
```

The order matters in two places. The wireless chip comes first because the Pico 2 W's LED hangs off it (`led_init()` would drive a chip that is not up). The settings are loaded before `story_init()` because the catalogue check needs them: a saved story that is no longer there (card changed, stories re-embedded) resets the state.

Despite its name, `sd_ok` means "the storage is usable". With the flash backend it is always true; with the card it is false when the mount failed, and then nothing is ever saved (`save_settings()` tests it), so a device without a card still runs, with a random story and no memory.

### What the firmware remembers

Everything the firmware keeps between two boots is one struct, loaded here and saved after every action:

```c
typedef struct {
    char story[STORY_NAME_LEN];   // "" when no story is chosen
    int page;                     // 0 = none shown yet
    bool ai_mode;
    int dial;                     // STORY_DIAL: wheel position 1..DIAL_POSITIONS, 0 = unknown
    bool dial_moved;              // STORY_DIAL: the wheel moved since the last RESET / story start (next A reads it)
    int dial_detents;             // STORY_DIAL=KY040: detents from the lower stop (finer than dial)
    int dial_zero;                // STORY_DIAL=AS5600: raw angle of the "1" found by the calibration, -1 = none
} settings_t;
```

`save_settings()` in `main.cpp` is the only writer, and it first copies the dial's state into the struct, so the dial driver never knows about the storage:

```cpp
static void save_settings(void) {
#if HADISTORY_STORY_DIAL
    settings.dial = dial_position();
    settings.dial_moved = dial_moved();
    settings.dial_detents = dial_detents();
    settings.dial_zero = dial_zero();
#endif
    if (sd_ok && !storage_save_settings(&settings)) puts("Could not save state");
}
```

Two lines of the boot are about the dial's memory across the sleep. `dial_set_moved(settings.dial_moved)` restores the "the wheel was turned" flag, so a turn made just before the board fell asleep still selects the story at the next A. And for the AS5600, which reads its position instead of counting it, a wheel found at boot on another number than the one saved means it was turned while the board slept: the flag is set and the state marked dirty so that it is saved at the next opportunity.

### The first press

```cpp
    if (woke) {
#if HADISTORY_DEEP_SLEEP
        if (wake_button < BUTTON_COUNT) buttons_inject_press(wake_button);   // act on the button that woke us
#endif
        led_mode_set(settings.ai_mode);   // no announcement on a wake-up, just the state
    } else {
        announce_mode();
    }
    wait_for_press();
```

On a wake-up the button is injected into the button module (section 3) so that the loop handles it at its first turn exactly as if it had just been pressed: a press on A during the sleep shows the next page about a second after the press, the boot time, with no welcome, no bursts, no pause. On a cold boot the mode is announced (the console line and, if `MODE_LED_BURSTS` is on, the bursts) and the firmware waits.

## 3. The loop

### Polling and edges

The loop is the Python one of `pi/hadistory.py`, 100 ms a turn (`BUTTON_POLL_MS`, the `time.sleep(0.1)` of the Python). Buttons are polled, not interrupt driven: the loop is slow enough for a human and a poll is immune to contact bounce as long as the state is read once per turn. A press is a falling edge between two polls:

```cpp
static bool previous_state[BUTTON_COUNT];   // true = released (pull-up), as in hadistory.py
static bool current_state[BUTTON_COUNT];
static bool injected[BUTTON_COUNT];         // press to report on the next poll (wake from sleep)

void buttons_poll(void) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (injected[i]) {
            injected[i] = false;             // present the injected press as a falling edge for this poll
            previous_state[i] = true;
            current_state[i] = false;
        } else {
            previous_state[i] = current_state[i];
            current_state[i] = read_button(i);
        }
    }
}
bool buttons_pressed(button_t b) { return previous_state[b] && !current_state[b]; }
bool buttons_down(button_t b)    { return !current_state[b]; }
```

The states are initialised to `true` (released) in `buttons_init()`, so a button held down at a cold boot is seen as one press at the first poll, not as a stuck press. `buttons_down()` is the level, used only by the AS5600 calibration hold. With the KY-040, the push button of the dial is wired-OR with button B in software:

```cpp
static bool read_button(int i) {
    bool released = gpio_get(button_gpio[i]);
#if HADISTORY_DIAL_KY040
    if (i == BUTTON_RESET) released = released && gpio_get(DIAL_PIN_SW);
#endif
    return released;
}
```

An injected press is consumed by the poll that reports it: `current_state` is false for that one turn, and at the next poll `read_button()` takes over. If the reader is still holding the button, the next poll reads it down with `previous_state` false too: no second edge. That is the whole trick that makes the wake-up button act once.

### The skeleton

```cpp
    absolute_time_t idle_since = get_absolute_time();
    while (true) {
        buttons_poll();
        bool any_press = false;
        for (int b = 0; b < BUTTON_COUNT; b++) if (buttons_pressed((button_t)b)) any_press = true;
        ... dial steps, calibration hold, WiFi poll (below) ...
        if (buttons_pressed(BUTTON_EXECUTE)) {
            led_breathe_start();
            battery_update();
            if (settings.ai_mode) {
                story_reset(&settings);
                save_settings();
                if (!gpt_generate_page()) led_rapid_blink();
            } else {
                if (!story_next_page(&settings)) led_rapid_blink();
                save_settings();
            }
            led_breathe_stop();
            sleep_ms(POST_ACTION_PAUSE_MS);
            wait_for_press();
        } else if (buttons_pressed(BUTTON_BACK) && !settings.ai_mode) {
            if (settings.story[0] && settings.page > 1) {
                ... same shape around story_previous_page() ...
            } else {
                puts("Nothing to show, skipping ...");
            }
        } else if (buttons_pressed(BUTTON_RESET)) {
            story_reset(&settings);
            dial_set_position(1);   // KY-040, the ritual: wheel at the lower stop, RESET, turn to the story, GO
            dial_dirty = false;
            save_settings();
        } else if (buttons_pressed(BUTTON_MODE)) {
            ... (below) ...
        }
        if (any_press) idle_since = get_absolute_time();
        ... sleep (below) ...
        sleep_ms(BUTTON_POLL_MS);
    }
```

The chain is `if ... else if`, so two buttons pressed in the same 100 ms are served in priority order, A first, and the other is lost (its edge was consumed by this poll). This is the Python behaviour and nobody presses two buttons at once on purpose.

Every action has the same shape: breathing LED on, battery read, the action, breathing off, a 3 s pause (`POST_ACTION_PAUSE_MS`, so that a second press during the pause is ignored: the panel is still settling), then "Waiting for next button press..." and the LED solid. The battery is read before the action, while the board is idle, and the page composers draw the stored value afterwards (section 11).

In the AI mode the order around `save_settings()` differs on purpose: the story state is reset and saved *before* the generation, which can take two minutes and may be interrupted by a power loss; in the story mode the page is shown first and saved after, so that a failure to show it leaves the previous page as the saved one.

### Things that count as presses

Two events are not buttons but set `any_press`:

```cpp
#if HADISTORY_STORY_DIAL
        if (int steps = dial_take_steps()) {
            printf("Dial: %+d -> %d\n", steps, dial_position());
            dial_dirty = true;
            any_press = true;
            bool quiet = settings.ai_mode;
#if HADISTORY_WITH_GPT
            quiet = quiet || ai_switch_pending;   // the LED is busy breathing
#endif
            if (!quiet) { led_off(); sleep_ms(DIAL_BLIP_MS); led_on(); }
        }
#endif
```

A counted step of the dial blips the status LED (off for 50 ms) and keeps the board awake, and marks the position dirty; it is not saved at once because flash writes are slow and wear the sector, it is saved by the next action or just before the sleep. `quiet` avoids fighting the breathing LED: the blip would call `led_off()`, which stops the breathing.

The other is the WiFi joining in the background while a switch to the AI mode is pending (next paragraph). Both matter for the idle timer: `idle_since` is reset at the end of the turn, after the action, so the 20 s count from the end of a page refresh, not from the press that started it.

### Mode switch and the WiFi

```cpp
#if HADISTORY_WITH_GPT
        wifi_state_t wifi = wifi_poll();
        if (ai_switch_pending) {
            if (wifi == WIFI_UP) {
                ai_switch_pending = false;
                settings.ai_mode = true;
                led_breathe_stop();
                announce_mode();
                save_settings();
                led_on();
            } else if (wifi == WIFI_FAILED) {
                ai_switch_pending = false;
                puts("AI mode refused: no WiFi. Staying in story mode.");
                led_mode_set(false);
                led_breathe_stop();
                led_rapid_blink();
                led_on();
            }
            any_press = true;   // keep the board awake while connecting
        } else if (settings.ai_mode && (wifi == WIFI_OFF || wifi == WIFI_FAILED)) {
            wifi_connect();   // link lost or failed at boot: keep trying while in AI mode
        }
#endif
```

A press on C does not switch to the AI mode: it *asks* for it. If the link is already up the switch is immediate; otherwise `ai_switch_pending` is set, `wifi_connect()` starts the scan, the status LED breathes and the mode LED blinks, and the switch completes, or is refused, in a later turn when the WiFi state machine reaches `UP` or `FAILED`. Meanwhile the reader can still press A for a story page, and the board does not fall asleep (`any_press`). Pressing C again while pending changes nothing: `settings.ai_mode` is still false, so the handler takes its last branch again and restates the request, `wifi_connect()` being a no-op while a scan or a join runs. Only once in the AI mode does C switch back, and it clears a pending request on the way. In the AI mode a lost link is retried forever, silently: the next A will say "WiFi is connecting, cannot generate" if it is not back.

### The sleep decision

```cpp
#if HADISTORY_DEEP_SLEEP
        if (!settings.ai_mode && absolute_time_diff_us(idle_since, get_absolute_time()) > IDLE_BEFORE_SLEEP_MS * 1000) {
            puts("Going to sleep, press any button to wake up.");
#if HADISTORY_STORY_DIAL
            if (dial_dirty) { save_settings(); dial_dirty = false; }   // keep the wheel position across the sleep
            dial_sleep();             // AS5600: sensor off while asleep
#endif
            led_off();
            power_sleep();            // reboots on a button press, returns only on failure
#if HADISTORY_STORY_DIAL
            dial_init(dial_position(), dial_detents(), dial_zero());
#endif
            led_on();
            idle_since = get_absolute_time();
        }
#endif
```

The AI mode never sleeps: the power-down would kill the WiFi link, and the mode is meant for a session with an adult around. `power_sleep()` normally never returns, the wake-up being a reboot; it returns when the power manager refuses (a button held down, a bad state), and then the dial is re-initialised because `dial_sleep()` has powered the sensor off, and the loop goes on as if nothing had happened, trying again 20 s later.

## 4. A page of a story

### The catalogue

`story.cpp` is the story mode. At boot it lists the stories through the storage interface and sorts them by the number at the end of their name, so that the dial's 1 to 12 follow `story_1` to `story_12` and not the alphabetical `story_1, story_10, story_11, story_12, story_2`:

```cpp
static int story_number(const char *name) {      // story_10 -> 10, -1 without one
    size_t n = strlen(name), i = n;
    while (i > 0 && name[i - 1] >= '0' && name[i - 1] <= '9') i--;
    return i < n ? atoi(name + i) : -1;
}
static int story_compare(const story_info_t *a, const story_info_t *b) {
    int na = story_number(a->name), nb = story_number(b->name);
    if (na < 0 && nb >= 0) return 1;      // unnumbered after the numbered
    if (nb < 0 && na >= 0) return -1;
    if (na != nb) return na < nb ? -1 : 1;
    return strcmp(a->name, b->name);
}
```

The sort is an insertion sort: 64 entries at most (`MAX_STORIES`), once at boot, no need for `qsort` and its callback cast. `story_index()` gives the 1-based position of a story in that order, which is the number printed before the page counter (`3: 1/9`) and the number on the wheel.

### Picking and stepping

A press on A is `story_next_page()`: pick a story when none is running or the last page was shown, then show the next page:

```cpp
bool story_next_page(settings_t *settings) {
    if (catalogue_size == 0) return false;
    if (settings->page >= story_page_count(settings)) story_reset(settings);
    if (settings->story[0] == '\0') {
        const story_info_t *pick;
#if HADISTORY_STORY_DIAL
        if (dial_moved()) {
            int pos = dial_position();
            if (pos > catalogue_size) pos = catalogue_size;   // beyond the catalogue: the last story
            pick = &catalogue[pos - 1];
            printf("Dial at %d: story %s\n", dial_position(), pick->name);
        } else {
            pick = random_pick();
            printf("Dial untouched: story %s at random\n", pick->name);
        }
        dial_set_moved(false);   // the next pick needs a new turn of the wheel
#else
        pick = random_pick();
#endif
        snprintf(settings->story, STORY_NAME_LEN, "%s", pick->name);
        settings->page = 0;
    }
    settings->page++;
    if (!show_current_page(settings)) return false;
    puts(settings->page >= story_page_count(settings) ? "The end." : "To be continued...");
    return true;
}
```

The first line handles the end of a story lazily: the last page stays on the panel and saved, and it is the *next* A that resets and picks. That is why the dial only speaks at a boundary, after the end or after a RESET, and why turning it during a story changes nothing: the wheel is an override of the random pick, consulted once, and cleared once consulted. `story_reset()` remembers the name of the story just left in `last_story`, and the random pick avoids it when there is a choice:

```cpp
static const story_info_t *random_pick(void) {
    const story_info_t *pick;
    do {
        pick = &catalogue[get_rand_32() % catalogue_size];
    } while (catalogue_size > 1 && strcmp(pick->name, last_story) == 0);
    return pick;
}
```

`get_rand_32()` is the SDK's generator, seeded from the ring oscillator and the time, so the sequence differs at every boot. The guard `catalogue_size > 1` keeps the loop finite with a single story. `story_previous_page()` is the mirror, refused on page 1 or without a story.

### Showing a page

One sequence every page goes through, story or AI: load, lay out the text, decode the illustration into the square left, the counter, the battery, then the panel update:

```cpp
static bool show_current_page(const settings_t *settings) {
    printf("Let's show the following story : %s, on page %d/%d\n", ...);
    page_data_t page;
    if (!storage_load_page(settings->story, settings->page, &page)) return false;
    render_layout_t layout = render_text(page.text);
    if (!render_jpeg(layout, page.jpeg, page.jpeg_len)) return false;
#if HADISTORY_STORY_DIAL
    render_page_counter(story_index(settings->story), settings->page, story_page_count(settings));
#else
    render_page_counter(0, settings->page, story_page_count(settings));
#endif
    render_battery();
    display_update();
    return true;
}
```

A failure anywhere before `display_update()` leaves the panel as it was and returns false, which the loop turns into the error blink; the settings then record the page that failed, so the next A tries the page after it (the Python did the same). The counter and the battery are drawn last, over the illustration, on white boxes.

## 5. Where the pages come from

### The interface

`storage.h` is a five-function interface with two implementations chosen at build time; a page is a JPEG plus a text, wherever they live:

```c
typedef struct { char name[STORY_NAME_LEN]; int page_count; } story_info_t;
typedef struct {
    const uint8_t *jpeg;          // in flash, or in the shared image buffer (SD card)
    size_t jpeg_len;
    const char *text;             // NUL terminated
} page_data_t;

bool storage_init(void);
int  storage_list_stories(story_info_t *out, int max);     // returns count
bool storage_load_page(const char *story, int page, page_data_t *out);
bool storage_load_settings(settings_t *s);
bool storage_save_settings(const settings_t *s);
```

The pointers in `page_data_t` are the whole difference between the backends: with the flash they point into the firmware image, with the card into buffers that the next load overwrites. The callers never free anything and never keep a page across two loads.

### FLASH: pages read in place

`tools/prepare_pages.py` resizes the illustrations to 448 px, re-encodes them as JPEG at `EMBED_QUALITY` and copies the texts; with `--embed` it also writes two files that the build compiles:

```asm
    .section .rodata.hadistory_stories, "a"
    .balign 4
    .global hadistory_story_1_p1_jpg
hadistory_story_1_p1_jpg:
    .incbin "/.../prepared_pages/story_1/p1.jpg"
    .global hadistory_story_1_p1_txt
hadistory_story_1_p1_txt:
    .incbin "/.../prepared_pages/story_1/p1.txt"
    .byte 0
```

```c
extern const uint8_t hadistory_story_1_p1_jpg[];
extern const char hadistory_story_1_p1_txt[];
static const embedded_page_t story_1_pages[] = { { hadistory_story_1_p1_jpg, 23412, hadistory_story_1_p1_txt }, ... };
const embedded_story_t embedded_stories[] = {
    { "story_1", 3, story_1_pages },
    ...
};
const int embedded_story_count = 12;
```

(The sizes are illustrative, the rest is what `write_embedded()` in the script writes; the `.balign 4` keeps every JPEG word aligned, the `.byte 0` terminates the text.) `.incbin` makes the assembler copy the file's bytes into the object, so the pages sit in the `.rodata` of the firmware, in flash, and loading a page is three pointer assignments, read through the XIP cache like any constant:

```cpp
bool storage_load_page(const char *story, int page, page_data_t *out) {
    for (int i = 0; i < embedded_story_count; i++) {
        if (strcmp(embedded_stories[i].name, story) != 0) continue;
        if (page < 1 || page > embedded_stories[i].page_count) return false;
        const embedded_page_t *p = &embedded_stories[i].pages[page - 1];   // read in place from XIP flash
        out->jpeg = p->jpeg;
        out->jpeg_len = p->jpeg_len;
        out->text = p->text;
        return true;
    }
    return false;
}
```

The JPEG decoder reads from that pointer directly (`openRAM` on a flash address works: it is memory), so the 160 KB image buffer of `render.cpp` is unused in this mode by the story pages. The total depends on the quality (`tools/README.md` has the table; the largest page at quality 85 is about 95 KB), and the Waveshare board's 16 MB of flash is what allows a higher quality or more stories than the Pico 2 W's 4 MB, which is the reason the project carries a board header for it.

### FLASH: the settings sector

The settings go in the last 4 KB sector of the flash, as a record with a magic word, so that a blank sector (all `0xFF`) reads as "no settings":

```cpp
#define SETTINGS_MAGIC         0x48414449u   // "HADI"
#define SETTINGS_FLASH_OFFSET  (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

typedef struct {
    uint32_t magic;
    char story[STORY_NAME_LEN];
    int32_t page;
    uint32_t ai_mode;
    int32_t dial;        // added with the story dial; an older record reads -1 here, clamped by dial_init()
    int32_t dial_moved;  // an older record reads -1: treated as moved
    int32_t dial_detents;
    int32_t dial_zero;   // AS5600 raw angle of the "1" (an older record reads -1: not calibrated)
} settings_record_t;
```

The fields added over time are at the end on purpose: a record written by an older firmware is shorter, and the bytes after it are the sector's erased `0xFF`, which read as `-1` in an `int32_t`. Every consumer of those fields treats `-1` as "unknown" (`dial_init()` clamps the position and recomputes the detents, the AS5600 sees "not calibrated"), so a firmware update never needs a settings reset. Reading is a cast of the flash address:

```cpp
const settings_record_t *rec = (const settings_record_t *)(XIP_BASE + SETTINGS_FLASH_OFFSET);
if (rec->magic != SETTINGS_MAGIC) return false;
```

Writing is the one dangerous operation of the firmware. The flash cannot be read while it is being erased or programmed, and the code itself executes from the flash, so the two SDK calls (which run from RAM) must not be interrupted by anything that would fetch an instruction from the flash:

```cpp
bool storage_save_settings(const settings_t *s) {
    static uint8_t buf[FLASH_PAGE_SIZE] __attribute__((aligned(4)));
    settings_record_t rec; ... fill ...
    memset(buf, 0xFF, sizeof buf);
    memcpy(buf, &rec, sizeof rec);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(SETTINGS_FLASH_OFFSET, buf, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    return true;
}
```

A sector erase takes tens of milliseconds, during which the LED timer, the USB console and the dial's interrupts are held; the breathing LED freezes for a blink, which is visible if you look for it. The record is padded to one flash page (256 bytes) because `flash_range_program()` writes whole pages. The AI mode keeps its own sector just before this one for the networks learnt at run time (`STORE_OFFSET = PICO_FLASH_SIZE_BYTES - 2 * FLASH_SECTOR_SIZE`), with the same recipe; the linker never puts code there because the firmware is far smaller than the flash, but nothing checks it: a firmware bigger than the flash minus 8 KB would corrupt itself at the first save.

### SDCARD: FatFs over PIO SPI

The same five functions over ChaN's FatFs and Pimoroni's SD card driver (SPI bit-banged on a PIO state machine, which is why the pins are called `SDCARD_PIN_SPI0_*` whatever hardware SPI the display uses). The card holds `/stories/<name>/p1.jpg`, `p1.txt`, `p2.jpg` ... as written by `tools/prepare_pages.py --out`, and the catalogue is a directory listing where the page count comes from probing `pN.txt` until one is missing:

```cpp
static int count_pages(const char *story) {
    char path[64];
    int n = 0;
    for (;;) {
        FILINFO fi;
        snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.txt", story, n + 1);
        if (f_stat(path, &fi) != FR_OK) break;
        n++;
    }
    return n;
}
```

A page load reads the JPEG into the shared 160 KB image buffer of `render.cpp` (a file bigger than that is refused with a message, not truncated) and the text into a static 1 KB buffer, NUL-terminated by the loader because the file is not. The settings are `key=value` lines in `/currently.txt`, parsed with `strncmp` on each prefix, so that a line an older firmware did not write leaves its default (`dial_zero` is preset to `-1` before the parse for that reason).

## 6. Rendering a portrait page on a landscape panel

### The rotation

`render.cpp` composes the page of the Pi version, portrait, 448 wide and 600 tall, illustration square on top, text under it, straight into the panel's frame buffer, which is landscape, 600 by 448. One inline function does the rotation, the same transform as `Image.rotate(90, expand=1)` in the Python, and every drawing call goes through it or through its rectangle form:

```cpp
static const int PAGE_W = DISPLAY_HEIGHT;   // 448
static const int PAGE_H = DISPLAY_WIDTH;    // 600

static inline Point to_panel(int x, int y) {        // portrait (x, y) -> panel (X, Y)
    return Point(y, PAGE_W - 1 - x);
}
static inline Rect prect(int x, int y, int w, int h) {   // portrait rectangle -> panel rectangle
    return Rect(y, PAGE_W - x - w, h, w);
}
```

Portrait x grows to the right of the page and ends up growing *upwards* on the panel (panel Y decreases), portrait y grows downwards and becomes panel X. A rectangle's top-left corner in portrait is therefore not its top-left on the panel, hence `PAGE_W - x - w` in `prect()`. Nothing else in the file knows the panel is rotated; the panel itself is then flipped 180 degrees in hardware by the UC8159 driver (`DISPLAY_ROTATION`), which puts the HAT buttons at the top.

### Text first

The text comes first because its height decides the square left for the illustration. The characters are normalised to what the bitmap font can draw (it knows ASCII and the Latin-1 accented letters, UTF-8 lead bytes `0xC2` and `0xC3`): typographic apostrophes and quotes, dashes, the ellipsis, the no-break space and `œ` are mapped, anything else outside that range becomes `?` with its whole UTF-8 sequence skipped so that no stray continuation byte is drawn:

```cpp
unsigned char c = (unsigned char)*p;
if (c >= 0x80 && c != 0xC2 && c != 0xC3 && !(c >= 0x80 && c <= 0xBF)) {
    int n = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : 2;   // lead byte of a sequence the font cannot show
    out += '?';
    p += n;
    continue;
}
```

Then a greedy word wrap with the real glyph widths (`measure_text()` of PicoGraphics), a line break in the text forcing a new line:

```cpp
if (!word.empty()) {
    std::string candidate = line.empty() ? word : line + " " + word;
    if (!line.empty() && g.measure_text(candidate, TEXT_SCALE) > width) {
        lines.push_back(line);
        line = word;
    } else {
        line = candidate;
    }
    word.clear();
}
if (c == '\n') { lines.push_back(line); line.clear(); }
```

A word wider than the page goes on its own line and is clipped by the panel edge, never split. The layout then follows:

```cpp
render_layout_t render_text(const char *text) {
    PicoGraphics &g = display_graphics();
    g.set_font("bitmap8");
    std::vector<std::string> lines = wrap(g, text, PAGE_W - 2 * TEXT_X);
    int text_height = ACCENT_ROOM + (int)lines.size() * LINE_H + TEXT_GAP;
    render_layout_t layout;
    layout.lines = (int)lines.size();
    layout.image_size = (PAGE_H - text_height >= PAGE_W) ? PAGE_W : PAGE_H - text_height;
    layout.image_offset = (PAGE_W - layout.image_size) / 2;
    display_clear(DISPLAY_WHITE);
    g.set_pen(DISPLAY_BLACK);
    int y = layout.image_size + TEXT_GAP + ACCENT_ROOM;   // portrait y of the first line
    for (const std::string &line : lines) {
        g.text(line, to_panel(TEXT_X, y), 100000, TEXT_SCALE, 270.0f);
        y += LINE_H;
    }
    return layout;
}
```

`LINE_H` is `(8 + 1) * 2`, the pitch PicoGraphics uses for `bitmap8` at scale 2, 18 px per line; `ACCENT_ROOM` leaves 8 px above the first line because the bitmap fonts draw accents above the nominal box. The square is the full width when the text is short enough, else what is left (a long text eats into the illustration, never the reverse), centred horizontally. The `100000` is the wrap width given to PicoGraphics, deliberately infinite: wrapping is done here, with the same measurer, so that the layout knows the line count before drawing. The angle `270.0f` makes PicoGraphics rotate the glyphs themselves, advancing towards the top of the panel: no intermediate buffer, no second pass.

### The illustration, strip by strip

The illustration is the expensive part. A 448 by 448 RGB image is 600 KB, more than the RP2350 has, and the dithering must see the whole image in scan order. The JPEG decoder (`jpegdec`) solves the first half: it decodes one row of MCUs at a time (blocks of 8 or 16 lines, left to right) and hands each block to a callback, so the image never exists in full. The callback resamples the block into the square:

```cpp
static int jpeg_draw(JPEGDRAW *d) {
    jpeg_ctx_t *ctx = (jpeg_ctx_t *)d->pUser;
    const int S = ctx->layout->image_size;
    const int W = ctx->src_w, H = ctx->src_h;
    // Target pixels covered by this block: [x*S/W, (x+w)*S/W) and the same for y
    int tx0 = d->x * S / W, tx1 = (d->x + d->iWidthUsed) * S / W;
    int ty0 = d->y * S / H, ty1 = (d->y + d->iHeight) * S / H;
    ...
    if (ctx->strip_ty != ty0) {
        flush_strip(ctx);                     // a new row of MCUs begins
        ctx->strip_ty = ty0;
        ctx->strip_rows = ty1 - ty0;
        memset(ctx->strip, 255, (size_t)ctx->strip_rows * S * 3);
    }
    for (int ty = ty0; ty < ty1; ty++) {
        int sy = ty * H / S - d->y;           // source row inside the block
        const uint16_t *row = d->pPixels + sy * d->iWidth;
        uint8_t *out = ctx->strip + ((ty - ctx->strip_ty) * S + tx0) * 3;
        for (int tx = tx0; tx < tx1; tx++) {
            int sx = tx * W / S - d->x;
            uint16_t p = row[sx];             // RGB565
            *out++ = (uint8_t)(((p >> 11) & 0x1F) * 255 / 31);
            *out++ = (uint8_t)(((p >> 5) & 0x3F) * 255 / 63);
            *out++ = (uint8_t)((p & 0x1F) * 255 / 31);
        }
    }
    return 1;
}
```

Each block maps to a rectangle of target pixels by the ratio of the sizes (nearest-neighbour sampling: a target pixel takes the source pixel at the same relative position; no interpolation, the dithering hides it), and the rows of blocks become strips of at most 24 target rows (`MAX_STRIP_ROWS`, 16 source rows upscaled a little at most) in a 32 KB buffer. A strip is complete when a block arrives with a new `ty0`. The square being 448 at most and the JPEGs 448 or 1024, the decoder is also asked for its built-in reductions when the image is much bigger than the square, which cuts the decoding time by the same factor:

```cpp
int options = 0, div = 1;
int longest = w > h ? w : h;
if (longest / 8 >= layout.image_size)      { options = JPEG_SCALE_EIGHTH;  div = 8; }
else if (longest / 4 >= layout.image_size) { options = JPEG_SCALE_QUARTER; div = 4; }
else if (longest / 2 >= layout.image_size) { options = JPEG_SCALE_HALF;    div = 2; }
```

A 1024 px image from OpenAI into a 448 square is decoded at half size (512), then sampled down to 448; a 448 page from the tool is decoded as is. The decoder is told `RGB565_LITTLE_ENDIAN`, hence the unpacking above.

### Floyd-Steinberg across strips

The second half, the dithering, is where the Pi version's look comes from. Each pixel is matched to the nearest of the 7 panel colours and the error (the difference between the wanted colour and the chosen one) is pushed onto the neighbours not yet drawn, 7/16 to the right, 3/16 below left, 5/16 below, 1/16 below right. Two error rows are enough: the current row receives errors from its left neighbour, the next row collects what the current one pushes down. The subtlety is the strip boundary: the error of the last row of a strip must land on the first row of the next strip, so the two error rows live in the context, not in the strip, and survive the flush:

```cpp
static void flush_strip(jpeg_ctx_t *ctx) {
    if (ctx->strip_ty < 0) return;
    PicoGraphics &g = display_graphics();
    const int S = ctx->layout->image_size;
    for (int row = 0; row < ctx->strip_rows; row++) {
        const uint8_t *px = ctx->strip + row * S * 3;
        int16_t *cur = ctx->err_cur, *next = ctx->err_next;
        memset(next, 0, (size_t)(S + 2) * 3 * sizeof(int16_t));
        int ty = ctx->strip_ty + row;
        for (int x = 0; x < S; x++) {
            int c[3];
            int idx;
            {
                int r = px[x * 3 + 0] + cur[(x + 1) * 3 + 0];   // the pixel plus the error diffused onto it
                int gr = px[x * 3 + 1] + cur[(x + 1) * 3 + 1];
                int b = px[x * 3 + 2] + cur[(x + 1) * 3 + 2];
                idx = nearest_colour(r, gr, b);
                c[0] = r - PALETTE[idx][0];
                c[1] = gr - PALETTE[idx][1];
                c[2] = b - PALETTE[idx][2];
            }
            g.set_pen((uint)idx);
            g.set_pixel(to_panel(ctx->layout->image_offset + x, ty));
            for (int k = 0; k < 3; k++) {
                int e = c[k];
                cur[(x + 2) * 3 + k]  += (int16_t)((e * 7) / 16);
                next[(x + 0) * 3 + k] += (int16_t)((e * 3) / 16);
                next[(x + 1) * 3 + k] += (int16_t)((e * 5) / 16);
                next[(x + 2) * 3 + k] += (int16_t)(e / 16);
            }
        }
        int16_t *t = ctx->err_cur; ctx->err_cur = ctx->err_next; ctx->err_next = t;
    }
    ctx->strip_ty = -1;
    ctx->strip_rows = 0;
}
```

The error rows are `S + 2` wide with the pixel `x` at index `x + 1`, so that the pushes to `x - 1` and `x + 1` never leave the array at the edges; the values are not clamped on purpose (the sum can go a little outside 0..255 and `nearest_colour()` copes), and `nearest_colour()` is a plain squared distance in RGB over seven entries, which is cheap enough for 200 000 pixels. `render_jpeg()` calls `flush_strip()` once more after `decode()` returns, for the last strip, which no further block closes.

`PALETTE` is not the panel's nominal colours but the set the inky library uses for its own dithering, its saturated and desaturated palettes blended at 0.5, the default of the Pi version and of the tool. Dithering against the real, duller colours of the panel would make the pages look washed out; this makes a page on the Pico look like the same page on the Pi. PicoGraphics' own ordered dithering was tried first and gave a woven look, which is why the frame buffer is only used as a 3-bit pixel store here (`set_pen` with the palette index, `set_pixel`).

### Counter, badge, gauge

The three overlays are drawn after the illustration, each on a white box with a 5 px margin so that they read over any picture. The page counter sits at portrait (20, 20) as in the Python, prefixed by the story number when there is a dial:

```cpp
void render_page_counter(int story_no, int page, int total) {
    char counter[24];
    if (story_no > 0) snprintf(counter, sizeof counter, "%d: %d/%d", story_no, page, total);   // dial number first
    else snprintf(counter, sizeof counter, "%d/%d", page, total);
    const int x = 20, y = 20, h = 16;
    int w = g.measure_text(counter, TEXT_SCALE);
    g.set_pen(DISPLAY_WHITE);
    g.rectangle(Rect(y - 5, PAGE_W - 1 - (x + w + 5), h + 10, w + 10));
    g.set_pen(DISPLAY_BLACK);
    g.text(counter, to_panel(x, y), 100000, TEXT_SCALE, 270.0f);
}
```

The AI pages draw a blue robot head at the same place instead (`render_mode_badge()`, eleven rectangles), so that the mode is readable on the page itself while the board sleeps. The battery symbol goes top-right, its mirror; the value it draws is set by `main.cpp` before the action (section 11) and `-1` draws nothing, which is the state before the first reading and in a build without the gauge.

### Under it all

`display.cpp` is thirty lines: a `PicoGraphics_Pen3Bit` frame buffer (3 bits per pixel, 100 KB, allocated once at init) and the Pimoroni `UC8159` driver, created with the pins, the rotation and the BUSY and RESET lines. `display_update()` is the only call that touches the panel. It is blocking (`set_blocking(true)`): it converts the buffer to the panel's format on the fly, sends it, and waits for BUSY, 25 to 35 s. Everything that must keep moving meanwhile, the LED, runs on timers.

## 7. LEDs on timers

### Breathing while blocked

The status LED must breathe while `display_update()` or an HTTPS request blocks the main loop, and there is no second thread. The ramp therefore runs on a hardware repeating timer, the LED on a PWM slice: 50 ms ticks, 20 steps up (`PWM_TOP / 20`), a 0.75 s hold, 20 steps down, a hold, the `fade_leds()` of the Python:

```cpp
#define PWM_TOP 1000
static repeating_timer_t breathe_timer;
static volatile bool breathing = false;
static int level = 0, step = 0, hold_ticks = 0;

static bool breathe_tick(repeating_timer_t *t) {
    if (!breathing) { set_level(0); return false; }   // false: the timer stops itself
    if (hold_ticks > 0) { hold_ticks--; return true; }
    level += step;
    if (level >= PWM_TOP) { level = PWM_TOP; step = -step; hold_ticks = 15; }
    if (level <= 0)       { level = 0;       step = -step; hold_ticks = 15; }
    set_level(level);
    return true;
}
void led_breathe_start(void) {
    if (breathing) return;
    breathing = true;
    level = 0; step = PWM_TOP / 20; hold_ticks = 0;
    add_repeating_timer_ms(50, breathe_tick, NULL, &breathe_timer);
}
void led_breathe_stop(void) {
    if (!breathing) return;
    breathing = false;
    cancel_repeating_timer(&breathe_timer);
    set_level(0);
}
```

`led_on()` and `led_off()` both call `led_breathe_stop()` first, so there is never a timer fighting a steady level, and `breathing` is `volatile` because the timer callback runs in interrupt context. The PWM clock divider of 4 gives about 37 kHz at 150 MHz, invisible flicker. The error pattern (`led_rapid_blink()`, 19 blinks of 100 ms) is blocking on purpose: it is the one moment the firmware wants the reader to wait, and it stops the breathing first.

### The LED that is not a GPIO

The Pico 2 W's own LED sits on the wireless chip. It has no PWM, and it may only be driven from the cyw43 driver's context, not from an arbitrary timer interrupt. The same file compiles a second implementation when `HADISTORY_LED_CYW43` is set: a 1 Hz blink as a timed worker of the cyw43 async context, which is the SDK's way to run something "in the driver's thread" without a thread:

```cpp
static async_at_time_worker_t blink_worker;

static void blink_work(async_context_t *context, async_at_time_worker_t *worker) {
    if (!breathing) { set_led(false); return; }
    blink_state = !blink_state;
    set_led(blink_state);
    async_context_add_at_time_worker_in_ms(context, worker, 500);   // re-arms itself
}
void led_breathe_start(void) {
    if (breathing) return;
    breathing = true;
    blink_state = false;
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &blink_worker, 0);
}
```

That is the whole reason `main()` initialises the cyw43 driver for a build with `STATUS_LED=ONBOARD` and no AI mode, and why `power_sleep()` must deinitialise it before the power-down: its periodic timers would keep waking the cores.

### The mode LED

A plain GPIO: on in the AI mode, off in the story mode, and a 2 Hz blink on its own repeating timer while the WiFi is being joined for a switch (`led_mode_blink_start()`, stopped by `led_mode_set()` whichever way the switch ends). `led_mode_announce()` optionally plays the bursts of the Python at a mode change (3 long for AI, 5 short for story) before settling; they are off by default (`MODE_LED_BURSTS`), the steady state being the whole message. The status LED never says anything about the mode: one question per LED.

## 8. Deep sleep is a reboot

### Why a reboot

The RP2350 power manager can switch the core domain off entirely (state P1.7 in the datasheet's naming), leaving only the always-on domain with its timer and the GPIO wake-up logic. Waking from that state is a reboot of the firmware, there is no "resume". `power.cpp` embraces it: the state is saved before the sleep, the wake-up is a boot with a flag, and the button that woke the board is replayed. Compared with keeping the core in a lighter sleep, the draw of the chip falls to the always-on domain alone, which is what the battery life depends on.

### Arming the wake-up

```cpp
static const uint wake_pins[BUTTON_COUNT] = {
#if HADISTORY_DIAL_KY040
    BUTTON_PIN_EXECUTE, DIAL_PIN_SW, BUTTON_PIN_MODE, BUTTON_PIN_BACK
#else
    BUTTON_PIN_EXECUTE, BUTTON_PIN_RESET, BUTTON_PIN_MODE, BUTTON_PIN_BACK
#endif
};
static_assert(BUTTON_COUNT <= 4, "the RP2350 power manager has four GPIO wake-up slots");
```

Four wake-up slots, one per button role. With the KY-040 the RESET slot watches the dial's push button instead of the HAT's button B, because the ritual starts by pressing the wheel; B then only works awake. The AS5600 has no push button and B keeps its slot. The sleep itself:

```cpp
void power_sleep(void) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (!gpio_get(wake_pins[i])) {
            printf("power: button %d is held down, not sleeping\n", i);
            return;
        }
    }
    powman_timer_set_1khz_tick_source_lposc();     // the crystal stops: the power manager's timer must run from the low power oscillator
    if (!powman_timer_is_running()) powman_timer_start();
    for (int i = 0; i < BUTTON_COUNT; i++) {
        powman_enable_gpio_wakeup((uint)i, wake_pins[i], false, false);   // level low; the internal pull-ups stay active
    }
    powman_power_state off = POWMAN_POWER_STATE_NONE;
    powman_power_state on = POWMAN_POWER_STATE_NONE;
    on = powman_power_state_with_domain_on(on, POWMAN_POWER_DOMAIN_XIP_CACHE);
    on = powman_power_state_with_domain_on(on, POWMAN_POWER_DOMAIN_SWITCHED_CORE);
    if (!powman_configure_wakeup_state(off, on)) { puts("power: invalid wake-up state"); return; }
    int rc = powman_set_power_state(off);
```

A button already held down would fire its slot at once and the request would be ignored, so the function refuses to sleep in that case and the loop retries 20 s later. The wake-up is on a *level* (low), not an edge, with the pull-ups kept: the E9 erratum of the RP2350 pads only concerns pull-downs. The wake state lists the core and the XIP cache; the SRAM banks are not listed, which is the SDK's rule (they must be in the same state as during the sleep), the power manager's own boot sequence bringing them up.

### The timeout that is not one

```cpp
    if (rc == PICO_ERROR_TIMEOUT) {
        // The SDK gives the power manager only a hundred core cycles to acknowledge, but it
        // runs on a slow clock and reports CHANGING first. The request stays valid: wait for
        // the WAITING bit ourselves (a few ms in practice).
        absolute_time_t deadline = make_timeout_time_ms(500);
        while (!(powman_hw->state & POWMAN_STATE_WAITING_BITS) && !time_reached(deadline)) {
            if (powman_hw->state & (POWMAN_STATE_BAD_SW_REQ_BITS | POWMAN_STATE_REQ_IGNORED_BITS)) break;
            tight_loop_contents();
        }
        rc = (powman_hw->state & POWMAN_STATE_WAITING_BITS) ? PICO_OK : rc;
    }
    if (rc != PICO_OK) { printf("power: power-down refused (%d) ...", rc); return; }
```

`powman_set_power_state()` in SDK 2.1 polls the acknowledgement for about a hundred core cycles. The power manager runs on a slow clock and first reports `CHANGING`, so the call returns a timeout while the request is perfectly valid and ends in `WAITING` a few milliseconds later. The code reads the `WAITING` bit itself, with a real deadline, and only gives up on a `BAD_SW_REQ` or `REQ_IGNORED` bit. Without this, the board never slept.

### Going down

```cpp
    stdio_flush();
    sleep_ms(50);
#if LIB_PICO_STDIO_USB
    stdio_usb_deinit();
#endif
#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
    cyw43_arch_deinit();
#endif
    scb_hw->scr |= M33_SCR_SLEEPDEEP_BITS;
    while (true) { __wfi(); }
}
```

`WAITING` means the power manager will cut the domain once *both* cores are in deep sleep: `SLEEPDEEP` set in the system control block, then a wait-for-interrupt. Anything that interrupts the core defeats it, so the two things that keep interrupting are shut first: the USB console (its SOF interrupt every millisecond) and the wireless driver (its periodic timers). The `while (true)` is there because an interrupt that slipped through would return from `__wfi()`; the loop goes right back. The console is flushed and given 50 ms before the deinit so that "power-down accepted" reaches the terminal.

### Coming back

```cpp
bool power_woke_from_sleep(void) {
    return (powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_SWCORE_PD_BITS) != 0;
}
button_t power_wake_button(void) {
    for (int i = 0; i < 4; i++) wake_slots[i] = powman_hw->pwrup[i];   // kept for the diagnostics line
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (powman_hw->pwrup[i] & POWMAN_PWRUP0_STATUS_BITS) return (button_t)i;
    }
    for (int i = 0; i < BUTTON_COUNT; i++) {           // fallback: the button still held down
        gpio_set_input_enabled(wake_pins[i], true);
        if (!gpio_get(wake_pins[i])) return (button_t)i;
    }
    return BUTTON_COUNT;
}
```

`chip_reset` records that the switched core domain was powered down, which tells a wake-up from a cold boot or a RESET press. The slot that fired keeps its status bit until the pads are reconfigured, so `main()` reads it first thing; if no bit is set (it happened), the pads keep their pull-ups through the sleep and the reader is still holding the button, hence the fallback. The button then goes through `buttons_inject_press()` and the loop. `power_print_wake_slots()` prints the four registers on the console at each wake-up, bit 9 being "fired": a diagnostic, kept in.

## 9. Two dials behind one interface

`dial.h` declares one interface and two files implement it; `main.cpp` and `story.cpp` do not know which one is built:

```c
void dial_init(int position, int detents, int zero);
int  dial_position(void);         // 1 .. DIAL_POSITIONS
int  dial_detents(void);          // KY-040: detents from the lower stop (saved); AS5600: 0
int  dial_zero(void);             // AS5600: raw angle of the "1" (saved); KY-040: -1
void dial_set_position(int p);    // RESET. KY-040: re-arm, the wheel is at that position; AS5600: only clears moved
bool dial_calibrate(void);        // AS5600: the 1 is under the arrow now; KY-040: re-arm at 1
void dial_sleep(void);            // before the deep sleep: AS5600 powered off
bool dial_moved(void);            // the wheel moved since the last dial_set_moved(false) / dial_set_position()
void dial_set_moved(bool moved);
int  dial_take_steps(void);       // net steps counted since the last call (0 = nothing moved)
```

Both drivers keep the same four pieces of state: `position` (1 to 12), `pending` (position changes since the loop last asked, for the LED blip), `moved` (for the story pick), and their own notion of where the wheel is. The main loop calls `dial_take_steps()` every turn; the encoder answers from what its interrupts counted, the sensor polls its angle inside that call.

### KY-040: counting detents against the stops

An incremental encoder gives two square waves, CLK and DT, a quarter cycle apart; the direction is which one leads. `dial.cpp` decodes the full quadrature by interrupt on both edges of both lines, with the classic 16-entry table indexed by the previous and the current pair of levels:

```cpp
static const int8_t transitions[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
};
static inline uint8_t lines(void) { return (uint8_t)((gpio_get(DIAL_PIN_CLK) << 1) | gpio_get(DIAL_PIN_DT)); }

static void on_edge(uint gpio, uint32_t events) {
    uint8_t current = lines();
    int8_t d = transitions[(previous << 2) | current];
    previous = current;
    if (d) quarter += d;
    if (current != 3 && current != 0) return;   // between two detents
    int step = quarter >= 1 ? 1 : quarter <= -1 ? -1 : 0;
    quarter = 0;
    if (!step) return;
#if DIAL_REVERSE
    step = -step;
#endif
    int next = clamp(detents + step);
    if (next == detents) return;     // past a stop nothing is counted: no blip either
    detents = next;
    int p = 1 + detents / DIAL_DETENTS_PER_STEP;
    if (p != position) {
        pending += p - position;
        position = p;
        moved = true;
    }
}
```

Three ideas are stacked here. The table turns any pair of consecutive readings into +1, -1 or 0, and the 0 entries are the impossible transitions (both lines changing at once, or no change), which is where contact bounce goes: a bounce is a transition and its reverse, +1 then -1, and sums to nothing. The lines rest at a detent either both high (3) or both low (0); on the KY-040 at hand there is half a cycle per detent so the two rests alternate, and a detent is counted when a rest is reached having moved at least a quarter in one direction since the last one (`quarter` is reset at every rest, so a wobble between two rests cannot accumulate). And the count is of *detents* from the lower stop, clamped between 0 and `DETENTS_MAX`, the position being the count divided by `DIAL_DETENTS_PER_STEP` (2 here: the wheel is numbered every 24 degrees, two detents of 12), rounded down.

The stops are what make this incremental encoder absolute. `clamp()` saturates at the two printed stops, so a wheel turned against a stop never counts past it, and the division rounded down means the numbers sit on the even detents: an odd detent between two numbers keeps the number below, and one stray detent at the stop never shifts the numbers. The earlier firmware counted a step at only one of the two rests, chosen at RESET from the lines read at that moment; when that read fell between two rests, or on a glitch, the other rest was chosen and every number sat a detent off the arrow, with the 12 out of reach. Counting every detent and dividing removed the choice.

RESET re-arms the count, and has to deal with the wheel pressed against the lower stop a few degrees *past* its detent, which it is at that moment (the reader is pushing it down while pressing):

```cpp
void dial_set_position(int p) {
    uint32_t ints = save_and_disable_interrupts();
    position = p;
    detents = (p - 1) * DIAL_DETENTS_PER_STEP;
    pending = 0;
    moved = false;
    previous = lines();
    // Pressed against the lower stop the wheel may sit a few degrees past its detent, the lines
    // between two rests: released, it springs back up to the detent, one quarter, which must not
    // count. At a rest nothing is pending.
    quarter = (previous == 0 || previous == 3) ? 0 : (DIAL_REVERSE ? 1 : -1);
    restore_interrupts(ints);
}
```

If the lines are between two rests at RESET, the spring-back to the detent will be one quarter step in the "up" direction and reach a rest, which would count as a detent; pre-loading `quarter` with the opposite quarter makes that spring-back sum to zero. The interrupts are disabled around the update because `on_edge()` touches the same variables. `dial_init()` restores the saved detent count when it is consistent with the saved position, else rebuilds it from the position (an older record, or a record written while the counts disagreed), so that a position between two numbers survives a sleep.

Nothing counts during the deep sleep, by choice: keeping the core awake to count would cost the battery, and the stops give a cheap way back. The wake-up slot is the push button of the dial, so the ritual (wheel to the stop, press the wheel, turn, A) is also what wakes the board.

### AS5600: reading an angle

The sensor gives the absolute angle of a magnet over it, 12 bits a turn, over I2C, so there is nothing to count and nothing to lose during the sleep. The driver reads two registers:

```cpp
#define REG_STATUS      0x0B     // MD 0x20 magnet detected, ML 0x10 too weak, MH 0x08 too strong
#define REG_RAW_ANGLE   0x0C     // 12 bits, high byte first

static int read_raw(void) {   // 0..4095, or -1 when the sensor does not answer or has no magnet
    uint8_t st, b[2];
    if (!reg_read(REG_STATUS, &st, 1) || !reg_read(REG_RAW_ANGLE, b, 2)) { present = false; return -1; }
    present = true;
    magnet = (st & STATUS_MD) != 0;
    if (!magnet) return -1;
    return ((b[0] & 0x0F) << 8) | b[1];
}
```

The I2C calls have a 5 ms timeout and `read_raw()` gives up at the first failed transfer, so an unplugged sensor costs a few milliseconds per poll, not a hang. The angle of the "1" (the zero) depends on how the magnet sits in the axle, so it is learnt once by the calibration (`dial_calibrate()`: `zero = raw; position = 1;`, then `main.cpp` saves) and the firmware works in angles from the 1, in tenths of a degree:

```cpp
#define STEP_DD   (DIAL_STEP_DEG * 10)                       // a step, 24 degrees, in tenths
#define TRAVEL_DD ((DIAL_POSITIONS - 1) * STEP_DD)          // 264 degrees of numbers

static int angle_dd(int raw) {
    int d = (raw - zero) & 4095;
#if DIAL_REVERSE
    d = (4096 - d) & 4095;
#endif
    int a = d * 3600 / 4096;
    if (a > TRAVEL_DD + (3600 - TRAVEL_DD) / 2) a -= 3600;
    return a;
}
static int nearest(int a) {
    if (a <= 0) return 1;
    int p = 1 + (a + STEP_DD / 2) / STEP_DD;
    return p > DIAL_POSITIONS ? DIAL_POSITIONS : p;
}
```

`& 4095` is the modulo of the 12-bit circle. The wheel's travel is 264 degrees of numbers; the 96 degrees the stops leave out are split in two: the half next to the 1 counts as "below 1" (negative angle, position 1), the other half as "above 12" (position 12). The printed stops keep the wheel inside the 264 degrees anyway; this is for a wheel pressed on its D a little off, or a calibration done a few degrees off.

The position follows the wheel with hysteresis, so that a wheel left exactly between two numbers does not flicker between them at every poll:

```cpp
static void poll(void) {
    int raw = read_raw();
    if (raw < 0 || zero < 0) return;
    int a = angle_dd(raw);
    if (abs(a - (position - 1) * STEP_DD) <= STEP_DD / 2 + DIAL_HYST_DEG * 10) return;   // still on this number
    int p = nearest(a);
    if (p != position) { pending += p - position; position = p; moved = true; }
}
int dial_take_steps(void) {
    if (time_reached(next_poll)) { next_poll = make_timeout_time_ms(DIAL_POLL_MS); poll(); }
    int steps = pending; pending = 0; return steps;
}
```

The current number is kept as long as the angle is within half a step *plus* 3 degrees of it (`DIAL_HYST_DEG`); only beyond that is the nearest number taken. So the switch from 3 to 4 happens at 3 degrees past the middle going up, and the switch back at 3 degrees past the middle going down: a 6 degree band where nothing changes. `poll()` runs every 30 ms from the main loop's call, not from a timer, so there is no concurrency with the loop and no `volatile`.

Three more things. `dial_init()` sets the sensor's power mode (`DIAL_AS5600_PM` 2: 1.8 mA, a reading every 20 ms, which is faster than the poll), prints whether the magnet is detected and the AGC gain (a gauge of the air gap: 128 is the top of the range at 3.3 V, so a reading there means "too weak" even when the angle works), and then reads the position *from the sensor*, whatever was saved; `main.cpp` compares the two to know whether the wheel turned during the sleep. `dial_set_position()` ignores its argument: RESET cannot move a wheel that is read, it only clears `moved`. And `dial_sleep()` tears the bus down before the power-down:

```cpp
void dial_sleep(void) {
    i2c_deinit(DIAL_I2C);
    const uint lines[2] = { DIAL_PIN_SDA, DIAL_PIN_SCL };
    for (int i = 0; i < 2; i++) {
        gpio_set_function(lines[i], GPIO_FUNC_SIO);
        gpio_set_dir(lines[i], GPIO_IN);
        gpio_disable_pulls(lines[i]);
    }
    gpio_put(DIAL_PIN_VCC, false);
}
```

The pads keep their state while the core is off, so a pull-up left on SDA would feed the unpowered sensor through its protection diode, and the VCC pin left high would feed it outright. Both go, and the sensor costs nothing asleep. The AS5600 calibration in `main.cpp` is a 3 s hold on B: `buttons_pressed()` arms a timestamp, `buttons_down()` keeps it alive, and the hold fires once (`reset_hold_done`), after the short press has already done its RESET, which is harmless since RESET only clears `moved`.

## 10. AI mode: WiFi, HTTPS, OpenAI

### WiFi: a state machine the loop polls

`wifi.cpp` never blocks. `wifi_connect()` starts a scan and `wifi_poll()`, called every loop turn, moves a state machine along: `OFF`, `SCANNING`, `CONNECTING`, `UP`, `FAILED`. The candidates are the networks built in from `secrets.cmake`, in their order, then the ones stored in flash at run time (meant for a set-up portal that does not exist yet; a stored network with the same name as a built-in one replaces its password):

```cpp
static void build_candidates(void) {
    cand_count = 0;
    for (int i = 0; i < WIFI_NETWORKS_COUNT; i++) add_candidate(built_in[i].ssid, built_in[i].password);
    for (uint32_t i = 0; i < store.count; i++) add_candidate(store.net[i].ssid, store.net[i].password);
}
```

The scan marks the candidates in range (the driver calls `scan_result()` once per access point heard, from its own context, hence the `volatile` counters) and keeps a list of every name heard with its best signal, for the console. When the scan ends, or its 8 s deadline passes, the join order is built: the candidates seen, in list order, and the unseen ones only when nothing was seen (a hidden name, or a scan that failed):

```cpp
static void scan_done(void) {
    order_count = 0;
    int seen = 0;
    for (int i = 0; i < cand_count; i++) if (cand[i].seen) { order[order_count++] = i; seen++; }
    if (seen == 0) for (int i = 0; i < cand_count; i++) order[order_count++] = i;
    ...
    current = -1;
    join_next();
}
```

Joining is `cyw43_arch_wifi_connect_async()` on the current candidate, with a 15 s deadline when the scan saw it and 30 s when it did not, and the state machine watches the link:

```cpp
case WIFI_CONNECTING: {
    int st = link_status();
    if (st == CYW43_LINK_UP) {
        state = WIFI_UP;
        printf("WiFi: connected to %s\n", current_ssid);
    } else if (st < 0 || time_reached(deadline)) {
        printf("WiFi: %s failed (%s)\n", current_ssid,
               st == CYW43_LINK_NONET ? "network not found" :
               st == CYW43_LINK_BADAUTH ? "bad password" :
               st == CYW43_LINK_FAIL ? "join failed" : "timeout");
        if (st < 0 && st != CYW43_LINK_BADAUTH && tries < WIFI_JOIN_TRIES && join_current()) break;
        join_next();
    }
    break;
}
case WIFI_UP:
    if (link_status() != CYW43_LINK_UP) { state = WIFI_OFF; puts("WiFi: link lost"); }
    break;
```

`CYW43_LINK_UP` means an IP address was obtained, not just an association; the negative codes are the driver's failures. The retry line exists because the chip answers "not found" now and then to a join started right after a scan, and the next try passes: a quick failure other than a bad password gets a second try (`WIFI_JOIN_TRIES`) before the next candidate. A bad password is final for that network. When every candidate failed the state is `FAILED`, which the loop turns into the refused switch; in the AI mode the loop calls `wifi_connect()` again at the next turn, forever.

`link_status()` wraps the driver call in `cyw43_arch_lwip_begin()` / `end()`: in the `threadsafe_background` mode of the SDK the lwIP stack runs from an interrupt-driven worker, and any call into it from the main loop must take that lock.

### The flash store of networks

`wifi_store_add()` keeps up to four networks in the sector before the settings one, newest last: a known name is moved to the end with its new password, a full store drops the oldest. The record is loaded at boot with its string terminators forced, because a flash record is never trusted to be well formed:

```cpp
static void store_load(void) {
    const store_t *f = (const store_t *)(XIP_BASE + STORE_OFFSET);
    if (f->magic == STORE_MAGIC && f->count <= WIFI_STORE_MAX) {
        memcpy(&store, f, sizeof store);
        for (uint32_t i = 0; i < store.count; i++) {   // never trust the terminators of a flash record
            store.net[i].ssid[WIFI_SSID_MAX] = 0;
            store.net[i].password[WIFI_PASSWORD_MAX] = 0;
        }
    } else { memset(&store, 0, sizeof store); store.magic = STORE_MAGIC; }
}
```

Nothing in the firmware calls `wifi_store_add()` yet: the API is there for the captive-portal set-up of the roadmap.

### HTTPS: blocking on top of callbacks

`https.cpp` is one function, `https_post_json()`, blocking from the caller's point of view and event driven underneath. The request is built as one string, headers and JSON body, `Connection: close` so that the end of the response is unambiguous:

```cpp
c->request_len = snprintf(c->request, c->request_len,
    "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Authorization: Bearer %s\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: %u\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s",
    path, host, bearer_token, (unsigned)body_len, json_body);
```

The connection is an `altcp` pcb, lwIP's abstraction that puts TLS (mbedtls) under the TCP API, and the five callbacks of a TCP client are registered on it:

```cpp
cyw43_arch_lwip_begin();
c->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
altcp_arg(c->pcb, c);
int poll_ticks = idle_timeout_s * 2;                 // the poll interval counts in 500 ms units on 8 bits: 127 s at most
if (poll_ticks > 255) poll_ticks = 255;
altcp_poll(c->pcb, on_poll, (u8_t)poll_ticks);
altcp_recv(c->pcb, on_recv);
altcp_sent(c->pcb, on_sent);
altcp_err(c->pcb, on_err);
mbedtls_ssl_set_hostname((mbedtls_ssl_context *)altcp_tls_context(c->pcb), host);
err_t err = dns_gethostbyname(host, &ip, on_dns, c);
if (err == ERR_OK) connect_ip(&ip, c);                // cached: connect now
else if (err != ERR_INPROGRESS) { ... c->error = -1; client_close(c); }
cyw43_arch_lwip_end();

while (!c->complete) { sleep_ms(10); }               // the work happens in the lwIP background context
```

Everything after the lock is released happens in lwIP's context: `on_dns` connects, `on_connected` starts sending, `on_sent` sends the rest of the request as the send buffer frees up (`send_more()` writes as much as `altcp_sndbuf()` allows, with `TCP_WRITE_FLAG_MORE` until the last piece), `on_recv` parses, `on_poll` fires when nothing has been received for the idle timeout and aborts, `on_err` catches a connection reset (and must not close the pcb, which lwIP has already freed). The main loop just sleeps on the `complete` flag, `volatile` because it is set from the other context. `mbedtls_ssl_set_hostname()` is what makes the certificate check verify the name, and SNI work; without it the server would answer with a default certificate and the verification would fail.

`tls_config` is created once and kept: it holds the two root certificates of `openai_roots.h` parsed by mbedtls, and `ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED` from CMake makes a chain that does not reach them a fatal error. `TLS_NO_VERIFY=ON` builds the config with no roots, for debugging only.

### Parsing a response on the fly

The body never lands in one buffer. `on_recv()` walks the pbuf chain of each packet and feeds every piece to a small parser:

```cpp
static void feed(client_t *c, const uint8_t *data, size_t len) {
    if (c->headers_done) { feed_body(c, data, len); return; }
    while (len > 0) {
        if (c->hdr_len >= HEADER_MAX - 1) { ... error -4 ... }
        c->hdr[c->hdr_len++] = (char)*data++; len--;
        if (c->hdr_len >= 4 && memcmp(c->hdr + c->hdr_len - 4, "\r\n\r\n", 4) == 0) {
            c->hdr[c->hdr_len] = '\0';
            c->headers_done = true;
            c->status = (int)strtol(c->hdr + 9, NULL, 10);   // "HTTP/1.1 200 ..."
            const char *v;
            if (header_find(c->hdr, "Transfer-Encoding", &v) && strncasecmp(v, "chunked", 7) == 0) c->chunked = true;
            if (header_find(c->hdr, "Content-Length", &v)) c->content_length = strtol(v, NULL, 10);
            if (len > 0) feed_body(c, data, len);
            return;
        }
    }
}
```

The headers are collected byte by byte until the blank line (2 KB at most), the status and two headers are read, and the rest of that same packet is already body. OpenAI answers chunked, so the body goes through a four-state de-chunker, `CH_SIZE` (hex digits up to the newline, extensions and `\r` ignored), `CH_DATA` (deliver up to the chunk's size), `CH_CRLF` (eat the line end), `CH_DONE` on the zero-size chunk, which closes the connection. The pieces delivered to the sink are therefore the chunks' payload, in order, free of the framing, and of arbitrary sizes: a sink must be written to accept a key split across two calls, which both sinks of `gpt.cpp` are. With a `Content-Length` instead, the connection is closed when the count is reached; with neither, when the server closes. A remote close before the headers is an error (`-6`); the error codes are small negatives, `-1` DNS, `-2` connect, `-3` idle timeout, `-4` headers too long, `-5` write, `-7` TLS config, `-8` memory, reported by `gpt.cpp` with the status.

`lwipopts.h` sizes the stack for this: a 16 KB receive window because a TLS record is up to 16 KB and must fit, an 8 segment send buffer, 24 pbufs, and `MEM_SIZE` 8000 for lwIP's own heap. `mbedtls_config.h` is a TLS 1.2 client and nothing else (`MBEDTLS_SSL_CLI_C`, no server, no TLS 1.3): ECDHE with ECDSA or RSA and plain RSA key exchange, AES in GCM and CBC, the NIST curves and Curve25519, SNI on (`MBEDTLS_SSL_SERVER_NAME_INDICATION`, what `mbedtls_ssl_set_hostname()` relies on), hardware entropy from the SDK, outgoing records capped at 2 KB (`MBEDTLS_SSL_OUT_CONTENT_LEN`, the request goes out in pieces anyway), and SHA-384 as its own option because the chain of `api.openai.com` is signed with it and mbedtls 3 no longer bundles it with SHA-512, which is the one line that broke when moving to SDK 2.3.1.

### GPT: two requests, two sinks

`gpt_generate_page()` strings the pipeline together: the prompt, the story text from the chat API, the illustration from the image API, then the same rendering as a story page:

```cpp
bool gpt_generate_page(void) {
    static char prompt[1024];
    if (!wifi_up()) { ... return false; }
    if (!prompt_create(prompt, sizeof prompt)) return false;
    absolute_time_t t0 = get_absolute_time();
    if (!get_story(prompt)) return false;
    print_elapsed("Story generated", t0);
    bool have_image = get_image(story_text);
    render_layout_t layout = render_text(story_text);
    if (have_image) {
        size_t cap;
        uint8_t *buf = render_image_buffer(&cap);
        render_jpeg(layout, buf, last_image_len);   // the download filled the shared buffer
    }
    render_mode_badge();
    render_battery();
    display_update();
    return have_image;
}
```

A failed image is not a failed page: the text is shown alone (the Python did the same), and the return value makes the loop blink the error anyway so that the reader knows. The chat request is the small one; its sink accumulates the answer in an 8 KB buffer and the content is pulled out of the JSON:

```cpp
snprintf(body, sizeof body, "{\"model\":\"%s\",\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}", GPT_MODEL, escaped);
...
if (!json_extract_string(response, acc.len, "content", story_text, sizeof story_text)) { ... }
for (char *p = story_text; *p; p++) if (*p == '\n') *p = ' ';   // one paragraph, as hadistory.py does
```

`json.cpp` is not a JSON parser: `json_escape()` escapes a string for a body, and `json_extract_string()` finds `"key":` followed by a quoted string and copies it with the escapes decoded, `\n`, `\"`, `\uXXXX` turned into UTF-8 (a surrogate pair, an emoji, is recognised and skipped, since the font could not draw it). It finds the *first* `"content":` in the answer, which for the chat completions API is the message; a model whose answer embedded that key in the text would confuse it, and that is the accepted limit of 85 lines instead of a parser.

The image request is the big one. The answer is JSON holding a base64 string of the JPEG, some 100 KB that would need 100 KB of text plus 75 KB of decoded bytes if held twice. The sink instead looks for the key one character at a time, then decodes the base64 as it streams, straight into the 160 KB image buffer that `render_jpeg()` will read:

```cpp
static const char B64_KEY[] = "\"b64_json\"";

static void image_sink(const uint8_t *data, size_t len, void *ctx) {
    image_sink_t *s = (image_sink_t *)ctx;
    ... keep the first 512 bytes in s->head, for the error messages ...
    size_t i = 0;
    while (i < len && !s->done) {
        if (!s->in_data) {
            char c = (char)data[i++];
            if (s->key_found) {
                if (c == '"') s->in_data = true;             // the opening quote of the value
                else if (c != ':' && c != ' ' && c != '\n' && c != '\r' && c != '\t') s->key_found = false;
            } else if (c == B64_KEY[s->matched]) {
                s->matched++;
                if (B64_KEY[s->matched] == '\0') { s->key_found = true; s->matched = 0; }
            } else {
                s->matched = (c == B64_KEY[0]) ? 1 : 0;
            }
        } else {
            size_t start = i;
            while (i < len && data[i] != '"') i++;
            base64_feed(&s->b64, (const char *)data + start, i - start);
            if (i < len) s->done = true;                       // closing quote
        }
    }
}
```

The matcher keeps its position in the key across calls (`matched`), so a key split between two chunks is still found; after the key it skips the colon and any whitespace the API may put before the value; inside the value it hands whole runs to the base64 decoder, which keeps its own partial sextets across calls (`acc`, `acc_bits`) and flags an overflow of the buffer instead of writing past it. After the closing quote the rest of the body is read and dropped. `IMAGE_COMPRESSION` (40) is the JPEG compression asked to the API so that the image fits the buffer; an overflow prints a message to raise it.

### The prompt

`prompt_create()` in `prompt.cpp` is the port of the Python prompt builder over the same `prompts/prompts.txt`, embedded at build time. A line is picked with a weight (`12:text` weighs 12, a bare line 1) by walking the file twice, once to sum the weights and once to stop at the chosen one, so that the file is never copied. The chosen line's groups `{a|2:b|c}`, `[...]`, `(...)` are then resolved in that order, each group replaced by one of its options with the same weight syntax, repeatedly until none is left, the spaces collapsed, and the fixed incipit and excipit put around it:

```cpp
static const char *const BRACKETS[] = { "{}", "[]", "()" };   // reverse of the Python list, as parse_multiple_brackets does
...
for (const char *pair : BRACKETS) resolve_brackets(line, sizeof line, pair[0], pair[1]);
collapse_spaces(line);
snprintf(out, out_size, "%s%s %s", PROMPT_INCIPIT, line, PROMPT_EXCIPIT);
```

`resolve_brackets()` finds the first opening bracket and the first closing one after it, which means groups do not nest within the same bracket kind; nesting across kinds works because the kinds are resolved one after the other, and the order (braces first) is the Python's.

## 11. Battery

`battery.cpp` reads VSYS, the board's supply after the diode or the USB, through the on-board 3:1 divider on ADC channel 3 (`PICO_VSYS_PIN`), eight samples averaged after the first ones are thrown away because the ADC's first conversions after a channel change read low:

```cpp
static int read_vsys_mv(void) {
#if HADISTORY_HAS_CYW43 && CYW43_USES_VSYS_PIN
    cyw43_thread_enter();                            // hold the wireless driver while the pin is borrowed
#endif
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - ADC_BASE_PIN);
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    int ignore = SAMPLES;
    while (!adc_fifo_is_empty() || ignore-- > 0) (void)adc_fifo_get_blocking();
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
#if HADISTORY_HAS_CYW43 && CYW43_USES_VSYS_PIN
    cyw43_thread_exit();
#endif
    uint32_t raw = sum / SAMPLES;                    // 0 .. 4095 for 0 .. 3.3 V
    uint32_t nominal = raw * 3300 * 3 / 4095;        // over the nominal reference and divider
    return (int)((uint64_t)nominal * BATTERY_CAL_METER_MV / BATTERY_CAL_CONSOLE_MV);   // measured correction
}
```

On the Pico W family the VSYS pin is also the clock of the SPI to the wireless chip, which is why the driver's lock is held while the pin is reconfigured as an analogue input: the driver reclaims the pin at its next transfer. The Waveshare board senses VSYS on a pin of its own and `CYW43_USES_VSYS_PIN` is not set for it. The conversion assumes the nominal 3.3 V reference and the nominal divider; both are off by a few percent, so the reading is multiplied by a ratio measured once with a multimeter (`BATTERY_CAL_METER_MV / BATTERY_CAL_CONSOLE_MV`, 4760 / 4656 on the prototype).

```cpp
battery_t battery_read(void) {
    battery_t b;
    int vsys = read_vsys_mv();
    b.on_usb = vsys >= BATTERY_USB_MV;              // 4.3 V: a cell never gives more than 4.0 V after the diode
    if (b.on_usb) { b.bars = BATTERY_BARS; ... return b; }
    b.millivolts = vsys + BATTERY_DIODE_DROP_MV;    // the diode between the cell and VSYS, 580 mV measured
    b.bars = 0;
    for (int i = 0; i < BATTERY_BARS; i++) {
        if (b.millivolts >= BAR_MV[i]) { b.bars = BATTERY_BARS - i; break; }
    }
    return b;
}
```

Above 4.3 V it is USB power (the page shows a lightning bolt); otherwise the diode drop is added back to estimate the cell and the bars come from the thresholds of `BATTERY_BAR_MV` (4.0, 3.8, 3.6, 3.45 V). The reading is taken by `main.cpp` before each action, while the board is idle; `render_set_battery()` stores it and `render_battery()` draws it after the page. The gauge is informative only: nothing in the firmware acts on the level, the protection circuit of the battery module cuts the power when the cell is really empty.

## 12. Memory map and sizes

Where the big things live, for a board with 520 KB of RAM:

| What | Size | Where | Lifetime |
|---|---|---|---|
| Frame buffer (`PicoGraphics_Pen3Bit`) | 100 KB | heap, `display_init()` | the whole run |
| JPEG image buffer (`image_buffer`) | 160 KB | static, `render.cpp` | the whole run; SD card pages and OpenAI images; unused by flash pages |
| Dithering strip + error rows | 32 KB + 5 KB | static, `render.cpp` | the whole run |
| JPEG decoder (`JPEGDEC`) | its pixel and MCU buffers, some KB | static, `render.cpp` | the whole run |
| Chat response, prompt and bodies | 8 KB + 2 KB + a few KB | static, `gpt.cpp` | AI build only |
| lwIP heap and pbufs, mbedtls session | about 8 KB + 24 pbufs + 40 KB | lwIP and mbedtls | AI build, during a request |
| Settings | 256 bytes | last flash sector | across boots |
| WiFi store | one sector | second to last flash sector | across boots |
| Embedded pages | per the quality table in `tools/README.md`, the largest page about 95 KB | flash `.rodata` | the firmware |

Nothing is allocated per page: every buffer is static and reused, so the heap cannot fragment over weeks of pages. The only run-time allocations are the display objects at init and, in the AI mode, the HTTPS client and its request string, both freed before `https_post_json()` returns, plus what lwIP and mbedtls take from their own pools during a request.
