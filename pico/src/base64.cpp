#include "base64.h"

void base64_init(base64_decoder_t *d, uint8_t *out, size_t out_cap) {
    d->out = out;
    d->out_cap = out_cap;
    d->out_len = 0;
    d->acc = 0;
    d->acc_bits = 0;
    d->overflow = false;
}

static int sextet(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+' || c == '-') return 62;   // '-' and '_' for the url-safe variant
    if (c == '/' || c == '_') return 63;
    return -1;
}

bool base64_feed(base64_decoder_t *d, const char *text, size_t len) {
    for (size_t i = 0; i < len; i++) {
        int v = sextet(text[i]);
        if (v < 0) continue;   // '=', whitespace, anything else
        d->acc = (d->acc << 6) | (uint32_t)v;
        d->acc_bits += 6;
        if (d->acc_bits >= 8) {
            d->acc_bits -= 8;
            uint8_t byte = (uint8_t)(d->acc >> d->acc_bits);
            d->acc &= (1u << d->acc_bits) - 1;
            if (d->out_len < d->out_cap) {
                d->out[d->out_len++] = byte;
            } else {
                d->overflow = true;
            }
        }
    }
    return !d->overflow;
}
