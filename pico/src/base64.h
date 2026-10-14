/* Streaming base64 decoder: bytes arrive in arbitrary pieces, decoded output goes to a buffer. */
#ifndef HADISTORY_BASE64_H
#define HADISTORY_BASE64_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *out;
    size_t out_cap;
    size_t out_len;
    uint32_t acc;      // pending sextets
    int acc_bits;
    bool overflow;
} base64_decoder_t;

void base64_init(base64_decoder_t *d, uint8_t *out, size_t out_cap);
// Feeds text; whitespace and '=' are ignored. Returns false once the output overflowed.
bool base64_feed(base64_decoder_t *d, const char *text, size_t len);

#endif
