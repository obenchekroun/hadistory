#include "story.h"
#include "hadistory_config.h"
#include "display.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/rand.h"
#if HADISTORY_STORY_DIAL
#include "dial.h"
#endif

static story_info_t catalogue[MAX_STORIES];
static int catalogue_size = 0;
static char last_story[STORY_NAME_LEN];   // the story just left, not drawn again at random

// Number at the end of a story name (story_10 -> 10), -1 without one
static int story_number(const char *name) {
    size_t n = strlen(name), i = n;
    while (i > 0 && name[i - 1] >= '0' && name[i - 1] <= '9') i--;
    return i < n ? atoi(name + i) : -1;
}

// Numeric order so that the dial positions follow the story numbers: story_2 before story_10;
// unnumbered stories after, alphabetically
static int story_compare(const story_info_t *a, const story_info_t *b) {
    int na = story_number(a->name), nb = story_number(b->name);
    if (na < 0 && nb >= 0) return 1;
    if (nb < 0 && na >= 0) return -1;
    if (na != nb) return na < nb ? -1 : 1;
    return strcmp(a->name, b->name);
}

static void sort_catalogue(void) {
    for (int i = 1; i < catalogue_size; i++) {
        story_info_t key = catalogue[i];
        int j = i - 1;
        while (j >= 0 && story_compare(&catalogue[j], &key) > 0) { catalogue[j + 1] = catalogue[j]; j--; }
        catalogue[j + 1] = key;
    }
}

static int story_index(const char *name) {   // 1-based position in the catalogue, 0 if absent
    for (int i = 0; i < catalogue_size; i++) if (strcmp(catalogue[i].name, name) == 0) return i + 1;
    return 0;
}

static const story_info_t *find(const char *name) {
    for (int i = 0; i < catalogue_size; i++) {
        if (strcmp(catalogue[i].name, name) == 0) return &catalogue[i];
    }
    return NULL;
}

void story_init(settings_t *settings) {
    catalogue_size = storage_list_stories(catalogue, MAX_STORIES);
    sort_catalogue();
    printf("Stories available: %d\n", catalogue_size);
    for (int i = 0; i < catalogue_size; i++) printf("  %2d: %s (%d pages)\n", i + 1, catalogue[i].name, catalogue[i].page_count);
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
    if (settings->story[0]) snprintf(last_story, STORY_NAME_LEN, "%s", settings->story);
    settings->story[0] = '\0';
    settings->page = 0;
}

// A story at random, not the one just left when there is a choice
static const story_info_t *random_pick(void) {
    const story_info_t *pick;
    do {
        pick = &catalogue[get_rand_32() % catalogue_size];
    } while (catalogue_size > 1 && strcmp(pick->name, last_story) == 0);
    return pick;
}

static bool show_current_page(const settings_t *settings) {
    printf("Let's show the following story : %s, on page %d/%d\n",
           settings->story, settings->page, story_page_count(settings));
    page_data_t page;
    if (!storage_load_page(settings->story, settings->page, &page)) return false;
    render_layout_t layout = render_text(page.text);
    if (!render_jpeg(layout, page.jpeg, page.jpeg_len)) return false;
#if HADISTORY_STORY_DIAL
    render_page_counter(story_index(settings->story), settings->page, story_page_count(settings));
#else
    render_page_counter(0, settings->page, story_page_count(settings));
#endif
    render_battery();
    display_update();
    return true;
}

bool story_next_page(settings_t *settings) {
    if (catalogue_size == 0) return false;

    if (settings->page >= story_page_count(settings)) {
        story_reset(settings);
    }
    if (settings->story[0] == '\0') {
        const story_info_t *pick;
#if HADISTORY_STORY_DIAL
        if (dial_moved()) {
            // The wheel was turned since the last RESET / story: the story under the arrow
            // (a position beyond the catalogue gives the last story)
            int pos = dial_position();
            if (pos > catalogue_size) pos = catalogue_size;
            pick = &catalogue[pos - 1];
            printf("Dial at %d: story %s\n", dial_position(), pick->name);
        } else {
            pick = random_pick();
            printf("Dial untouched: story %s at random\n", pick->name);
        }
        dial_set_moved(false);   // the next pick needs a new turn of the wheel
#else
        pick = random_pick();
#endif
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
