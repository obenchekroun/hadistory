/* Display output through the Pimoroni libraries: a PicoGraphics 3-bit frame buffer
 * and the UC8159 driver of the Inky Impression 5.7". Changing panel means changing
 * the driver class and the geometry here, nothing else in the project.
 * Pages are composed into display_graphics() by render.cpp.
 */
#ifndef HADISTORY_DISPLAY_H
#define HADISTORY_DISPLAY_H

#include <stdint.h>
#include "libraries/pico_graphics/pico_graphics.hpp"

// Palette indices of the panel, same order as the UC8159 driver
enum display_colour { DISPLAY_BLACK = 0, DISPLAY_WHITE, DISPLAY_GREEN, DISPLAY_BLUE,
                      DISPLAY_RED, DISPLAY_YELLOW, DISPLAY_ORANGE, DISPLAY_CLEAN };

void display_init(void);
pimoroni::PicoGraphics &display_graphics(void);
void display_clear(uint8_t colour);
void display_update(void);                                 // blocking, about 30 s on this panel

#endif
