#include "https.h"
#include "hadistory_config.h"
#include "openai_roots.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "mbedtls/ssl.h"

#define HEADER_MAX 2048

enum chunk_state { CH_SIZE, CH_DATA, CH_CRLF, CH_DONE };

struct client_t {
    struct altcp_pcb *pcb;
    const char *host;
    char *request;
    size_t request_len;
    size_t sent;
    https_sink_t sink;
    void *ctx;
    int idle_timeout_s;

    // response
    bool headers_done;
    char hdr[HEADER_MAX];
    size_t hdr_len;
    int status;
    bool chunked;
    long content_length;     // -1 when unknown
    size_t body_len;
    enum chunk_state chstate;
    long chunk_remaining;
    char sizebuf[16];
    int sizelen;

    volatile bool complete;
    int error;
};

static struct altcp_tls_config *tls_config = NULL;

// ---------------------------------------------------------------------------
// Connection teardown
// ---------------------------------------------------------------------------
static err_t client_close(client_t *c) {
    err_t err = ERR_OK;
    if (c->pcb) {
        altcp_arg(c->pcb, NULL);
        altcp_poll(c->pcb, NULL, 0);
        altcp_recv(c->pcb, NULL);
        altcp_sent(c->pcb, NULL);
        altcp_err(c->pcb, NULL);
        err = altcp_close(c->pcb);
        if (err != ERR_OK) {
            altcp_abort(c->pcb);
            err = ERR_ABRT;
        }
        c->pcb = NULL;
    }
    c->complete = true;
    return err;
}

// ---------------------------------------------------------------------------
// Response parsing
// ---------------------------------------------------------------------------
static bool header_find(const char *hdr, const char *name, const char **value) {
    size_t n = strlen(name);
    for (const char *p = hdr; *p; p++) {
        if (strncasecmp(p, name, n) == 0 && p[n] == ':') {
            p += n + 1;
            while (*p == ' ') p++;
            *value = p;
            return true;
        }
        p = strchr(p, '\n');
        if (!p) break;
    }
    return false;
}

static void deliver(client_t *c, const uint8_t *data, size_t len) {
    if (len == 0) return;
    if (c->sink) c->sink(data, len, c->ctx);
    c->body_len += len;
}

static void feed_body(client_t *c, const uint8_t *data, size_t len) {
    if (!c->chunked) {
        deliver(c, data, len);
        if (c->content_length >= 0 && (long)c->body_len >= c->content_length) {
            client_close(c);
        }
        return;
    }
    while (len > 0 && c->chstate != CH_DONE) {
        switch (c->chstate) {
        case CH_SIZE: {
            char ch = (char)*data++; len--;
            if (ch == '\n') {
                c->sizebuf[c->sizelen] = '\0';
                c->chunk_remaining = strtol(c->sizebuf, NULL, 16);
                c->sizelen = 0;
                c->chstate = c->chunk_remaining == 0 ? CH_DONE : CH_DATA;
            } else if (isxdigit((unsigned char)ch) && c->sizelen < (int)sizeof c->sizebuf - 1) {
                c->sizebuf[c->sizelen++] = ch;   // '\r' and chunk extensions are ignored
            }
            break;
        }
        case CH_DATA: {
            size_t n = len < (size_t)c->chunk_remaining ? len : (size_t)c->chunk_remaining;
            deliver(c, data, n);
            data += n; len -= n; c->chunk_remaining -= n;
            if (c->chunk_remaining == 0) c->chstate = CH_CRLF;
            break;
        }
        case CH_CRLF: {
            char ch = (char)*data++; len--;
            if (ch == '\n') c->chstate = CH_SIZE;
            break;
        }
        default:
            break;
        }
    }
    if (c->chstate == CH_DONE) client_close(c);
}

static void feed(client_t *c, const uint8_t *data, size_t len) {
    if (c->headers_done) {
        feed_body(c, data, len);
        return;
    }
    while (len > 0) {
        if (c->hdr_len >= HEADER_MAX - 1) {
            puts("https: response headers too long");
            c->error = -4;
            client_close(c);
            return;
        }
        c->hdr[c->hdr_len++] = (char)*data++; len--;
        if (c->hdr_len >= 4 && memcmp(c->hdr + c->hdr_len - 4, "\r\n\r\n", 4) == 0) {
            c->hdr[c->hdr_len] = '\0';
            c->headers_done = true;
            c->status = (int)strtol(c->hdr + 9, NULL, 10);   // "HTTP/1.1 200 ..."
            const char *v;
            if (header_find(c->hdr, "Transfer-Encoding", &v) && strncasecmp(v, "chunked", 7) == 0) {
                c->chunked = true;
            }
            if (header_find(c->hdr, "Content-Length", &v)) {
                c->content_length = strtol(v, NULL, 10);
            }
            printf("https: status %d, %s\n", c->status,
                   c->chunked ? "chunked" : (c->content_length >= 0 ? "content-length" : "until close"));
            if (len > 0) feed_body(c, data, len);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// lwIP callbacks
// ---------------------------------------------------------------------------
static void send_more(client_t *c) {
    while (c->sent < c->request_len) {
        u16_t room = altcp_sndbuf(c->pcb);
        if (room == 0) break;
        size_t n = c->request_len - c->sent;
        if (n > room) n = room;
        bool more = c->sent + n < c->request_len;
        err_t err = altcp_write(c->pcb, c->request + c->sent, n,
                                TCP_WRITE_FLAG_COPY | (more ? TCP_WRITE_FLAG_MORE : 0));
        if (err != ERR_OK) {
            printf("https: write error %d\n", err);
            c->error = -5;
            client_close(c);
            return;
        }
        c->sent += n;
    }
    altcp_output(c->pcb);
}

static err_t on_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    client_t *c = (client_t *)arg;
    if (err != ERR_OK) {
        printf("https: connect failed %d\n", err);
        c->error = -2;
        return client_close(c);
    }
    puts("https: connected, sending request");
    send_more(c);
    return ERR_OK;
}

static err_t on_sent(void *arg, struct altcp_pcb *pcb, u16_t len) {
    client_t *c = (client_t *)arg;
    if (c->sent < c->request_len) send_more(c);
    return ERR_OK;
}

static err_t on_poll(void *arg, struct altcp_pcb *pcb) {
    client_t *c = (client_t *)arg;
    puts("https: timed out waiting for the server");
    c->error = -3;
    return client_close(c);
}

static void on_err(void *arg, err_t err) {
    client_t *c = (client_t *)arg;
    printf("https: connection error %d\n", err);
    c->pcb = NULL;   // already freed by lwIP
    c->error = -2;
    c->complete = true;
}

static err_t on_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    client_t *c = (client_t *)arg;
    if (!p) {
        // Remote close: the end of a "Connection: close" response
        if (!c->headers_done) c->error = -6;
        return client_close(c);
    }
    for (struct pbuf *q = p; q != NULL; q = q->next) {
        feed(c, (const uint8_t *)q->payload, q->len);
        if (c->complete) break;
    }
    if (c->pcb) altcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static void connect_ip(const ip_addr_t *ip, client_t *c) {
    err_t err = altcp_connect(c->pcb, ip, 443, on_connected);
    if (err != ERR_OK) {
        printf("https: connect start failed %d\n", err);
        c->error = -2;
        client_close(c);
    }
}

static void on_dns(const char *hostname, const ip_addr_t *ip, void *arg) {
    client_t *c = (client_t *)arg;
    if (ip) {
        connect_ip(ip, c);
    } else {
        printf("https: cannot resolve %s\n", hostname);
        c->error = -1;
        client_close(c);
    }
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
bool https_post_json(const char *host, const char *path, const char *bearer_token,
                     const char *json_body, https_sink_t sink, void *ctx,
                     int idle_timeout_s, https_result_t *result) {
    memset(result, 0, sizeof *result);

    if (!tls_config) {
#if HADISTORY_TLS_NO_VERIFY
        tls_config = altcp_tls_create_config_client(NULL, 0);
#else
        tls_config = altcp_tls_create_config_client((const u8_t *)OPENAI_ROOT_CERTS, sizeof OPENAI_ROOT_CERTS);
#endif
        if (!tls_config) {
            puts("https: TLS config failed");
            result->error = -7;
            return false;
        }
    }

    client_t *c = (client_t *)calloc(1, sizeof *c);
    if (!c) { result->error = -8; return false; }
    c->host = host;
    c->sink = sink;
    c->ctx = ctx;
    c->idle_timeout_s = idle_timeout_s;
    c->content_length = -1;

    size_t body_len = strlen(json_body);
    c->request_len = strlen(path) + strlen(host) + strlen(bearer_token) + body_len + 256;
    c->request = (char *)malloc(c->request_len);
    if (!c->request) { free(c); result->error = -8; return false; }
    c->request_len = snprintf(c->request, c->request_len,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Authorization: Bearer %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        path, host, bearer_token, (unsigned)body_len, json_body);

    cyw43_arch_lwip_begin();
    c->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!c->pcb) {
        cyw43_arch_lwip_end();
        puts("https: cannot create TLS pcb");
        free(c->request); free(c);
        result->error = -8;
        return false;
    }
    altcp_arg(c->pcb, c);
    // The poll interval counts in 500 ms units on 8 bits: 127 s at most
    int poll_ticks = idle_timeout_s * 2;
    if (poll_ticks > 255) poll_ticks = 255;
    altcp_poll(c->pcb, on_poll, (u8_t)poll_ticks);
    altcp_recv(c->pcb, on_recv);
    altcp_sent(c->pcb, on_sent);
    altcp_err(c->pcb, on_err);
    mbedtls_ssl_set_hostname((mbedtls_ssl_context *)altcp_tls_context(c->pcb), host);

    printf("https: resolving %s\n", host);
    ip_addr_t ip;
    err_t err = dns_gethostbyname(host, &ip, on_dns, c);
    if (err == ERR_OK) {
        connect_ip(&ip, c);
    } else if (err != ERR_INPROGRESS) {
        printf("https: DNS error %d\n", err);
        c->error = -1;
        client_close(c);
    }
    cyw43_arch_lwip_end();

    // Wait for completion; the work happens in the lwIP background context
    while (!c->complete) {
        sleep_ms(10);
    }

    result->status = c->status;
    result->error = c->error;
    result->body_len = c->body_len;
    free(c->request);
    free(c);
    return result->error == 0 && result->status != 0;
}
