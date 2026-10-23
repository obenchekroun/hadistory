/* Storage backend for STORAGE=SDCARD: FAT microSD card over PIO SPI (pimoroni sdcard + fatfs).
 * Card layout (copied from tools/out/): /stories/<story>/pN.jpg + pN.txt, state in /currently.txt */
#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "render.h"

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
    // Pages are p1 .. pN without gaps, like the txt/pN.txt of the Pi version
    char path[64];
    int n = 0;
    for (;;) {
        FILINFO fi;
        snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.txt", story, n + 1);
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
        snprintf(out[count].name, STORY_NAME_LEN, "%.*s", STORY_NAME_LEN - 1, fi.fname);
        out[count].page_count = count_pages(out[count].name);
        if (out[count].page_count > 0) count++;
    }
    f_closedir(&dir);
    return count;
}

static bool read_file(const char *path, uint8_t *buf, size_t cap, size_t *len) {
    FIL f;
    UINT read = 0;
    *len = 0;
    if (f_open(&f, path, FA_READ) != FR_OK) {
        printf("SD card: cannot open %s\n", path);
        return false;
    }
    FSIZE_t size = f_size(&f);
    bool ok = size <= cap && f_read(&f, buf, (UINT)size, &read) == FR_OK && read == size;
    f_close(&f);
    if (!ok) printf("SD card: cannot read %s (%lu bytes, buffer %u)\n", path, (unsigned long)size, (unsigned)cap);
    *len = read;
    return ok;
}

bool storage_load_page(const char *story, int page, page_data_t *out) {
    static char text[PAGE_TEXT_MAX];
    char path[64];
    size_t cap, len;
    uint8_t *jpeg = render_image_buffer(&cap);

    snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.jpg", story, page);
    if (!read_file(path, jpeg, cap, &len)) return false;
    out->jpeg = jpeg;
    out->jpeg_len = len;

    snprintf(path, sizeof path, STORIES_DIR "/%s/p%d.txt", story, page);
    if (!read_file(path, (uint8_t *)text, sizeof text - 1, &len)) return false;
    text[len] = '\0';
    out->text = text;
    return true;
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
        } else if (strncmp(line, "dial=", 5) == 0) {
            s->dial = atoi(line + 5);
        }
    }
    f_close(&f);
    return true;
}

bool storage_save_settings(const settings_t *s) {
    FIL f;
    if (f_open(&f, SETTINGS_FILE, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return false;
    f_printf(&f, "story=%s\npage=%d\nmode=%s\ndial=%d\n", s->story, s->page, s->ai_mode ? "AI" : "local", s->dial);
    f_close(&f);
    return true;
}
