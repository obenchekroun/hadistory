/* Storage backend for STORAGE=SDCARD: FAT microSD card over PIO SPI (pimoroni sdcard + fatfs).
 * Card layout (copied from tools/out/): /stories/<story>/pN.bmp, state in /currently.txt */
#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"

static FATFS fs;

bool storage_init(void) {
    FRESULT r = f_mount(&fs, "", 1);
    if (r != FR_OK) {
        printf("SD card: mount failed (%d)\n", r);
        return false;
    }
    printf("SD card: mounted\n");
    return true;
}

static int count_pages(const char *story) {
    // Pages are p1.bmp .. pN.bmp without gaps, like the txt/pN.txt of the Pi version
    char path[64];
    int n = 0;
    for (;;) {
        FILINFO fi;
        snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.bmp", story, n + 1);
        if (f_stat(path, &fi) != FR_OK) break;
        n++;
    }
    return n;
}

int storage_list_stories(story_info_t *out, int max) {
    DIR dir;
    FILINFO fi;
    int count = 0;
    if (f_opendir(&dir, STORIES_DIR) != FR_OK) {
        printf("SD card: cannot open " STORIES_DIR "\n");
        return 0;
    }
    while (count < max && f_readdir(&dir, &fi) == FR_OK && fi.fname[0] != '\0') {
        if (!(fi.fattrib & AM_DIR) || fi.fname[0] == '.') continue;
        strncpy(out[count].name, fi.fname, STORY_NAME_LEN - 1);
        out[count].name[STORY_NAME_LEN - 1] = '\0';
        out[count].page_count = count_pages(out[count].name);
        if (out[count].page_count > 0) count++;
    }
    f_closedir(&dir);
    return count;
}

bool storage_load_page(const char *story, int page, uint8_t *frame) {
    char path[64];
    FIL f;
    UINT read = 0;
    snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.bmp", story, page);
    if (f_open(&f, path, FA_READ) != FR_OK) {
        printf("SD card: cannot open %s\n", path);
        return false;
    }
    bool ok = f_size(&f) == PAGE_BMP_SIZE
           && f_lseek(&f, PAGE_BMP_PIXEL_OFFSET) == FR_OK
           && f_read(&f, frame, FRAME_BYTES, &read) == FR_OK
           && read == FRAME_BYTES;
    f_close(&f);
    if (!ok) printf("SD card: %s is not a valid page file\n", path);
    return ok;
}

bool storage_load_settings(settings_t *s) {
    FIL f;
    char line[64];
    memset(s, 0, sizeof *s);
    if (f_open(&f, SETTINGS_FILE, FA_READ) != FR_OK) return false;
    while (f_gets(line, sizeof line, &f)) {
        char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
        if (strncmp(line, "story=", 6) == 0) {
            snprintf(s->story, STORY_NAME_LEN, "%.*s", STORY_NAME_LEN - 1, line + 6);
        } else if (strncmp(line, "page=", 5) == 0) {
            s->page = atoi(line + 5);
        } else if (strncmp(line, "mode=", 5) == 0) {
            s->ai_mode = strcmp(line + 5, "AI") == 0;
        }
    }
    f_close(&f);
    return true;
}

bool storage_save_settings(const settings_t *s) {
    FIL f;
    if (f_open(&f, SETTINGS_FILE, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return false;
    f_printf(&f, "story=%s\npage=%d\nmode=%s\n", s->story, s->page, s->ai_mode ? "AI" : "local");
    f_close(&f);
    return true;
}
