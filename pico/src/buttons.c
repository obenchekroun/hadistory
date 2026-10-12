#include "buttons.h"
#include "hadistory_config.h"

#include "pico/stdlib.h"

static const uint button_gpio[BUTTON_COUNT] = {
    BUTTON_PIN_EXECUTE, BUTTON_PIN_RESET, BUTTON_PIN_MODE, BUTTON_PIN_BACK
};

static bool previous_state[BUTTON_COUNT];   // true = released (pull-up), as in hadistory.py
static bool current_state[BUTTON_COUNT];

void buttons_init(void) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        gpio_init(button_gpio[i]);
        gpio_set_dir(button_gpio[i], GPIO_IN);
        gpio_pull_up(button_gpio[i]);
        previous_state[i] = true;
        current_state[i] = true;
    }
}

void buttons_poll(void) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        previous_state[i] = current_state[i];
        current_state[i] = gpio_get(button_gpio[i]);
    }
}

bool buttons_pressed(button_t b) {
    return previous_state[b] && !current_state[b];
}
