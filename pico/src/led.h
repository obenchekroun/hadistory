/* Status LED, same semantics as the Python version:
 *   solid on   : waiting for a button press
 *   breathing  : busy (generating / displaying), driven by a hardware timer, no thread
 *   bursts     : mode announcements and errors (blocking, as in hadistory.py)
 */
#ifndef HADISTORY_LED_H
#define HADISTORY_LED_H

#include <stdbool.h>

void led_init(void);
void led_on(void);
void led_off(void);
void led_breathe_start(void);   // fade in/out until led_breathe_stop()
void led_breathe_stop(void);    // leaves the LED off
void led_five_short_bursts(void);   // story mode
void led_three_long_bursts(void);   // AI mode
void led_rapid_blink(void);         // error

#endif
