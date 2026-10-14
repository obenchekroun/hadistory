/* Deep sleep between pages (DEEP_SLEEP=ON, HADISTORY_DEEP_SLEEP=1), RP2350 power manager.
 * The switched core, SRAM and XIP are powered off (state P1.7, about 150 uA for the chip);
 * any of the four buttons, held low, powers the chip up again. Waking is a reboot: the
 * firmware restarts, detects it was woken and acts on the button that did it.
 */
#ifndef HADISTORY_POWER_H
#define HADISTORY_POWER_H

#include "buttons.h"

// True when this boot follows a sleep (the switched core had been powered down)
bool power_woke_from_sleep(void);

// Button that triggered the wake-up, from the power manager status, or BUTTON_COUNT.
// To be called first thing at boot, before any delay.
button_t power_wake_button(void);
void power_print_wake_slots(void);   // diagnostics: the wake-up registers seen by power_wake_button()

// Powers the core off until a button is pressed. Returns only on failure.
void power_sleep(void);

#endif
