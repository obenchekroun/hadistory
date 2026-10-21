#include "battery.h"
#include "hadistory_config.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#if HADISTORY_HAS_CYW43 && CYW43_USES_VSYS_PIN
#include "pico/cyw43_arch.h"
#endif

static const int SAMPLES = 8;
static const int BAR_MV[BATTERY_BARS] = BATTERY_BAR_MV;   // cell voltage for 4, 3, 2, 1 bars

void battery_init(void) {
    adc_init();
}

// VSYS in millivolts: PICO_VSYS_PIN sees VSYS / 3 (200k / 100k divider on the board),
// converted over the 3.3 V reference of the 12-bit ADC.
static int read_vsys_mv(void) {
#if HADISTORY_HAS_CYW43 && CYW43_USES_VSYS_PIN
    // On the Pico W family the same pin is the clock of the wireless SPI: hold the
    // driver while the pin is borrowed, it reclaims the pin at its next transfer
    cyw43_thread_enter();
#endif
    adc_gpio_init(PICO_VSYS_PIN);                    // analogue input, pulls off
    adc_select_input(PICO_VSYS_PIN - ADC_BASE_PIN);
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    int ignore = SAMPLES;                            // the first conversions read low
    while (!adc_fifo_is_empty() || ignore-- > 0) (void)adc_fifo_get_blocking();
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLES; i++) sum += adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
#if HADISTORY_HAS_CYW43 && CYW43_USES_VSYS_PIN
    cyw43_thread_exit();
#endif
    uint32_t raw = sum / SAMPLES;                    // 0 .. 4095 for 0 .. 3.3 V
    return (int)(raw * 3300 * 3 / 4095);
}

battery_t battery_read(void) {
    battery_t b;
    int vsys = read_vsys_mv();
    b.on_usb = vsys >= BATTERY_USB_MV;
    if (b.on_usb) {
        b.millivolts = 0;
        b.bars = BATTERY_BARS;
        printf("Battery: VSYS %d mV, on USB\n", vsys);
        return b;
    }
    b.millivolts = vsys + BATTERY_DIODE_DROP_MV;
    b.bars = 0;
    for (int i = 0; i < BATTERY_BARS; i++) {
        if (b.millivolts >= BAR_MV[i]) { b.bars = BATTERY_BARS - i; break; }
    }
    printf("Battery: VSYS %d mV, cell about %d mV, %d/%d bars\n", vsys, b.millivolts, b.bars, BATTERY_BARS);
    return b;
}
