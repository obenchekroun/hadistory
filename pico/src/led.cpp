#include "led.h"
#include "hadistory_config.h"

#include "pico/stdlib.h"

#if HADISTORY_LED_CYW43
// ============================================================================
// Onboard LED of the Pico W family, driven through the wireless chip.
// No PWM there: the LED is on/off only and the busy indication is a 1 Hz blink.
// The blink runs as a timed worker of the cyw43 async context, which is the
// context where cyw43 calls are allowed; cyw43_arch_init() is done in main().
// ============================================================================
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

static async_at_time_worker_t blink_worker;
static volatile bool breathing = false;
static bool blink_state = false;

static void set_led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

static void blink_work(async_context_t *context, async_at_time_worker_t *worker) {
    if (!breathing) {
        set_led(false);
        return;
    }
    blink_state = !blink_state;
    set_led(blink_state);
    async_context_add_at_time_worker_in_ms(context, worker, 500);
}

void led_init(void) {
    blink_worker.do_work = blink_work;
    set_led(false);
}

void led_on(void)  { led_breathe_stop(); set_led(true); }
void led_off(void) { led_breathe_stop(); set_led(false); }

void led_breathe_start(void) {
    if (breathing) return;
    breathing = true;
    blink_state = false;
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &blink_worker, 0);
}

void led_breathe_stop(void) {
    if (!breathing) return;
    breathing = false;
    async_context_remove_at_time_worker(cyw43_arch_async_context(), &blink_worker);
    set_led(false);
}

static void bursts(int count, int on_ms, int off_ms) {
    led_breathe_stop();
    for (int i = 0; i < count; i++) {
        set_led(true);
        sleep_ms(on_ms);
        set_led(false);
        sleep_ms(off_ms);
    }
}

#else
// ============================================================================
// LED on a GPIO (external LED, or the board LED on e.g. the Pico Plus 2): PWM,
// with a breathing ramp driven by a hardware timer during busy phases.
// ============================================================================
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

#endif  // HADISTORY_LED_CYW43

// Error pattern of hadistory.py, whatever the LED
void led_rapid_blink(void)       { bursts(19, 100, 100); }

// ============================================================================
// Mode LED: a plain GPIO, 2 Hz blink on a hardware timer while the WiFi is joined
// ============================================================================
static repeating_timer_t mode_timer;
static volatile bool mode_blinking = false;
static bool mode_state = false;

static bool mode_tick(repeating_timer_t *t) {
    (void)t;
    if (!mode_blinking) return false;
    mode_state = !mode_state;
    gpio_put(MODE_LED_PIN, mode_state);
    return true;
}

void led_mode_init(void) {
    gpio_init(MODE_LED_PIN);
    gpio_set_dir(MODE_LED_PIN, GPIO_OUT);
    gpio_put(MODE_LED_PIN, false);
}

void led_mode_set(bool ai) {
    if (mode_blinking) {
        mode_blinking = false;
        cancel_repeating_timer(&mode_timer);
    }
    mode_state = ai;
    gpio_put(MODE_LED_PIN, ai);
}

void led_mode_blink_start(void) {
    if (mode_blinking) return;
    mode_blinking = true;
    mode_state = true;
    gpio_put(MODE_LED_PIN, true);
    add_repeating_timer_ms(250, mode_tick, NULL, &mode_timer);
}
