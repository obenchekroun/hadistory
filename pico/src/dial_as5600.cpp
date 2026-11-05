#include "dial.h"
#include "hadistory_config.h"

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

// Story dial on an AS5600 (STORY_DIAL=AS5600): a magnet on the wheel's axle turns over the
// sensor, which gives the absolute angle of the wheel, 12 bits a turn, over I2C. Nothing is
// counted: the position is read, at boot and then every DIAL_POLL_MS from the main loop, so it
// is right after a deep sleep and whatever happened meanwhile, with no stop to go back to.
//
// The number under the arrow is the angle from the "1", DIAL_STEP_DEG a number (the wheel_step of
// the case model), rounded to the nearest, with DIAL_HYST_DEG of hysteresis so that a wheel left
// between two numbers does not flicker. The angle of the "1" (the zero) depends on how the magnet
// sits in the axle: it is learnt once by dial_calibrate() with the 1 under the arrow, and saved
// with the settings. The wheel's travel stops DIAL_POSITIONS - 1 steps further; the arc left
// over counts as "below 1" for its half next to the 1, "above the last" for the other.
#define AS5600_ADDR     0x36
#define REG_CONF_L      0x08     // PWMF 7:6, OUTS 5:4, HYST 3:2, PM 1:0
#define REG_STATUS      0x0B     // MD 0x20 magnet detected, ML 0x10 too weak, MH 0x08 too strong
#define REG_RAW_ANGLE   0x0C     // 12 bits, high byte first
#define REG_AGC         0x1A     // gain: 128 in the middle of the range at 3.3 V
#define STATUS_MH       0x08
#define STATUS_ML       0x10
#define STATUS_MD       0x20
#define I2C_TIMEOUT_US  5000

#define STEP_DD   (DIAL_STEP_DEG * 10)                       // a step, in tenths of a degree
#define TRAVEL_DD ((DIAL_POSITIONS - 1) * STEP_DD)

static bool present;          // the sensor answers
static bool magnet;           // and sees its magnet
static int zero = -1;         // raw angle with the 1 under the arrow, -1 = not calibrated
static int position = 1;
static int pending;           // steps (position changes) since dial_take_steps()
static bool moved;            // the position changed since the last story start / re-arm
static absolute_time_t next_poll;

static bool reg_read(uint8_t reg, uint8_t *buf, size_t n) {
    if (i2c_write_timeout_us(DIAL_I2C, AS5600_ADDR, &reg, 1, true, I2C_TIMEOUT_US) != 1) return false;
    return i2c_read_timeout_us(DIAL_I2C, AS5600_ADDR, buf, n, false, I2C_TIMEOUT_US) == (int)n;
}

static bool reg_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = { reg, value };
    return i2c_write_timeout_us(DIAL_I2C, AS5600_ADDR, buf, 2, false, I2C_TIMEOUT_US) == 2;
}

// Raw angle 0..4095, or -1 when the sensor does not answer or has no magnet over it
static int read_raw(void) {
    uint8_t st, b[2];
    if (!reg_read(REG_STATUS, &st, 1) || !reg_read(REG_RAW_ANGLE, b, 2)) { present = false; return -1; }
    present = true;
    magnet = (st & STATUS_MD) != 0;
    if (!magnet) return -1;
    return ((b[0] & 0x0F) << 8) | b[1];
}

// Angle of the wheel from the 1, in tenths of a degree: 0 .. TRAVEL_DD over the numbers,
// negative below the 1 (at most half of the arc the stops leave out), more than TRAVEL_DD above
static int angle_dd(int raw) {
    int d = (raw - zero) & 4095;
#if DIAL_REVERSE
    d = (4096 - d) & 4095;
#endif
    int a = d * 3600 / 4096;
    if (a > TRAVEL_DD + (3600 - TRAVEL_DD) / 2) a -= 3600;
    return a;
}

static int nearest(int a) {
    if (a <= 0) return 1;
    int p = 1 + (a + STEP_DD / 2) / STEP_DD;
    return p > DIAL_POSITIONS ? DIAL_POSITIONS : p;
}

// One reading: the position follows the wheel once it is clearly on another number
static void poll(void) {
    int raw = read_raw();
    if (raw < 0 || zero < 0) return;
    int a = angle_dd(raw);
    if (abs(a - (position - 1) * STEP_DD) <= STEP_DD / 2 + DIAL_HYST_DEG * 10) return;
    int p = nearest(a);
    if (p != position) {
        pending += p - position;
        position = p;
        moved = true;
    }
}

void dial_init(int saved_position, int saved_detents, int saved_zero) {
    (void)saved_detents;
    position = saved_position < 1 ? 1 : saved_position > DIAL_POSITIONS ? DIAL_POSITIONS : saved_position;
    zero = (saved_zero >= 0 && saved_zero < 4096) ? saved_zero : DIAL_ZERO_RAW;
    pending = 0;
    moved = false;
#if DIAL_PIN_VCC >= 0
    // The module is fed by a GPIO so that it is off during the deep sleep: 6.5 mA at most,
    // 1.8 mA in the power mode set below; the strongest drive keeps the drop low
    gpio_init(DIAL_PIN_VCC);
    gpio_set_drive_strength(DIAL_PIN_VCC, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(DIAL_PIN_VCC, GPIO_OUT);
    gpio_put(DIAL_PIN_VCC, true);
#endif
#if DIAL_PIN_DIR >= 0
    gpio_init(DIAL_PIN_DIR);         // DIR must not float: low = angle growing clockwise seen from above the chip
    gpio_set_dir(DIAL_PIN_DIR, GPIO_OUT);
    gpio_put(DIAL_PIN_DIR, false);
#endif
    i2c_init(DIAL_I2C, 100 * 1000);
    gpio_set_function(DIAL_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DIAL_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DIAL_PIN_SDA);      // the module has its own pull-ups as well
    gpio_pull_up(DIAL_PIN_SCL);

    uint8_t st = 0;
    present = false;
    for (int i = 0; i < 6 && !present; i++) {   // 10 ms of power-up at most, per the datasheet
        sleep_ms(5);
        present = reg_read(REG_STATUS, &st, 1);
    }
    if (!present) {
        printf("Dial: no AS5600 at 0x%02x on GP%d / GP%d, the stories are picked at random\n", AS5600_ADDR, DIAL_PIN_SDA, DIAL_PIN_SCL);
        return;
    }
    uint8_t conf, agc = 0;
    if (reg_read(REG_CONF_L, &conf, 1)) reg_write(REG_CONF_L, (uint8_t)((conf & ~0x03) | (DIAL_AS5600_PM & 0x03)));
    reg_read(REG_AGC, &agc, 1);
    magnet = (st & STATUS_MD) != 0;
    printf("Dial: AS5600 on GP%d / GP%d, magnet %s%s, gain %d\n", DIAL_PIN_SDA, DIAL_PIN_SCL,
           magnet ? "detected" : "NOT DETECTED",
           (st & STATUS_MH) ? " (too strong: too close)" : (st & STATUS_ML) ? " (too weak: too far)" : "", agc);
    if (zero < 0) {
        puts("Dial: not calibrated: put the 1 under the arrow and hold RESET (B) for 3 s");
        return;
    }
    int raw = read_raw();
    if (raw >= 0) position = nearest(angle_dd(raw));   // where the wheel is now, whatever was saved
    printf("Dial: position %d of %d (raw %d, zero %d)\n", position, DIAL_POSITIONS, raw, zero);
    next_poll = make_timeout_time_ms(DIAL_POLL_MS);
}

int dial_position(void) { return position; }
int dial_detents(void) { return 0; }
int dial_zero(void) { return zero; }

// RESET: the wheel stays where it is, only the "moved" flag is re-armed
void dial_set_position(int p) {
    (void)p;
    pending = 0;
    moved = false;
}

bool dial_calibrate(void) {
    int raw = read_raw();
    if (raw < 0) {
        printf("Dial: cannot calibrate, %s\n", present ? "no magnet over the sensor" : "the sensor does not answer");
        return false;
    }
    zero = raw;
    position = 1;
    pending = 0;
    moved = false;
    printf("Dial: calibrated, the 1 is at raw %d (%d degrees)\n", zero, zero * 360 / 4096);
    return true;
}

bool dial_moved(void) { return moved; }
void dial_set_moved(bool m) { moved = m; }

int dial_take_steps(void) {
    if (time_reached(next_poll)) {
        next_poll = make_timeout_time_ms(DIAL_POLL_MS);
        poll();
    }
    int steps = pending;
    pending = 0;
    return steps;
}

// Before the deep sleep: the pads keep their state while the core is off, so the module is
// switched off and nothing is left pulling on its lines
void dial_sleep(void) {
    i2c_deinit(DIAL_I2C);
    const uint lines[2] = { DIAL_PIN_SDA, DIAL_PIN_SCL };
    for (int i = 0; i < 2; i++) {
        gpio_set_function(lines[i], GPIO_FUNC_SIO);
        gpio_set_dir(lines[i], GPIO_IN);
        gpio_disable_pulls(lines[i]);
    }
#if DIAL_PIN_VCC >= 0
    gpio_put(DIAL_PIN_VCC, false);
#endif
}
