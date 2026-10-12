/* Inky Impression 5.7" (UC8159) driver.
 * Frames are 600 x 448 at 4 bits per pixel, UC8159 colour indices, top row first,
 * exactly the pixel data of the pN.bmp files produced by tools/render_pages.py.
 */
#ifndef HADISTORY_DISPLAY_H
#define HADISTORY_DISPLAY_H

#include <stdint.h>

enum display_colour { DISPLAY_BLACK = 0, DISPLAY_WHITE, DISPLAY_GREEN, DISPLAY_BLUE,
                      DISPLAY_RED, DISPLAY_YELLOW, DISPLAY_ORANGE, DISPLAY_CLEAN };

void display_init(void);
void display_show(const uint8_t *frame);   // blocking, about 30 s on this panel
void display_fill(uint8_t *frame, uint8_t colour);

#endif
