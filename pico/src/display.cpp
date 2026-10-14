#include "display.h"
#include "hadistory_config.h"

#include "drivers/uc8159/uc8159.hpp"

using namespace pimoroni;

static UC8159 *driver = nullptr;
static PicoGraphics_Pen3Bit *graphics = nullptr;

void display_init(void) {
    SPIPins pins = { DISPLAY_SPI, DISPLAY_PIN_CS, DISPLAY_PIN_SCK, DISPLAY_PIN_MOSI,
                     PIN_UNUSED, DISPLAY_PIN_DC, PIN_UNUSED };
    driver = new UC8159(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_ROTATION, pins, DISPLAY_PIN_BUSY, DISPLAY_PIN_RESET);
    driver->set_blocking(true);
    graphics = new PicoGraphics_Pen3Bit(DISPLAY_WIDTH, DISPLAY_HEIGHT, nullptr);
    display_clear(UC8159::WHITE);
}

PicoGraphics &display_graphics(void) {
    return *graphics;
}

void display_clear(uint8_t colour) {
    graphics->set_pen(colour);
    graphics->clear();
}

void display_update(void) {
    driver->update(graphics);
}
