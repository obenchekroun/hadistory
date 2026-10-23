/* Story mode: pick a story (at random, or by the dial with STORY_DIAL), step through its pages, go back.
 * Mirrors the "Story Mode" branches of the main loop in hadistory.py.
 */
#ifndef HADISTORY_STORY_H
#define HADISTORY_STORY_H

#include <stdbool.h>
#include "storage.h"

void story_init(settings_t *settings);          // loads the catalogue from the storage
bool story_next_page(settings_t *settings);     // picks a story if needed, shows the page
bool story_previous_page(settings_t *settings);
void story_reset(settings_t *settings);
int  story_page_count(const settings_t *settings);

#endif
