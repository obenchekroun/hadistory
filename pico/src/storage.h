/* Storage of the stories and of the persisted state. Two backends, chosen at build
 * time with -DSTORAGE=SDCARD|FLASH (HADISTORY_STORAGE_SDCARD / HADISTORY_STORAGE_FLASH):
 *   storage_sdcard.cpp  FAT microSD card: /stories/<story>/pN.jpg + pN.txt (tools/README.md),
 *                       state in /currently.txt
 *   storage_flash.cpp   pages embedded in the firmware by tools/prepare_pages.py --embed,
 *                       state in the last flash sector
 * A page is a JPEG illustration plus its text; the firmware composes it (render.cpp).
 */
#ifndef HADISTORY_STORAGE_H
#define HADISTORY_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
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
    int dial;                     // STORY_DIAL: wheel position 1..DIAL_POSITIONS, 0 = unknown
} settings_t;

typedef struct {
    const uint8_t *jpeg;          // in flash, or in the shared image buffer (SD card)
    size_t jpeg_len;
    const char *text;             // NUL terminated
} page_data_t;

bool storage_init(void);
int  storage_list_stories(story_info_t *out, int max);     // returns count
bool storage_load_page(const char *story, int page, page_data_t *out);
bool storage_load_settings(settings_t *s);
bool storage_save_settings(const settings_t *s);

#endif
