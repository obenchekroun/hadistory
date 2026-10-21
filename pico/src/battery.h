/* Battery gauge (BATTERY_GAUGE=ON, HADISTORY_BATTERY=1): the cell voltage estimated
 * from VSYS, through the divider by 3 the board has on PICO_VSYS_PIN, plus the drop of
 * the Schottky diode between the battery and VSYS (pico/README.md, "TP4056 wiring").
 * Shown as bars on each page, purely informative: nothing in the firmware acts on it.
 * On USB, VSYS is the USB voltage and says nothing about the cell: the gauge then only
 * reports "on USB".
 */
#ifndef HADISTORY_BATTERY_H
#define HADISTORY_BATTERY_H

#include <stdbool.h>

typedef struct {
    int  millivolts;   // estimated cell voltage, 0 when on USB
    bool on_usb;       // VSYS above BATTERY_USB_MV: powered by the USB, battery charging
    int  bars;         // 0 (empty) .. BATTERY_BARS (full); BATTERY_BARS when on USB
} battery_t;

void      battery_init(void);
battery_t battery_read(void);                  // a few conversions, about a millisecond

#endif
