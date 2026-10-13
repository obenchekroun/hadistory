#include "led.h"
#include "hadistory_config.h"

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define PWM_TOP 1000

static uint slice, channel;
static repeating_timer_t breathe_timer;
static volatile bool breathing = false;
static int level = 0;      // 0..PWM_TOP
static int step = 0;       // signed increment per tick
static int hold_ticks = 0; // pause at the ends of the ramp, as the 0.75 s in fade_leds()

static void set_level(int l) {
    pwm_set_chan_level(slice, channel, (uint16_t)l);
}

// 50 ms tick: ramp 0 -> 100 % in 1 s, hold 0.75 s, ramp down, hold, like fade_leds() in hadistory.py
static bool breathe_tick(repeating_timer_t *t) {
    (void)t;
    if (!breathing) {
        set_level(0);
        return false;
    }
    if (hold_ticks > 0) {
        hold_ticks--;
        return true;
    }
    level += step;
    if (level >= PWM_TOP) { level = PWM_TOP; step = -step; hold_ticks = 15; }
    if (level <= 0)       { level = 0;       step = -step; hold_ticks = 15; }
    set_level(level);
    return true;
}

void led_init(void) {
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(LED_PIN);
    channel = pwm_gpio_to_channel(LED_PIN);
    pwm_set_wrap(slice, PWM_TOP);
    pwm_set_clkdiv(slice, 4.0f);   // ~37 kHz at 150 MHz, flicker free
    set_level(0);
    pwm_set_enabled(slice, true);
}

void led_on(void)  { led_breathe_stop(); set_level(PWM_TOP); }
void led_off(void) { led_breathe_stop(); set_level(0); }

void led_breathe_start(void) {
    if (breathing) return;
    breathing = true;
    level = 0;
    step = PWM_TOP / 20;
    hold_ticks = 0;
    add_repeating_timer_ms(50, breathe_tick, NULL, &breathe_timer);
}

void led_breathe_stop(void) {
    if (!breathing) return;
    breathing = false;
    cancel_repeating_timer(&breathe_timer);
    set_level(0);
}

static void bursts(int count, int on_ms, int off_ms) {
    led_breathe_stop();
    for (int i = 0; i < count; i++) {
        set_level(PWM_TOP);
        sleep_ms(on_ms);
        set_level(0);
        sleep_ms(off_ms);
    }
}

void led_five_short_bursts(void) { bursts(5, 250, 250); }
void led_three_long_bursts(void) { bursts(3, 800, 800); }
void led_rapid_blink(void)       { bursts(19, 100, 100); }
