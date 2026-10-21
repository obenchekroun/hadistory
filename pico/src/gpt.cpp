/* AI mode, port of generate_page() in pi/hadistory.py (online mode).
 * Step 1 (done): prompt from prompts.txt, story text from the chat API, printed on the console.
 * Step 2 (done): text rendered on the display, same layout as the story pages.
 * Step 3 (done): illustration from the image API, base64 decoded on the fly into the
 *                shared image buffer, then JPEG decoded and dithered into the display.
 */
#include "gpt.h"
#include "hadistory_config.h"
#include "https.h"
#include "json.h"
#include "prompt.h"
#include "render.h"
#include "display.h"
#include "base64.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

static char story_text[STORY_MAX_LEN];
static size_t last_image_len = 0;   // JPEG bytes in the shared image buffer after get_image()

// Accumulates a response body into a fixed buffer (text answers are a few KB)
struct accumulator_t {
    char *buf;
    size_t len;
    size_t cap;
};

static void accumulate(const uint8_t *data, size_t len, void *ctx) {
    accumulator_t *a = (accumulator_t *)ctx;
    size_t room = a->cap - a->len;
    if (len > room) len = room;
    memcpy(a->buf + a->len, data, len);
    a->len += len;
    a->buf[a->len] = '\0';
}

static void print_elapsed(const char *what, absolute_time_t t0) {
    int64_t s = absolute_time_diff_us(t0, get_absolute_time()) / 1000000;
    printf("%s in %02lldh%02lldm%02llds\n", what, s / 3600, (s % 3600) / 60, s % 60);
}

// ---------------------------------------------------------------------------
// WiFi, non blocking: the join runs in the cyw43 driver, the main loop polls the link
// ---------------------------------------------------------------------------
static wifi_state_t wifi_state = WIFI_OFF;
static absolute_time_t wifi_deadline;

const char *gpt_wifi_state_name(wifi_state_t s) {
    switch (s) {
    case WIFI_CONNECTING: return "connecting";
    case WIFI_UP: return "up";
    case WIFI_FAILED: return "failed";
    default: return "off";
    }
}

void gpt_init(void) {
    // cyw43_arch_init() is done once in main()
    cyw43_arch_enable_sta_mode();
}

void gpt_wifi_connect(void) {
    if (wifi_state == WIFI_CONNECTING || wifi_state == WIFI_UP) return;
    printf("WiFi: connecting to %s ...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK)) {
        puts("WiFi: cannot start the connection");
        wifi_state = WIFI_FAILED;
        return;
    }
    wifi_state = WIFI_CONNECTING;
    wifi_deadline = make_timeout_time_ms(WIFI_CONNECT_TIMEOUT_MS);
}

static int link_status(void) {
    cyw43_arch_lwip_begin();
    int st = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    cyw43_arch_lwip_end();
    return st;
}

wifi_state_t gpt_wifi_poll(void) {
    if (wifi_state == WIFI_CONNECTING) {
        int st = link_status();
        if (st == CYW43_LINK_UP) {
            wifi_state = WIFI_UP;
            puts("WiFi: connected");
        } else if (st < 0 || time_reached(wifi_deadline)) {
            wifi_state = WIFI_FAILED;
            printf("WiFi: connection failed (%s)\n",
                   st == CYW43_LINK_NONET ? "network not found" :
                   st == CYW43_LINK_BADAUTH ? "bad password" :
                   st == CYW43_LINK_FAIL ? "join failed" : "timeout");
        }
    } else if (wifi_state == WIFI_UP) {
        if (link_status() != CYW43_LINK_UP) {
            wifi_state = WIFI_OFF;
            puts("WiFi: link lost");
        }
    }
    return wifi_state;
}

bool gpt_wifi_up(void) {
    return gpt_wifi_poll() == WIFI_UP;
}

static bool get_story(const char *prompt) {
    static char escaped[2048];
    static char body[2400];
    static char response[8192];

    json_escape(prompt, escaped, sizeof escaped);
    snprintf(body, sizeof body,
             "{\"model\":\"%s\",\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
             GPT_MODEL, escaped);

    accumulator_t acc = { response, 0, sizeof response - 1 };
    https_result_t res;
    if (!https_post_json(OPENAI_HOST, "/v1/chat/completions", OPENAI_API_KEY, body,
                         accumulate, &acc, GPT_TIMEOUT_S, &res)) {
        printf("Chat request failed (error %d, status %d)\n", res.error, res.status);
        return false;
    }
    if (res.status != 200) {
        printf("Chat request: HTTP %d\n%.300s\n", res.status, response);
        return false;
    }
    if (!json_extract_string(response, acc.len, "content", story_text, sizeof story_text)) {
        printf("Chat request: no content in the answer\n%.300s\n", response);
        return false;
    }
    char finish[32] = "?";
    json_extract_string(response, acc.len, "finish_reason", finish, sizeof finish);
    printf("Chat request: %u bytes received, finish_reason=%s\n", (unsigned)acc.len, finish);
    for (char *p = story_text; *p; p++) {
        if (*p == '\n') *p = ' ';   // one paragraph, as hadistory.py does
    }
    return true;
}

// ---------------------------------------------------------------------------
// Image: the response is {"created":..,"data":[{"b64_json":"<base64 jpeg>"}],..}
// The base64 string is decoded as it streams in, the rest of the body is skipped.
// ---------------------------------------------------------------------------
static const char B64_KEY[] = "\"b64_json\"";

struct image_sink_t {
    int matched;             // characters of B64_KEY matched so far
    bool key_found;          // key seen, waiting for the opening quote of the value
    bool in_data;            // inside the base64 string
    bool done;
    base64_decoder_t b64;
    char head[512];          // beginning of the body, for error messages
    size_t head_len;
};

static void image_sink(const uint8_t *data, size_t len, void *ctx) {
    image_sink_t *s = (image_sink_t *)ctx;
    if (s->head_len < sizeof s->head - 1) {
        size_t n = sizeof s->head - 1 - s->head_len;
        if (n > len) n = len;
        memcpy(s->head + s->head_len, data, n);
        s->head_len += n;
        s->head[s->head_len] = '\0';
    }
    size_t i = 0;
    while (i < len && !s->done) {
        if (!s->in_data) {
            // Look for the key, one character at a time, across chunk boundaries,
            // then for the opening quote of its value (the API may indent: "b64_json": "...)
            char c = (char)data[i++];
            if (s->key_found) {
                if (c == '"') s->in_data = true;
                else if (c != ':' && c != ' ' && c != '\n' && c != '\r' && c != '\t') s->key_found = false;
            } else if (c == B64_KEY[s->matched]) {
                s->matched++;
                if (B64_KEY[s->matched] == '\0') { s->key_found = true; s->matched = 0; }
            } else {
                s->matched = (c == B64_KEY[0]) ? 1 : 0;
            }
        } else {
            size_t start = i;
            while (i < len && data[i] != '"') i++;
            base64_feed(&s->b64, (const char *)data + start, i - start);
            if (i < len) s->done = true;   // closing quote
        }
    }
}

static bool get_image(const char *story) {
    static char image_prompt[STORY_MAX_LEN + 512];
    static char escaped[2 * sizeof image_prompt];
    static char body[sizeof escaped + 256];

    snprintf(image_prompt, sizeof image_prompt, "%s%s", PROMPT_IMAGE_INCIPIT, story);
    json_escape(image_prompt, escaped, sizeof escaped);
    snprintf(body, sizeof body,
             "{\"model\":\"%s\",\"prompt\":\"%s\",\"size\":\"1024x1024\",\"quality\":\"%s\","
             "\"output_format\":\"jpeg\",\"output_compression\":%d,\"n\":1}",
             IMAGE_MODEL, escaped, IMAGE_QUALITY, IMAGE_COMPRESSION);

    size_t cap;
    uint8_t *buf = render_image_buffer(&cap);
    image_sink_t sink;
    memset(&sink, 0, sizeof sink);
    base64_init(&sink.b64, buf, cap);

    https_result_t res;
    if (!https_post_json(OPENAI_HOST, "/v1/images/generations", OPENAI_API_KEY, body,
                         image_sink, &sink, IMAGE_TIMEOUT_S, &res)) {
        printf("Image request failed (error %d, status %d)\n", res.error, res.status);
        return false;
    }
    if (res.status != 200) {
        printf("Image request: HTTP %d\n%s\n", res.status, sink.head);
        return false;
    }
    if (!sink.in_data || sink.b64.out_len == 0) {
        printf("Image request: no b64_json in the answer\n%s\n", sink.head);
        return false;
    }
    if (sink.b64.overflow) {
        printf("Image request: image larger than the %u byte buffer, raise IMAGE_COMPRESSION\n", (unsigned)cap);
        return false;
    }
    last_image_len = sink.b64.out_len;
    printf("Image downloaded: %u bytes of JPEG (%u bytes received)\n", (unsigned)last_image_len, (unsigned)res.body_len);
    return true;
}

bool gpt_generate_page(void) {
    static char prompt[1024];

    if (!gpt_wifi_up()) {
        printf("AI mode: WiFi is %s, cannot generate\n", gpt_wifi_state_name(wifi_state));
        return false;
    }
    puts("Creating a new story...");
    if (!prompt_create(prompt, sizeof prompt)) return false;
    printf("Here is the prompt (%u chars) : %s\n", (unsigned)strlen(prompt), prompt);

    absolute_time_t t0 = get_absolute_time();
    if (!get_story(prompt)) return false;
    printf("Here is a story (%u chars): \n", (unsigned)strlen(story_text));
    puts(story_text);
    print_elapsed("Story generated", t0);

    puts("Creating the image, may take a while ...");
    t0 = get_absolute_time();
    bool have_image = get_image(story_text);
    print_elapsed(have_image ? "Image generated" : "Image step failed", t0);

    puts("Showing image ...");
    t0 = get_absolute_time();
    render_layout_t layout = render_text(story_text);
    printf("Layout: %d lines, illustration square of %d px\n", layout.lines, layout.image_size);
    if (have_image) {
        size_t cap;
        uint8_t *buf = render_image_buffer(&cap);
        render_jpeg(layout, buf, last_image_len);   // the download filled the shared buffer
    }
    render_mode_badge();
    render_battery();
    display_update();
    print_elapsed("Image shown", t0);
    puts("The end.");
    return have_image;
}
