/* Two sphere LEDs, one question each.
 * Status LED (white, LED_PIN): activity only
 *   solid on   : waiting for a button press
 *   breathing  : busy (generating / displaying), driven by a hardware timer, no thread
 *   rapid blink: error (blocking, as in hadistory.py)
 * Mode LED (green, MODE_LED_PIN): on in AI mode, off in story mode, 2 Hz blink while the WiFi
 *   is being joined for a switch to AI mode. Its steady state says it all; the bursts of
 *   hadistory.py at a mode change (3 long = AI, 5 short = story) stay behind MODE_LED_BURSTS, off
 *   by default. The status LED never says anything about the mode; the page shows it too (robot
 *   badge top-left on the AI pages).
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

void led_mode_init(void);
void led_mode_set(bool ai);          // steady: on = AI mode (stops a blink)
void led_mode_announce(bool ai);     // mode change: steady state, after the optional bursts (MODE_LED_BURSTS)
void led_mode_blink_start(void);     // joining the WiFi for the switch to AI mode

#endif
