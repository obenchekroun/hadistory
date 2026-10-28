#include "dial.h"
#include "hadistory_config.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

// Full quadrature decoding on both edges of both lines, the table rejects the invalid
// transitions (bounce). Every detent is counted: at a detent the lines rest both high or both low
// (the two rests alternate on an encoder with half a cycle per detent, the KY-040 at hand; an
// encoder with a full cycle per detent only ever rests both high), and a detent is counted when
// the lines reach a rest having moved at least a quarter cycle in one direction since the last
// one. The count runs from the lower stop, where RESET zeroes it, and the position is
// 1 + detents / DIAL_DETENTS_PER_STEP, rounded down: the printed wheel is numbered one step apart
// (wheel_step in the case model) and the numbers sit on the detents that are whole steps from
// the stop, so an odd detent in between keeps the number below it, and a stray detent (the
// wheel held against the stop a few degrees past its detent when RESET is pressed, and springing
// back; a glitch of the lines when the wheel is pushed for RESET) never shifts the numbers.
// The earlier firmware chose, at RESET, which of the two rests to count a step at, from the
// lines read at that moment: wrong once in a while, and then every number sat a detent off.
static const int8_t transitions[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0,
};

#define DETENTS_MAX  (DIAL_DETENTS_PER_STEP * (DIAL_POSITIONS - 1) + DIAL_DETENTS_PER_STEP - 1)

static volatile uint8_t previous;    // last (CLK, DT) pair
static volatile int quarter;         // quarter steps since the last rest
static volatile int detents;         // detents from the lower stop, 0 .. DETENTS_MAX
static volatile int position = 1;    // 1 + detents / DIAL_DETENTS_PER_STEP
static volatile int pending;         // steps (position changes) since dial_take_steps()
static volatile bool moved;          // the position changed since the last story start / re-arm

static int clamp(int d) {
    if (d < 0) return 0;
    if (d > DETENTS_MAX) return DETENTS_MAX;
    return d;
}

static inline uint8_t lines(void) { return (uint8_t)((gpio_get(DIAL_PIN_CLK) << 1) | gpio_get(DIAL_PIN_DT)); }

static void on_edge(uint gpio, uint32_t events) {
    (void)gpio; (void)events;
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

void dial_init(int saved_position, int saved_detents) {
    if (saved_detents < 0 || saved_detents > DETENTS_MAX || 1 + saved_detents / DIAL_DETENTS_PER_STEP != saved_position) {
        int p = saved_position < 1 ? 1 : saved_position > DIAL_POSITIONS ? DIAL_POSITIONS : saved_position;
        saved_detents = (p - 1) * DIAL_DETENTS_PER_STEP;   // older or inconsistent record: from the position alone
    }
    detents = saved_detents;
    position = 1 + detents / DIAL_DETENTS_PER_STEP;
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
    previous = lines();
    gpio_set_irq_enabled_with_callback(DIAL_PIN_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, on_edge);
    gpio_set_irq_enabled(DIAL_PIN_DT, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    printf("Dial: position %d of %d, detent %d (GP%d / GP%d, %d detents a step)\n", position, DIAL_POSITIONS, detents, DIAL_PIN_CLK, DIAL_PIN_DT, DIAL_DETENTS_PER_STEP);
}

int dial_position(void) { return position; }
int dial_detents(void) { return detents; }

void dial_set_position(int p) {
    uint32_t ints = save_and_disable_interrupts();
    if (p < 1) p = 1;
    if (p > DIAL_POSITIONS) p = DIAL_POSITIONS;
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

bool dial_moved(void) { return moved; }
void dial_set_moved(bool m) { moved = m; }

int dial_take_steps(void) {
    uint32_t ints = save_and_disable_interrupts();
    int steps = pending;
    pending = 0;
    restore_interrupts(ints);
    return steps;
}
