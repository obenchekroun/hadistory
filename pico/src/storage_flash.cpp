/* Storage backend for STORAGE=FLASH: pages embedded in the firmware image,
 * persisted state in the last 4 KB sector of the flash. */
#include "storage.h"
#include "stories_embedded.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define SETTINGS_MAGIC         0x48414449u   // "HADI"
#define SETTINGS_FLASH_OFFSET  (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

typedef struct {
    uint32_t magic;
    char story[STORY_NAME_LEN];
    int32_t page;
    uint32_t ai_mode;
} settings_record_t;

bool storage_init(void) {
    printf("Flash storage: %d embedded stories\n", embedded_story_count);
    return true;
}

int storage_list_stories(story_info_t *out, int max) {
    int count = 0;
    for (int i = 0; i < embedded_story_count && count < max; i++) {
        if (embedded_stories[i].page_count <= 0) continue;
        snprintf(out[count].name, STORY_NAME_LEN, "%.*s", STORY_NAME_LEN - 1, embedded_stories[i].name);
        out[count].page_count = embedded_stories[i].page_count;
        count++;
    }
    return count;
}

bool storage_load_page(const char *story, int page, page_data_t *out) {
    for (int i = 0; i < embedded_story_count; i++) {
        if (strcmp(embedded_stories[i].name, story) != 0) continue;
        if (page < 1 || page > embedded_stories[i].page_count) return false;
        const embedded_page_t *p = &embedded_stories[i].pages[page - 1];   // read in place from XIP flash
        out->jpeg = p->jpeg;
        out->jpeg_len = p->jpeg_len;
        out->text = p->text;
        return true;
    }
    printf("Flash storage: story %s not found\n", story);
    return false;
}

bool storage_load_settings(settings_t *s) {
    const settings_record_t *rec = (const settings_record_t *)(XIP_BASE + SETTINGS_FLASH_OFFSET);
    memset(s, 0, sizeof *s);
    if (rec->magic != SETTINGS_MAGIC) return false;
    snprintf(s->story, STORY_NAME_LEN, "%.*s", STORY_NAME_LEN - 1, rec->story);
    s->page = rec->page;
    s->ai_mode = rec->ai_mode != 0;
    return true;
}

bool storage_save_settings(const settings_t *s) {
    static uint8_t buf[FLASH_PAGE_SIZE] __attribute__((aligned(4)));
    settings_record_t rec;
    memset(&rec, 0, sizeof rec);
    rec.magic = SETTINGS_MAGIC;
    snprintf(rec.story, STORY_NAME_LEN, "%s", s->story);
    rec.page = s->page;
    rec.ai_mode = s->ai_mode ? 1 : 0;
    memset(buf, 0xFF, sizeof buf);
    memcpy(buf, &rec, sizeof rec);

    // Flash writes must run with interrupts off and nothing executing from flash
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(SETTINGS_FLASH_OFFSET, buf, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    return true;
}
