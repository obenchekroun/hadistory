/* WiFi for the AI mode (built with WITH_GPT=ON): several known networks, joined in order.
 *
 * The known networks are the ones built into the firmware (WIFI_NETWORKS, or WIFI_SSID /
 * WIFI_PASSWORD, in secrets.cmake: CMake writes them to the generated wifi_networks.h) followed
 * by the ones stored in flash at run time (wifi_store_add(), meant for a set-up portal; a stored
 * network with the same name as a built-in one replaces its password). A connection attempt
 * scans the air first, then joins the known networks the scan has seen, in the order of the list,
 * WIFI_JOIN_TIMEOUT_MS each; the ones not seen (hidden names) are only tried, blind and with
 * WIFI_CONNECT_TIMEOUT_MS, when none was seen. Everything runs in the background: the main loop
 * calls wifi_poll() and acts on the state.
 */
#ifndef HADISTORY_WIFI_H
#define HADISTORY_WIFI_H

#include <stdbool.h>

#define WIFI_SSID_MAX       32    // bytes, the 802.11 limit
#define WIFI_PASSWORD_MAX   63    // WPA2 passphrase

typedef enum {
    WIFI_OFF = 0,       // no connection attempted, or link lost
    WIFI_SCANNING,      // looking for the known networks
    WIFI_CONNECTING,    // joining one of them
    WIFI_UP,
    WIFI_FAILED,        // every candidate failed (timeout, not found, bad password)
} wifi_state_t;

void wifi_init(void);                 // station mode on, flash store read, no connection yet
void wifi_connect(void);              // start an attempt in the background (no-op while one runs or the link is up)
wifi_state_t wifi_poll(void);         // advance and return the state, logs the transitions
bool wifi_up(void);
const char *wifi_state_name(wifi_state_t s);
const char *wifi_ssid(void);          // network joined or being joined, "" when none

int  wifi_known_count(void);          // built-in plus stored networks (distinct names)
bool wifi_store_add(const char *ssid, const char *password);   // remember a network in flash (replaces a same-named one,
                                                               // the oldest goes when WIFI_STORE_MAX are stored)
bool wifi_store_forget(const char *ssid);                      // drop a stored network (not a built-in one)
void wifi_store_clear(void);                                   // drop them all

#endif
