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

    led_init();
    buttons_init();
    display_init();

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
