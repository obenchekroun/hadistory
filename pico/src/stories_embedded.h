/* Catalogue of the stories embedded in the firmware (STORAGE=FLASH).
 * stories_table.c and stories_data.S are generated at build time by
 * tools/render_pages.py --embed, from the shared stories/ folder.
 */
#ifndef HADISTORY_STORIES_EMBEDDED_H
#define HADISTORY_STORIES_EMBEDDED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *name;
    int page_count;
    const uint8_t *const *pages;   // page_count pointers to FRAME_BYTES of pixel data in flash
} embedded_story_t;

extern const embedded_story_t embedded_stories[];
extern const int embedded_story_count;

#ifdef __cplusplus
}
#endif

#endif
