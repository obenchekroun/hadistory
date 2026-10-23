/* Status LED (white sphere), the patterns of hadistory.py:
 *   solid on   : waiting for a button press
 *   breathing  : busy (generating / displaying), driven by a hardware timer, no thread
 *   rapid blink: error (blocking)
 *   3 long bursts  : switched to AI mode (also at a cold boot in that mode)
 *   5 short bursts : switched to story mode
 * The mode itself is readable on the page: a robot badge top-left on the AI pages.
 */
#ifndef HADISTORY_LED_H
#define HADISTORY_LED_H

#include <stdbool.h>

void led_init(void);
void led_on(void);
void led_off(void);
void led_breathe_start(void);   // fade in/out until led_breathe_stop()
void led_breathe_stop(void);    // leaves the LED off
void led_rapid_blink(void);         // error
void led_five_short_bursts(void);   // story mode
void led_three_long_bursts(void);   // AI mode

#endif
