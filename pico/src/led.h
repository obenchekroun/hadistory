/* Two LEDs, one question each.
 * Status LED (white sphere): activity only
 *   solid on   : waiting for a button press
 *   breathing  : busy (generating / displaying), driven by a hardware timer, no thread
 *   rapid blink: error (blocking, as in hadistory.py)
 * Mode LED (red, MODE_LED_PIN): on in AI mode, off in story mode, blinking while the
 *   WiFi is being joined for a switch to AI mode. The mode bursts of the Python version
 *   are gone: the mode LED and the robot badge on the page carry the mode.
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
void led_mode_blink_start(void);     // joining the WiFi for the switch to AI mode

#endif
