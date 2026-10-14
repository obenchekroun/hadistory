/* Minimal blocking HTTPS client on lwIP altcp_tls (mbedtls), threadsafe_background mode.
 * One POST with a JSON body; the response body is streamed to a sink callback,
 * de-chunked if the server uses chunked transfer encoding. Used by the AI mode only.
 */
#ifndef HADISTORY_HTTPS_H
#define HADISTORY_HTTPS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Called from the lwIP context with each piece of the response body, in order.
typedef void (*https_sink_t)(const uint8_t *data, size_t len, void *ctx);

typedef struct {
    int status;          // HTTP status code, 0 if no response
    int error;           // 0 ok, otherwise a negative code (timeout, connection, TLS, ...)
    size_t body_len;     // bytes delivered to the sink
} https_result_t;

// Blocks until the response is complete or the connection fails.
// idle_timeout_s: abort when nothing has been received for that long.
bool https_post_json(const char *host, const char *path, const char *bearer_token,
                     const char *json_body, https_sink_t sink, void *ctx,
                     int idle_timeout_s, https_result_t *result);

#endif
