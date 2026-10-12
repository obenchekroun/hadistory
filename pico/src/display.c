/* UC8159 sequence transposed from pimoroni-pico drivers/uc8159/uc8159.cpp
 * and pimoroni/inky inky_uc8159.py, trimmed to the 600 x 448 panel. */
#include "display.h"
#include "hadistory_config.h"

#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

enum reg {
    PSR = 0x00, PWR = 0x01, POF = 0x02, PFS = 0x03, PON = 0x04, BTST = 0x06,
    DSLP = 0x07, DTM1 = 0x10, DSP = 0x11, DRF = 0x12, PLL = 0x30, TSC = 0x40,
    CDI = 0x50, TCON = 0x60, TRES = 0x61, PWS = 0xE3
};

static void busy_wait(void) {
    while (!gpio_get(DISPLAY_PIN_BUSY)) {
        tight_loop_contents();
    }
}

static void command(uint8_t reg, const uint8_t *data, size_t len) {
    gpio_put(DISPLAY_PIN_CS, 0);
    gpio_put(DISPLAY_PIN_DC, 0);
    spi_write_blocking(DISPLAY_SPI, &reg, 1);
    if (len > 0) {
        gpio_put(DISPLAY_PIN_DC, 1);
        spi_write_blocking(DISPLAY_SPI, data, len);
    }
    gpio_put(DISPLAY_PIN_CS, 1);
}

#define CMD(reg, ...) do { const uint8_t d_[] = { __VA_ARGS__ }; command(reg, d_, sizeof d_); } while (0)

static void reset(void) {
    gpio_put(DISPLAY_PIN_RESET, 0); sleep_ms(10);
    gpio_put(DISPLAY_PIN_RESET, 1); sleep_ms(10);
    busy_wait();
}

static void setup(void) {
    reset();
    busy_wait();
    CMD(PSR,  0xE3, 0x08);                 // 600 px wide panel, rotation 0
    CMD(PWR,  0x37, 0x00, 0x23, 0x23);
    CMD(PFS,  0x00);
    CMD(BTST, 0xC7, 0xC7, 0x1D);
    CMD(PLL,  0x3C);
    CMD(TSC,  0x00);
    CMD(CDI,  0x37);
    CMD(TCON, 0x22);
    CMD(TRES, DISPLAY_WIDTH >> 8, DISPLAY_WIDTH & 0xFF, DISPLAY_HEIGHT >> 8, DISPLAY_HEIGHT & 0xFF);
    CMD(PWS,  0xAA);
    sleep_ms(100);
    CMD(CDI,  0x37);
}

void display_init(void) {
    spi_init(DISPLAY_SPI, 3 * 1000 * 1000);
    gpio_set_function(DISPLAY_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(DISPLAY_PIN_CS);    gpio_set_dir(DISPLAY_PIN_CS, GPIO_OUT);    gpio_put(DISPLAY_PIN_CS, 1);
    gpio_init(DISPLAY_PIN_DC);    gpio_set_dir(DISPLAY_PIN_DC, GPIO_OUT);
    gpio_init(DISPLAY_PIN_RESET); gpio_set_dir(DISPLAY_PIN_RESET, GPIO_OUT); gpio_put(DISPLAY_PIN_RESET, 1);
    gpio_init(DISPLAY_PIN_BUSY);  gpio_set_dir(DISPLAY_PIN_BUSY, GPIO_IN);   gpio_pull_up(DISPLAY_PIN_BUSY);
}

void display_show(const uint8_t *frame) {
    setup();
    command(DTM1, frame, FRAME_BYTES);
    busy_wait();
    command(PON, NULL, 0);
    busy_wait();
    command(DRF, NULL, 0);
    sleep_ms(200);
    busy_wait();          // the refresh itself, about 30 s
    command(POF, NULL, 0);
    busy_wait();
}

void display_fill(uint8_t *frame, uint8_t colour) {
    memset(frame, (colour << 4) | (colour & 0x0F), FRAME_BYTES);
}
