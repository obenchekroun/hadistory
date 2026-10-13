#include "story.h"
#include "hadistory_config.h"
#include "display.h"

#include <stdio.h>
#include <string.h>
#include "pico/rand.h"

static story_info_t catalogue[MAX_STORIES];
static int catalogue_size = 0;

static const story_info_t *find(const char *name) {
    for (int i = 0; i < catalogue_size; i++) {
        if (strcmp(catalogue[i].name, name) == 0) return &catalogue[i];
    }
    return NULL;
}

void story_init(settings_t *settings) {
    catalogue_size = storage_list_stories(catalogue, MAX_STORIES);
    printf("Stories available: %d\n", catalogue_size);
    if (settings->story[0] && !find(settings->story)) {
        printf("Saved story %s not available, starting over\n", settings->story);
        story_reset(settings);
    }
}

int story_page_count(const settings_t *settings) {
    const story_info_t *s = find(settings->story);
    return s ? s->page_count : 0;
}

void story_reset(settings_t *settings) {
    settings->story[0] = '\0';
    settings->page = 0;
}

static bool show_current_page(const settings_t *settings) {
    printf("Let's show the following story : %s, on page %d/%d\n",
           settings->story, settings->page, story_page_count(settings));
    if (!storage_load_page(settings->story, settings->page, display_set_row)) return false;
    display_update();
    return true;
}

bool story_next_page(settings_t *settings) {
    if (catalogue_size == 0) return false;

    if (settings->page >= story_page_count(settings)) {
        story_reset(settings);
    }
    if (settings->story[0] == '\0') {
        const story_info_t *pick = &catalogue[get_rand_32() % catalogue_size];
        snprintf(settings->story, STORY_NAME_LEN, "%s", pick->name);
        settings->page = 0;
    }
    settings->page++;
    if (!show_current_page(settings)) return false;
    puts(settings->page >= story_page_count(settings) ? "The end." : "To be continued...");
    return true;
}

bool story_previous_page(settings_t *settings) {
    if (settings->story[0] == '\0' || settings->page <= 1) {
        puts("Nothing to show, skipping ...");
        return false;
    }
    settings->page--;
    if (!show_current_page(settings)) return false;
    puts("To be continued...");
    return true;
}
