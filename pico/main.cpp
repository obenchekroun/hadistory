/********************************************************************************
 * hadistory - Pico version
 * main.cpp - button loop and mode switching, ported from the main loop of pi/hadistory.py
 *
 * Build options (see CMakeLists.txt):
 *   PICO_BOARD          target board, selects the pin mapping in hadistory_config.h
 *   STORAGE             SDCARD (pages on a FAT microSD) or FLASH (pages embedded in the firmware)
 *   WITH_GPT            adds the AI mode (OpenAI over WiFi); off = story mode only
 *   DEEP_SLEEP          core powered off between pages, any button reboots the board (story mode)
 *   BATTERY_GAUGE       battery bars on each page, display only
 *   STORY_DIAL          KY-040 story dial: RESET re-arms it at 1, GO starts the story under the arrow
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "hadistory_config.h"
#include "led.h"
#include "buttons.h"
#include "display.h"
#include "storage.h"
#include "story.h"
#if HADISTORY_WITH_GPT
#include "gpt.h"
#endif
#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
#include "pico/cyw43_arch.h"
#endif
#if HADISTORY_DEEP_SLEEP
#include "power.h"
#endif
#if HADISTORY_BATTERY
#include "battery.h"
#include "render.h"
#endif
#if HADISTORY_STORY_DIAL
#include "dial.h"
#endif

static settings_t settings;
static bool sd_ok = false;

static void save_settings(void) {
#if HADISTORY_STORY_DIAL
    settings.dial = dial_position();
    settings.dial_moved = dial_moved();
#endif
    if (sd_ok && !storage_save_settings(&settings)) {
        puts("Could not save state");
    }
}

// The mode LED says it: bursts of hadistory.py (3 long = AI, 5 short = story), then on = AI mode
static void announce_mode(void) {
    puts(settings.ai_mode ? "Switching to AI mode." : "Switching to local story mode.");
    led_mode_announce(settings.ai_mode);
}

static void wait_for_press(void) {
    puts("\nWaiting for next button press...");
    led_on();
}

// Battery reading before each page: hands the level to the page composers, which draw
// it in the corner before the panel update. Display only, nothing depends on the level.
static void battery_update(void) {
#if HADISTORY_BATTERY
    battery_t b = battery_read();
    render_set_battery(b.bars, b.on_usb);
#endif
}

int main(void) {
    stdio_init_all();
    sleep_ms(5000);
#if HADISTORY_DEEP_SLEEP
    // Waking from deep sleep is a reboot: no console delay then, the reader is waiting.
    // The button that woke us is captured right away, before any delay, while it is
    // still recorded by the power manager and probably still held down.
    bool woke = power_woke_from_sleep();
    button_t wake_button = woke ? power_wake_button() : BUTTON_COUNT;
#else
    bool woke = false;
#endif
    puts(woke ? "Welcome back to hadistory !" : "Welcome to hadistory !");
#if HADISTORY_DEEP_SLEEP
    if (woke) {
        power_print_wake_slots();
        printf("Woken up by button %d (0 = A, 1 = B, 2 = C, 3 = D, 4 = unknown)\n", (int)wake_button);
    }
#endif

#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
    // Wireless chip driver: needed by the AI mode and by the onboard LED of the Pico W family
    if (cyw43_arch_init()) {
        puts("cyw43: init failed");
    }
#endif

    led_init();
    led_mode_init();
    buttons_init();
    display_init();
#if HADISTORY_BATTERY
    battery_init();
#endif

// ---- DEBUG: buttons + display test. Uncomment to enable. ----
//     // Loop forever: print each button press and the raw BUSY level every second.
//     // Button A draws 7 colour bars and refreshes the display, timing the refresh
//     // (expected 25 to 35 s; a few hundred ms means BUSY never went low).
//     {
//         static const char *names[BUTTON_COUNT] = { "A execute (GP2)", "B reset (GP3)", "C mode (GP4)", "D back (GP5)" };
//         puts("Test: press B/C/D to check them, A to run the display test pattern");
//         for (int tick = 0; ; tick++) {
//             buttons_poll();
//             for (int b = 0; b < BUTTON_COUNT; b++) {
//                 if (!buttons_pressed((button_t)b)) continue;
//                 printf("pressed: %s\n", names[b]);
//                 if (b == BUTTON_EXECUTE) {
//                     pimoroni::PicoGraphics &g = display_graphics();
//                     int bar = DISPLAY_WIDTH / 7;
//                     for (int c = 0; c < 7; c++) {
//                         g.set_pen(c);
//                         g.rectangle(pimoroni::Rect(c * bar, 0, bar, DISPLAY_HEIGHT));
//                     }
//                     puts("Test pattern: refreshing display (LED breathing)...");
//                     // Probe: sample BUSY every ms during the update and count the low samples
//                     static volatile uint32_t busy_samples, busy_low;
//                     busy_samples = busy_low = 0;
//                     repeating_timer_t probe;
//                     add_repeating_timer_ms(1, [](repeating_timer_t *) {
//                         busy_samples++;
//                         if (!gpio_get(DISPLAY_PIN_BUSY)) busy_low++;
//                         return true;
//                     }, NULL, &probe);
//                     led_breathe_start();
//                     absolute_time_t t0 = get_absolute_time();
//                     display_update();
//                     led_breathe_stop();
//                     cancel_repeating_timer(&probe);
//                     printf("Test pattern: done in %lld ms\n", absolute_time_diff_us(t0, get_absolute_time()) / 1000);
//                     printf("BUSY probe: low during %lu of %lu ms (0 = the panel never signalled busy)\n",
//                            (unsigned long)busy_low, (unsigned long)busy_samples);
//                 } else {
//                     led_on(); sleep_ms(100); led_off();
//                 }
//             }
//             if (tick % 50 == 0) {   // every second
//                 printf("BUSY (GP%d) level: %d   buttons raw: A=%d B=%d C=%d D=%d\n",
//                        DISPLAY_PIN_BUSY, gpio_get(DISPLAY_PIN_BUSY),
//                        gpio_get(BUTTON_PIN_EXECUTE), gpio_get(BUTTON_PIN_RESET),
//                        gpio_get(BUTTON_PIN_MODE), gpio_get(BUTTON_PIN_BACK));
//             }
//             sleep_ms(20);
//         }
//     }
// ---- end DEBUG ----

    sd_ok = storage_init();
    if (sd_ok && storage_load_settings(&settings)) {
        puts("Currently on loaded !");
    } else {
        puts("No saved state, starting on local story mode.");
        memset(&settings, 0, sizeof settings);
    }
    story_init(&settings);
#if HADISTORY_STORY_DIAL
    dial_init(settings.dial);
    dial_set_moved(settings.dial_moved);   // a turn before the sleep still counts after the wake-up
    bool dial_dirty = false;    // position moved since the last save
#endif

#if HADISTORY_WITH_GPT
    gpt_init();
    if (settings.ai_mode) gpt_wifi_connect();   // resume in AI mode: join in the background
    bool ai_switch_pending = false;             // button C pressed, waiting for the WiFi
#else
    settings.ai_mode = false;   // no AI mode in this build
#endif
    if (woke) {
#if HADISTORY_DEEP_SLEEP
        if (wake_button < BUTTON_COUNT) buttons_inject_press(wake_button);   // act on the button that woke us
#endif
        led_mode_set(settings.ai_mode);   // no announcement on a wake-up, just the state
    } else {
        announce_mode();
    }
    wait_for_press();

    absolute_time_t idle_since = get_absolute_time();

    while (true) {
        buttons_poll();
        bool any_press = false;
        for (int b = 0; b < BUTTON_COUNT; b++) {
            if (buttons_pressed((button_t)b)) any_press = true;
        }
#if HADISTORY_STORY_DIAL
        // Each counted detent: a blip of the status LED, and the board stays awake a while longer
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

#if HADISTORY_WITH_GPT
        wifi_state_t wifi = gpt_wifi_poll();
        if (ai_switch_pending) {
            // The switch to AI mode completes only once the network is there
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
            gpt_wifi_connect();   // link lost or failed at boot: keep trying while in AI mode
        }
#endif

        if (buttons_pressed(BUTTON_EXECUTE)) {
            led_breathe_start();
            battery_update();
            if (settings.ai_mode) {
#if HADISTORY_WITH_GPT
                story_reset(&settings);
                save_settings();
                if (!gpt_generate_page()) led_rapid_blink();
#endif
            } else {
                if (!story_next_page(&settings)) led_rapid_blink();
                save_settings();
            }
            led_breathe_stop();
            sleep_ms(POST_ACTION_PAUSE_MS);
            wait_for_press();

        } else if (buttons_pressed(BUTTON_BACK) && !settings.ai_mode) {
            if (settings.story[0] && settings.page > 1) {
                led_breathe_start();
                battery_update();
                if (!story_previous_page(&settings)) led_rapid_blink();
                save_settings();
                led_breathe_stop();
                sleep_ms(POST_ACTION_PAUSE_MS);
                wait_for_press();
            } else {
                puts("Nothing to show, skipping ...");
            }

        } else if (buttons_pressed(BUTTON_RESET)) {
            story_reset(&settings);
#if HADISTORY_STORY_DIAL
            dial_set_position(1);   // the ritual: wheel at the lower stop, RESET, turn to the story, GO
            dial_dirty = false;
            save_settings();
            puts("Story reset, dial re-armed at 1: turn the wheel to a story, or press A for one at random");
#else
            save_settings();
            puts("Story reset. Press button to launch a new one");
#endif

        } else if (buttons_pressed(BUTTON_MODE)) {
#if HADISTORY_WITH_GPT
            if (settings.ai_mode) {
                settings.ai_mode = false;
                ai_switch_pending = false;
                announce_mode();
                save_settings();
                led_on();
            } else if (wifi == WIFI_UP) {
                settings.ai_mode = true;
                announce_mode();
                save_settings();
                led_on();
            } else {
                // Not connected yet: join in the background, switch when the link is up
                puts("Connecting to the WiFi before switching to AI mode...");
                ai_switch_pending = true;
                gpt_wifi_connect();   // no-op if already connecting
                led_breathe_start();
                led_mode_blink_start();
            }
#else
            puts("AI mode not available in this build.");
#endif
        }

        // The idle time counts from the end of the action, not from the press
        if (any_press) idle_since = get_absolute_time();

#if HADISTORY_DEEP_SLEEP
        // Story mode only: the WiFi link of the AI mode would not survive the power-down
        if (!settings.ai_mode && absolute_time_diff_us(idle_since, get_absolute_time()) > IDLE_BEFORE_SLEEP_MS * 1000) {
            puts("Going to sleep, press any button to wake up.");
#if HADISTORY_STORY_DIAL
            if (dial_dirty) { save_settings(); dial_dirty = false; }   // keep the wheel position across the sleep
#endif
            led_off();
            power_sleep();            // reboots on a button press, returns only on failure
            led_on();
            idle_since = get_absolute_time();
        }
#endif
        sleep_ms(BUTTON_POLL_MS);
    }
}
