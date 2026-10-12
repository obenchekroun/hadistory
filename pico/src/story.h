/* Story mode: pick a story at random, step through its pages, go back.
 * Mirrors the "Story Mode" branches of the main loop in hadistory.py.
 */
#ifndef HADISTORY_STORY_H
#define HADISTORY_STORY_H

#include <stdbool.h>
#include <stdint.h>
#include "storage.h"

void story_init(settings_t *settings);          // loads the catalogue from the card
bool story_next_page(settings_t *settings, uint8_t *frame);   // picks a story if needed
bool story_previous_page(settings_t *settings, uint8_t *frame);
void story_reset(settings_t *settings);
int  story_page_count(const settings_t *settings);

#endif
