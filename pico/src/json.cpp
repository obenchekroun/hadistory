#include "json.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t json_escape(const char *in, char *out, size_t out_size) {
    size_t o = 0;
    for (const unsigned char *p = (const unsigned char *)in; *p && o + 7 < out_size; p++) {
        switch (*p) {
        case '"':  out[o++] = '\\'; out[o++] = '"';  break;
        case '\\': out[o++] = '\\'; out[o++] = '\\'; break;
        case '\n': out[o++] = '\\'; out[o++] = 'n';  break;
        case '\r': out[o++] = '\\'; out[o++] = 'r';  break;
        case '\t': out[o++] = '\\'; out[o++] = 't';  break;
        default:
            if (*p < 0x20) {
                o += snprintf(out + o, out_size - o, "\\u%04x", *p);
            } else {
                out[o++] = (char)*p;   // UTF-8 bytes pass through
            }
        }
    }
    out[o] = '\0';
    return o;
}

static size_t put_utf8(unsigned cp, char *out, size_t room) {
    if (cp < 0x80) { if (room < 1) return 0; out[0] = (char)cp; return 1; }
    if (cp < 0x800) { if (room < 2) return 0; out[0] = (char)(0xC0 | (cp >> 6)); out[1] = (char)(0x80 | (cp & 0x3F)); return 2; }
    if (room < 3) return 0;
    out[0] = (char)(0xE0 | (cp >> 12)); out[1] = (char)(0x80 | ((cp >> 6) & 0x3F)); out[2] = (char)(0x80 | (cp & 0x3F));
    return 3;
}

bool json_extract_string(const char *json, size_t json_len, const char *key, char *out, size_t out_size) {
    char pattern[64];
    snprintf(pattern, sizeof pattern, "\"%s\":", key);
    const char *end = json + json_len;
    const char *p = json;
    const char *start = NULL;
    size_t plen = strlen(pattern);
    while (p + plen < end) {
        if (memcmp(p, pattern, plen) == 0) {
            const char *q = p + plen;
            while (q < end && (*q == ' ' || *q == '\n' || *q == '\r' || *q == '\t')) q++;
            if (q < end && *q == '"') { start = q + 1; break; }
        }
        p++;
    }
    if (!start) { out[0] = '\0'; return false; }

    size_t o = 0;
    for (p = start; p < end && *p != '"' && o + 4 < out_size; p++) {
        if (*p != '\\') { out[o++] = *p; continue; }
        p++;
        if (p >= end) break;
        switch (*p) {
        case 'n': out[o++] = '\n'; break;
        case 'r': out[o++] = '\r'; break;
        case 't': out[o++] = '\t'; break;
        case 'b': out[o++] = '\b'; break;
        case 'f': out[o++] = '\f'; break;
        case 'u': {
            if (p + 4 >= end) break;
            char hex[5] = { p[1], p[2], p[3], p[4], 0 };
            unsigned cp = (unsigned)strtoul(hex, NULL, 16);
            p += 4;
            if (cp >= 0xD800 && cp <= 0xDBFF && p + 6 < end && p[1] == '\\' && p[2] == 'u') {
                char hex2[5] = { p[3], p[4], p[5], p[6], 0 };
                unsigned lo = (unsigned)strtoul(hex2, NULL, 16);
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                p += 6;
                // 4-byte UTF-8 (emoji): skipped, the bitmap fonts cannot show them anyway
                break;
            }
            o += put_utf8(cp, out + o, out_size - o - 1);
            break;
        }
        default: out[o++] = *p; break;   // \" \\ \/
        }
    }
    out[o] = '\0';
    return true;
}
