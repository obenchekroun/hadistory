/********************************************************************************
 * hadistory - Pico version
 * main.cpp - button loop and mode switching, ported from the main loop of pi/hadistory.py
 *
 * Build options (see CMakeLists.txt):
 *   PICO_BOARD          target board, selects the pin mapping in hadistory_config.h
 *   STORAGE             SDCARD (pages on a FAT microSD) or FLASH (pages embedded in the firmware)
 *   WITH_GPT            adds the AI mode (OpenAI over WiFi); off = story mode only
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

static settings_t settings;
static bool sd_ok = false;

static void save_settings(void) {
    if (sd_ok && !storage_save_settings(&settings)) {
        puts("Could not save state");
    }
}

static void announce_mode(void) {
    if (settings.ai_mode) {
        puts("Switching to AI mode.");
        led_three_long_bursts();
    } else {
        puts("Switching to local story mode.");
        led_five_short_bursts();
    }
}

static void wait_for_press(void) {
    puts("\nWaiting for next button press...");
    led_on();
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);   // leave time for the USB console to attach
    puts("Welcome to hadistory !");

#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
    // Wireless chip driver: needed by the AI mode and by the onboard LED of the Pico W family
    if (cyw43_arch_init()) {
        puts("cyw43: init failed");
    }
#endif

    led_init();
    buttons_init();
    display_init();

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

#if HADISTORY_WITH_GPT
    if (!gpt_init()) {
        puts("AI mode unavailable, forcing story mode.");
        settings.ai_mode = false;
    }
#else
    settings.ai_mode = false;   // no AI mode in this build
#endif
    announce_mode();
    wait_for_press();

    while (true) {
        buttons_poll();

        if (buttons_pressed(BUTTON_EXECUTE)) {
            led_breathe_start();
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
            save_settings();
            puts("Story reset. Press button to launch a new one");

        } else if (buttons_pressed(BUTTON_MODE)) {
#if HADISTORY_WITH_GPT
            settings.ai_mode = !settings.ai_mode;
            announce_mode();
            save_settings();
            led_on();
#else
            puts("AI mode not available in this build.");
#endif
        }

        sleep_ms(BUTTON_POLL_MS);
    }
}
