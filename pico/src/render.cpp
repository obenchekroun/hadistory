#include "render.h"
#include "hadistory_config.h"
#include "display.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <string.h>
#include "JPEGDEC.h"

using namespace pimoroni;

// Portrait page geometry, as in pi/hadistory.py
static const int PAGE_W = DISPLAY_HEIGHT;   // 448
static const int PAGE_H = DISPLAY_WIDTH;    // 600
static const int TEXT_X = 7;                // left margin of the text
static const int TEXT_GAP = 2;              // gap between the illustration and the text
static const int ACCENT_ROOM = 8;           // bitmap fonts draw accents above the line
static const float TEXT_SCALE = 2.0f;       // bitmap8 at scale 2: 16 px letters
static const int LINE_H = (8 + 1) * 2;      // line pitch used by PicoGraphics for bitmap8 at scale 2

static uint8_t image_buffer[IMAGE_MAX_BYTES];
static JPEGDEC jpeg;

uint8_t *render_image_buffer(size_t *capacity) {
    if (capacity) *capacity = sizeof image_buffer;
    return image_buffer;
}

// Portrait (x, y) -> landscape panel (X, Y), a 90 degree counter-clockwise rotation,
// the same transform as Image.rotate(90, expand=1) in the Pi version
static inline Point to_panel(int x, int y) {
    return Point(y, PAGE_W - 1 - x);
}

// The bitmap fonts know ASCII and the Latin-1 accented letters (UTF-8 lead bytes 0xC2 and
// 0xC3) only. Other common characters of French text are mapped to what they can show.
static std::string normalise(const char *text) {
    static const struct { const char *from; const char *to; } map[] = {
        { "\xE2\x80\x99", "'" },   // ’ right single quotation mark (typographic apostrophe)
        { "\xE2\x80\x98", "'" },   // ‘
        { "\xE2\x80\x9C", "\"" },  // “
        { "\xE2\x80\x9D", "\"" },  // ”
        { "\xE2\x80\x93", "-" },   // – en dash
        { "\xE2\x80\x94", "-" },   // — em dash
        { "\xE2\x80\xA6", "..." }, // … ellipsis
        { "\xC5\x93", "oe" },       // œ
        { "\xC5\x92", "OE" },       // Œ
        { "\xC2\xA0", " " },        // no-break space
    };
    std::string out;
    for (const char *p = text; *p; ) {
        bool replaced = false;
        for (const auto &m : map) {
            size_t n = strlen(m.from);
            if (strncmp(p, m.from, n) == 0) { out += m.to; p += n; replaced = true; break; }
        }
        if (replaced) continue;
        unsigned char c = (unsigned char)*p;
        if (c >= 0x80 && c != 0xC2 && c != 0xC3 && !(c >= 0x80 && c <= 0xBF)) {
            // Lead byte of a sequence the font cannot show: skip the whole sequence
            int n = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : 2;
            out += '?';
            p += n;
            continue;
        }
        out += (char)c;
        p++;
    }
    return out;
}

// Greedy word wrap with the real glyph widths, like wrap_text_display() in hadistory.py
static std::vector<std::string> wrap(PicoGraphics &g, const char *raw_text, int width) {
    std::string text = normalise(raw_text);
    std::vector<std::string> lines;
    std::string line, word;
    const char *p = text.c_str();
    for (;;) {
        char c = *p;
        if (c == ' ' || c == '\n' || c == '\r' || c == '\0') {
            if (!word.empty()) {
                std::string candidate = line.empty() ? word : line + " " + word;
                if (!line.empty() && g.measure_text(candidate, TEXT_SCALE) > width) {
                    lines.push_back(line);
                    line = word;
                } else {
                    line = candidate;
                }
                word.clear();
            }
            if (c == '\n') { lines.push_back(line); line.clear(); }
            if (c == '\0') break;
        } else {
            word += c;
        }
        p++;
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

render_layout_t render_text(const char *text) {
    PicoGraphics &g = display_graphics();
    g.set_font("bitmap8");

    std::vector<std::string> lines = wrap(g, text, PAGE_W - 2 * TEXT_X);
    int text_height = ACCENT_ROOM + (int)lines.size() * LINE_H + TEXT_GAP;

    render_layout_t layout;
    layout.lines = (int)lines.size();
    layout.image_size = (PAGE_H - text_height >= PAGE_W) ? PAGE_W : PAGE_H - text_height;
    layout.image_offset = (PAGE_W - layout.image_size) / 2;

    display_clear(DISPLAY_WHITE);
    g.set_pen(DISPLAY_BLACK);
    int y = layout.image_size + TEXT_GAP + ACCENT_ROOM;   // portrait y of the first line
    for (const std::string &line : lines) {
        // Rotation 270: glyphs advance towards the top of the panel, lines towards the right
        g.text(line, to_panel(TEXT_X, y), 100000, TEXT_SCALE, 270.0f);
        y += LINE_H;
    }
    return layout;
}

void render_page_counter(int page, int total) {
    PicoGraphics &g = display_graphics();
    g.set_font("bitmap8");
    char counter[16];
    snprintf(counter, sizeof counter, "%d/%d", page, total);
    // As in hadistory.py: text at portrait (20, 20) on a white box with a 5 px margin
    const int x = 20, y = 20, h = 16;
    int w = g.measure_text(counter, TEXT_SCALE);
    // Portrait box (x-5, y-5) to (x+w+5, y+h+5) -> panel: X from y-5, Y from 447-(x+w+5)
    g.set_pen(DISPLAY_WHITE);
    g.rectangle(Rect(y - 5, PAGE_W - 1 - (x + w + 5), h + 10, w + 10));
    g.set_pen(DISPLAY_BLACK);
    g.text(counter, to_panel(x, y), 100000, TEXT_SCALE, 270.0f);
}

// ---------------------------------------------------------------------------
// JPEG: decoded blocks are resampled into the square, then dithered with Floyd-Steinberg
// error diffusion, the method of the Pi version. The decoder delivers the image as
// strips of blocks (one row of MCUs at a time, left to right); each strip is resampled
// into a small RGB buffer in target coordinates and dithered line by line, the error
// of its last line being carried into the next strip, so the result is the same as a
// full-image scan without holding the full image in memory.
// ---------------------------------------------------------------------------

// Panel colours as the inky library sees them for dithering: its saturated and
// desaturated palettes blended at 0.5, the default used by the Pi version and the tool
static const int PALETTE[7][3] = {
    { 28,  24,  28},   // black
    {255, 255, 255},   // white
    { 29, 173,  35},   // green
    { 30,  29, 174},   // blue
    {205,  36,  37},   // red
    {231, 222,  35},   // yellow
    {216, 123,  36},   // orange
};

static const int MAX_STRIP_ROWS = 24;   // target rows covered by one strip of MCUs (16 rows at most, upscaled a little)

struct jpeg_ctx_t {
    const render_layout_t *layout;
    int src_w, src_h;            // decoded (scaled) image size
    uint8_t *strip;              // MAX_STRIP_ROWS x image_size x RGB, in target coordinates
    int strip_ty;                // first target row of the strip, -1 when empty
    int strip_rows;
    int16_t *err_cur, *err_next; // diffusion errors, (image_size + 2) x 3 each
};

static uint8_t strip_buffer[MAX_STRIP_ROWS * DISPLAY_HEIGHT * 3];
static int16_t err_a[(DISPLAY_HEIGHT + 2) * 3], err_b[(DISPLAY_HEIGHT + 2) * 3];

static int nearest_colour(int r, int g, int b) {
    int best = 0, best_d = 0x7fffffff;
    for (int i = 0; i < 7; i++) {
        int dr = r - PALETTE[i][0], dg = g - PALETTE[i][1], db = b - PALETTE[i][2];
        int d = dr * dr + dg * dg + db * db;
        if (d < best_d) { best_d = d; best = i; }
    }
    return best;
}

// Dither and draw the rows of the current strip, top to bottom
static void flush_strip(jpeg_ctx_t *ctx) {
    if (ctx->strip_ty < 0) return;
    PicoGraphics &g = display_graphics();
    const int S = ctx->layout->image_size;
    for (int row = 0; row < ctx->strip_rows; row++) {
        const uint8_t *px = ctx->strip + row * S * 3;
        int16_t *cur = ctx->err_cur, *next = ctx->err_next;
        memset(next, 0, (size_t)(S + 2) * 3 * sizeof(int16_t));
        int ty = ctx->strip_ty + row;
        for (int x = 0; x < S; x++) {
            int c[3];
            int idx;
            {
                int r = px[x * 3 + 0] + cur[(x + 1) * 3 + 0];
                int gr = px[x * 3 + 1] + cur[(x + 1) * 3 + 1];
                int b = px[x * 3 + 2] + cur[(x + 1) * 3 + 2];
                idx = nearest_colour(r, gr, b);
                c[0] = r - PALETTE[idx][0];
                c[1] = gr - PALETTE[idx][1];
                c[2] = b - PALETTE[idx][2];
            }
            g.set_pen((uint)idx);
            g.set_pixel(to_panel(ctx->layout->image_offset + x, ty));
            // Floyd-Steinberg: 7/16 right, 3/16 below left, 5/16 below, 1/16 below right
            for (int k = 0; k < 3; k++) {
                int e = c[k];
                cur[(x + 2) * 3 + k]  += (int16_t)((e * 7) / 16);
                next[(x + 0) * 3 + k] += (int16_t)((e * 3) / 16);
                next[(x + 1) * 3 + k] += (int16_t)((e * 5) / 16);
                next[(x + 2) * 3 + k] += (int16_t)(e / 16);
            }
        }
        int16_t *t = ctx->err_cur; ctx->err_cur = ctx->err_next; ctx->err_next = t;
    }
    ctx->strip_ty = -1;
    ctx->strip_rows = 0;
}

static int jpeg_draw(JPEGDRAW *d) {
    jpeg_ctx_t *ctx = (jpeg_ctx_t *)d->pUser;
    const int S = ctx->layout->image_size;
    const int W = ctx->src_w, H = ctx->src_h;

    // Target pixels covered by this block: [x*S/W, (x+w)*S/W) and the same for y
    int tx0 = d->x * S / W, tx1 = (d->x + d->iWidthUsed) * S / W;
    int ty0 = d->y * S / H, ty1 = (d->y + d->iHeight) * S / H;
    if (tx1 > S) tx1 = S;
    if (ty1 > S) ty1 = S;
    if (ty1 - ty0 > MAX_STRIP_ROWS) ty1 = ty0 + MAX_STRIP_ROWS;

    if (ctx->strip_ty != ty0) {
        flush_strip(ctx);                     // a new row of MCUs begins
        ctx->strip_ty = ty0;
        ctx->strip_rows = ty1 - ty0;
        memset(ctx->strip, 255, (size_t)ctx->strip_rows * S * 3);
    }
    for (int ty = ty0; ty < ty1; ty++) {
        int sy = ty * H / S - d->y;           // source row inside the block
        if (sy < 0) sy = 0;
        if (sy >= d->iHeight) sy = d->iHeight - 1;
        const uint16_t *row = d->pPixels + sy * d->iWidth;
        uint8_t *out = ctx->strip + ((ty - ctx->strip_ty) * S + tx0) * 3;
        for (int tx = tx0; tx < tx1; tx++) {
            int sx = tx * W / S - d->x;
            if (sx < 0) sx = 0;
            if (sx >= d->iWidthUsed) sx = d->iWidthUsed - 1;
            uint16_t p = row[sx];             // RGB565
            *out++ = (uint8_t)(((p >> 11) & 0x1F) * 255 / 31);
            *out++ = (uint8_t)(((p >> 5) & 0x3F) * 255 / 63);
            *out++ = (uint8_t)((p & 0x1F) * 255 / 31);
        }
    }
    return 1;
}

bool render_jpeg(const render_layout_t &layout, const uint8_t *data, size_t len) {
    if (!jpeg.openRAM((uint8_t *)data, (int)len, jpeg_draw)) {
        printf("JPEG: cannot open (error %d)\n", jpeg.getLastError());
        return false;
    }
    int w = jpeg.getWidth(), h = jpeg.getHeight();

    // Decode at the largest reduction that still covers the square
    int options = 0, div = 1;
    int longest = w > h ? w : h;
    if (longest / 8 >= layout.image_size)      { options = JPEG_SCALE_EIGHTH;  div = 8; }
    else if (longest / 4 >= layout.image_size) { options = JPEG_SCALE_QUARTER; div = 4; }
    else if (longest / 2 >= layout.image_size) { options = JPEG_SCALE_HALF;    div = 2; }

    jpeg_ctx_t ctx;
    ctx.layout = &layout;
    ctx.src_w = w / div;
    ctx.src_h = h / div;
    ctx.strip = strip_buffer;
    ctx.strip_ty = -1;
    ctx.strip_rows = 0;
    ctx.err_cur = err_a;
    ctx.err_next = err_b;
    memset(err_a, 0, sizeof err_a);
    memset(err_b, 0, sizeof err_b);
    jpeg.setUserPointer(&ctx);
    jpeg.setPixelType(RGB565_LITTLE_ENDIAN);
    printf("JPEG: %d x %d, decoded at 1/%d into a %d px square, Floyd-Steinberg dithering\n", w, h, div, layout.image_size);
    int ok = jpeg.decode(0, 0, options);
    flush_strip(&ctx);                        // the last strip
    jpeg.close();
    if (!ok) printf("JPEG: decode failed (error %d)\n", jpeg.getLastError());
    return ok != 0;
}
