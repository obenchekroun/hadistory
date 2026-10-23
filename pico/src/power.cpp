#include "power.h"
#include "hadistory_config.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
extern "C" {
#include "hardware/powman.h"   // no C++ guards in SDK 2.1.0
}
#include "hardware/structs/powman.h"
#include "hardware/structs/scb.h"
#include "hardware/sync.h"
#if LIB_PICO_STDIO_USB
#include "pico/stdio_usb.h"
#endif
#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
#include "pico/cyw43_arch.h"
#endif

// One wake-up slot per button role. With the story dial, the RESET slot watches the dial's push
// button instead of the HAT button B (four slots only): the ritual starts by pressing the wheel.
static const uint wake_pins[BUTTON_COUNT] = {
#if HADISTORY_STORY_DIAL
    BUTTON_PIN_EXECUTE, DIAL_PIN_SW, BUTTON_PIN_MODE, BUTTON_PIN_BACK
#else
    BUTTON_PIN_EXECUTE, BUTTON_PIN_RESET, BUTTON_PIN_MODE, BUTTON_PIN_BACK
#endif
};
static_assert(BUTTON_COUNT <= 4, "the RP2350 power manager has four GPIO wake-up slots");

bool power_woke_from_sleep(void) {
    return (powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_SWCORE_PD_BITS) != 0;
}

static uint32_t wake_slots[4];   // PWRUP registers as found at boot, for diagnostics

void power_print_wake_slots(void) {
    printf("power: wake-up slots at boot 0x%08lx 0x%08lx 0x%08lx 0x%08lx (bit 9 = fired)\n",
           (unsigned long)wake_slots[0], (unsigned long)wake_slots[1],
           (unsigned long)wake_slots[2], (unsigned long)wake_slots[3]);
}

button_t power_wake_button(void) {
    // Called first thing after a wake-up, before the console is up: the slot that fired
    // keeps its status bit, the registers are saved for power_print_wake_slots()
    for (int i = 0; i < 4; i++) wake_slots[i] = powman_hw->pwrup[i];
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (powman_hw->pwrup[i] & POWMAN_PWRUP0_STATUS_BITS) return (button_t)i;
    }
    // Fallback: the button still held down (pads keep their pull-ups through the sleep)
    for (int i = 0; i < BUTTON_COUNT; i++) {
        gpio_set_input_enabled(wake_pins[i], true);
        if (!gpio_get(wake_pins[i])) return (button_t)i;
    }
    return BUTTON_COUNT;
}

void power_sleep(void) {
    // Diagnostics: button levels (all must read 1), pending power-up requests, current state
    printf("power: buttons A=%d B=%d C=%d D=%d, state=0x%08lx, pwrup_req=0x%08lx, chip_reset=0x%08lx\n",
           gpio_get(wake_pins[0]), gpio_get(wake_pins[1]), gpio_get(wake_pins[2]), gpio_get(wake_pins[3]),
           (unsigned long)powman_hw->state, (unsigned long)powman_hw->current_pwrup_req,
           (unsigned long)powman_hw->chip_reset);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (!gpio_get(wake_pins[i])) {
            printf("power: button %d is held down, not sleeping\n", i);
            return;
        }
    }
    // The power manager timer must run from the low power oscillator, the crystal stops
    powman_timer_set_1khz_tick_source_lposc();
    if (!powman_timer_is_running()) powman_timer_start();

    // One wake-up slot per button: level low, the internal pull-ups stay active (E9 erratum
    // only affects pull-downs). A button already held down blocks the power-down request.
    for (int i = 0; i < BUTTON_COUNT; i++) {
        powman_enable_gpio_wakeup((uint)i, wake_pins[i], false, false);
    }
    printf("power: wake-up slots armed: 0x%08lx 0x%08lx 0x%08lx 0x%08lx (GP%u GP%u GP%u GP%u)\n",
           (unsigned long)powman_hw->pwrup[0], (unsigned long)powman_hw->pwrup[1],
           (unsigned long)powman_hw->pwrup[2], (unsigned long)powman_hw->pwrup[3],
           wake_pins[0], wake_pins[1], wake_pins[2], wake_pins[3]);

    // Sleep state P1.7: everything off. Wake-up state: core and XIP cache on; the SRAM
    // banks must be listed in the same state as during sleep (SDK rule), the power
    // manager's own sequence powers them up before the boot.
    powman_power_state off = POWMAN_POWER_STATE_NONE;
    powman_power_state on = POWMAN_POWER_STATE_NONE;
    on = powman_power_state_with_domain_on(on, POWMAN_POWER_DOMAIN_XIP_CACHE);
    on = powman_power_state_with_domain_on(on, POWMAN_POWER_DOMAIN_SWITCHED_CORE);
    if (!powman_configure_wakeup_state(off, on)) {
        puts("power: invalid wake-up state");
        return;
    }
    int rc = powman_set_power_state(off);
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
    if (rc != PICO_OK) {
        printf("power: power-down refused (%d), state=0x%08lx pwrup_req=0x%08lx\n", rc,
               (unsigned long)powman_hw->state, (unsigned long)powman_hw->current_pwrup_req);
        return;
    }
    // From here the power manager waits for both cores to be in deep sleep
    printf("power: power-down accepted, state=0x%08lx\n", (unsigned long)powman_hw->state);
    stdio_flush();
    sleep_ms(50);

    // Silence everything that keeps interrupting the cores: the USB console and the
    // wireless driver (its periodic timers). Both come back at the reboot that follows.
#if LIB_PICO_STDIO_USB
    stdio_usb_deinit();
#endif
#if HADISTORY_WITH_GPT || HADISTORY_LED_CYW43
    cyw43_arch_deinit();
#endif
    // The power manager only cuts the domain once the processor is in deep sleep:
    // SLEEPDEEP set, then wait-for-interrupt. A button press then reboots the chip.
    scb_hw->scr |= M33_SCR_SLEEPDEEP_BITS;
    while (true) {
        __wfi();
    }
}
