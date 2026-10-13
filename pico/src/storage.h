/* Storage of the stories and of the persisted state. Two backends, chosen at build
 * time with -DSTORAGE=SDCARD|FLASH (HADISTORY_STORAGE_SDCARD / HADISTORY_STORAGE_FLASH):
 *   storage_sdcard.cpp  FAT microSD card: /stories/<story>/pN.bmp (tools/README.md),
 *                       state in /currently.txt
 *   storage_flash.cpp   pages embedded in the firmware by tools/render_pages.py --embed,
 *                       state in the last flash sector
 * Pages are delivered row by row (DISPLAY_ROW_BYTES of 4 bpp colour indices) to a sink,
 * so that no full-frame buffer is needed besides the PicoGraphics one.
 */
#ifndef HADISTORY_STORAGE_H
#define HADISTORY_STORAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "hadistory_config.h"

typedef struct {
    char name[STORY_NAME_LEN];
    int page_count;
} story_info_t;

typedef struct {
    char story[STORY_NAME_LEN];   // "" when no story is chosen
    int page;                     // 0 = none shown yet
    bool ai_mode;
} settings_t;

typedef void (*row_sink_t)(int y, const uint8_t *packed_row);

bool storage_init(void);
int  storage_list_stories(story_info_t *out, int max);     // returns count
bool storage_load_page(const char *story, int page, row_sink_t sink);
bool storage_load_settings(settings_t *s);
bool storage_save_settings(const settings_t *s);

#endif
