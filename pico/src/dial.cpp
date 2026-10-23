#include "dial.h"
#include "hadistory_config.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

// Full quadrature decoding on both edges of both lines: 4 transitions per detent on a KY-040,
// the table rejects the invalid ones (bounce). A detent is counted when the lines come back
// to rest (both high) having accumulated at least half a cycle in one direction.
static const int8_t transitions[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
};

static volatile uint8_t previous;    // last (CLK, DT) pair
static volatile int quarter;         // quarter steps since the last rest
static volatile int position = 1;
static volatile int pending;         // detents counted since dial_take_steps()
static volatile bool moved;          // a detent was counted since the last story start / re-arm

static int clamp(int p) {
    if (p < 1) return 1;
    if (p > DIAL_POSITIONS) return DIAL_POSITIONS;
    return p;
}

static void on_edge(uint gpio, uint32_t events) {
    (void)gpio; (void)events;
    uint8_t current = (uint8_t)((gpio_get(DIAL_PIN_CLK) << 1) | gpio_get(DIAL_PIN_DT));
    int8_t d = transitions[(previous << 2) | current];
    previous = current;
    if (d) quarter += d;
    if (current != 3) return;        // not at rest yet
    int step = quarter >= 2 ? 1 : quarter <= -2 ? -1 : 0;
    quarter = 0;
    if (!step) return;
#if DIAL_REVERSE
    step = -step;
#endif
    int next = clamp(position + step);
    if (next != position) {          // past a stop nothing is counted: no blip either
        position = next;
        pending += step;
        moved = true;
    }
}

void dial_init(int saved) {
    position = clamp(saved);
#if DIAL_PIN_VCC >= 0
    gpio_init(DIAL_PIN_VCC);         // the module's "+": its two 10 k pull-ups, 0.3 mA at most
    gpio_set_dir(DIAL_PIN_VCC, GPIO_OUT);
    gpio_put(DIAL_PIN_VCC, true);
#endif
    const uint pins[2] = { DIAL_PIN_CLK, DIAL_PIN_DT };
    for (int i = 0; i < 2; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);       // the module has its own 10 k pull-ups to its + pin as well
    }
    previous = (uint8_t)((gpio_get(DIAL_PIN_CLK) << 1) | gpio_get(DIAL_PIN_DT));
    gpio_set_irq_enabled_with_callback(DIAL_PIN_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, on_edge);
    gpio_set_irq_enabled(DIAL_PIN_DT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    printf("Dial: position %d of %d (GP%d / GP%d)\n", position, DIAL_POSITIONS, DIAL_PIN_CLK, DIAL_PIN_DT);
}

int dial_position(void) { return position; }

void dial_set_position(int p) {
    uint32_t ints = save_and_disable_interrupts();
    position = clamp(p);
    quarter = 0;
    pending = 0;
    moved = false;
    restore_interrupts(ints);
}

bool dial_moved(void) { return moved; }
void dial_set_moved(bool m) { moved = m; }

int dial_take_steps(void) {
    uint32_t ints = save_and_disable_interrupts();
    int steps = pending;
    pending = 0;
    restore_interrupts(ints);
    return steps;
}
