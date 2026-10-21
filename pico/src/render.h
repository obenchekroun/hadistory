/* Composition of a page directly into the display frame buffer, with the layout of
 * pi/hadistory.py: portrait page, illustration square on top, text below, the whole
 * rotated 90 degrees counter-clockwise onto the landscape panel. Used by the story
 * mode (JPEG + text from the storage) and by the AI mode (JPEG from OpenAI).
 * The bitmap font is drawn rotated by PicoGraphics, so no intermediate buffer is needed.
 */
#ifndef HADISTORY_RENDER_H
#define HADISTORY_RENDER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int image_size;      // side of the illustration square, 448 at most
    int image_offset;    // portrait x offset of the square (centred when smaller than 448)
    int lines;           // number of text lines
} render_layout_t;

// Clears the frame, lays out and draws the story text. Returns the layout so that the
// illustration can be drawn into the remaining square.
render_layout_t render_text(const char *text);

// Decodes a JPEG (any size, any aspect, the whole image is fitted into the square by
// nearest-neighbour sampling) and draws it dithered to the panel colours.
bool render_jpeg(const render_layout_t &layout, const uint8_t *data, size_t len);

// Draws "page/total" top-left on a white box, over the illustration, as the Pi version does
void render_page_counter(int page, int total);

// Draws the robot badge of the AI mode at the same place: the page itself tells the mode
void render_mode_badge(void);

// Battery gauge, top-right of each page (BATTERY_GAUGE=ON): main.cpp sets the reading before
// an action, the page composers draw it before the panel update. bars < 0 draws nothing.
void render_set_battery(int bars, bool on_usb);
void render_battery(void);

// Shared buffer for JPEG data read from the SD card or received from the network
uint8_t *render_image_buffer(size_t *capacity);

#endif
