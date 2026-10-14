/* Skeleton. Planned pipeline, mirroring generate_page() in hadistory.py:
 *   1. build the prompt from prompts/prompts.txt (weighted lines, bracket choices)
 *   2. POST /v1/chat/completions (gpt-4o-mini) -> story text
 *   3. POST /v1/images/generations (jpeg output) -> stream to storage
 *   4. jpegdec into display_graphics() (PicoGraphics dithers to the 7 colours),
 *      text with a PicoGraphics font
 *   5. display_update()
 */
#include "gpt.h"
#include "hadistory_config.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

bool gpt_init(void) {
    // cyw43_arch_init() is done once in main()
    cyw43_arch_enable_sta_mode();
    printf("WiFi: connecting to %s ...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        puts("WiFi: connection failed");
        return false;
    }
    puts("WiFi: connected");
    return true;
}

bool gpt_generate_page(void) {
    puts("AI mode: not implemented yet");
    return false;
}
