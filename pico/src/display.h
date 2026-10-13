/* Display output through the Pimoroni libraries: a PicoGraphics 3-bit frame buffer
 * and the UC8159 driver of the Inky Impression 5.7". Changing panel means changing
 * the driver class and the geometry here, nothing else in the project.
 *
 * Pages from the storage arrive as rows of 4 bpp colour indices (the BMP pixel data,
 * see tools/README.md) and are pushed with display_set_row(); the AI mode draws
 * directly into display_graphics().
 */
#ifndef HADISTORY_DISPLAY_H
#define HADISTORY_DISPLAY_H

#include <stdint.h>
#include "libraries/pico_graphics/pico_graphics.hpp"

void display_init(void);
pimoroni::PicoGraphics &display_graphics(void);
void display_clear(uint8_t colour);
void display_set_row(int y, const uint8_t *packed_row);   // DISPLAY_ROW_BYTES bytes, 2 pixels per byte, high nibble first
void display_update(void);                                 // blocking, about 30 s on this panel

#endif
