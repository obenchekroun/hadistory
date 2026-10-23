/* Four active-low buttons with internal pull-ups (plus the dial push button on RESET with STORY_DIAL).
 * buttons_poll() samples them once; buttons_pressed() reports a falling edge since
 * the previous poll, which mirrors the previous_*_state logic of hadistory.py.
 */
#ifndef HADISTORY_BUTTONS_H
#define HADISTORY_BUTTONS_H

#include <stdbool.h>

typedef enum {
    BUTTON_EXECUTE = 0,
    BUTTON_RESET,
    BUTTON_MODE,
    BUTTON_BACK,
    BUTTON_COUNT
} button_t;

void buttons_init(void);
void buttons_poll(void);
bool buttons_pressed(button_t b);   // true once per press, on the poll where it went down
void buttons_inject_press(button_t b);   // report b as pressed on the next buttons_pressed() (wake from sleep)

#endif
