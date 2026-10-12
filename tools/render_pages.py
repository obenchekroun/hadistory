#!/usr/bin/env python3
"""
Pre-render hadistory pages for the Pico version.

Reads the shared stories (stories/<story>/txt/pN.txt + stories/<story>/img/pN.png),
lays out each page exactly like pi/hadistory.py does (illustration on top, text below,
page counter top-left), dithers the result to the 7-colour palette of the Inky
Impression 5.7" (UC8159) and writes one 4-bit indexed BMP per page, ready to be copied
to the microSD card of the Pico.

Output format (one file per page, <out>/stories/<story>/pN.bmp):
  - standard 4 bits per pixel indexed BMP, uncompressed, viewable on any computer
  - 600 x 448 pixels, landscape (the portrait layout is rotated 90 degrees
    counter-clockwise, as in pi/hadistory.py)
  - stored top-down (negative height in the header), rows left to right
  - 2 pixels per byte, first pixel in the high nibble, 300 bytes per row (no padding)
  - pixel values are UC8159 colour indices, sendable to the panel as-is:
      0 black, 1 white, 2 green, 3 blue, 4 red, 5 yellow, 6 orange
  - pixel data starts at byte 118 and is 600 * 448 / 2 = 134400 bytes long,
    identical to the raw frame the UC8159 expects

Usage:
  python3 tools/render_pages.py                 # all stories -> tools/out/
  python3 tools/render_pages.py --story story_3 # one story
  python3 tools/render_pages.py --help
"""

import argparse
import re
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageEnhance, ImageFont

REPO_ROOT = Path(__file__).resolve().parent.parent

# ---------------------------------------------------------------------------
# Layout, mirrored from pi/hadistory.py
# ---------------------------------------------------------------------------
CANVAS_SIZE = (448, 600)          # portrait canvas, rotated to 600x448 before display
TEXT_WIDTH = 448
TEXT_ORIGIN_X = 7
TEXT_GAP = 2
PAGE_COUNTER_ORIGIN = (20, 20)
FONT_FILE = REPO_ROOT / "ressources" / "CormorantGaramond-Regular.ttf"
FONT_SIZE = 18
IMAGE_SHARPNESS = 1.2             # omni-epd.ini [Image Enhancements] sharpness

# ---------------------------------------------------------------------------
# Inky Impression 5.7" (UC8159) palette, from pimoroni/inky inky_uc8159.py
# ---------------------------------------------------------------------------
BLACK, WHITE, GREEN, BLUE, RED, YELLOW, ORANGE = range(7)

DESATURATED_PALETTE = [
    [0, 0, 0], [255, 255, 255], [0, 255, 0], [0, 0, 255],
    [255, 0, 0], [255, 255, 0], [255, 140, 0],
]
SATURATED_PALETTE = [
    [57, 48, 57], [255, 255, 255], [58, 91, 70], [61, 59, 94],
    [156, 72, 75], [208, 190, 71], [177, 106, 73],
]

OUTPUT_WIDTH, OUTPUT_HEIGHT = 600, 448
ROW_BYTES = OUTPUT_WIDTH // 2                    # 300, already a multiple of 4
FRAME_BYTES = ROW_BYTES * OUTPUT_HEIGHT          # 134400

# ---------------------------------------------------------------------------
# 4 bpp BMP container
# ---------------------------------------------------------------------------
BMP_FILE_HEADER = 14
BMP_INFO_HEADER = 40
BMP_PALETTE_ENTRIES = 16                          # 4 bpp BMPs carry 16 entries
BMP_PIXEL_OFFSET = BMP_FILE_HEADER + BMP_INFO_HEADER + 4 * BMP_PALETTE_ENTRIES  # 118


def blend_palette(saturation):
    """Blend the saturated and desaturated palettes, like inky's _palette_blend()."""
    saturation = min(max(saturation, 0.0), 1.0)
    palette = []
    for sat, desat in zip(SATURATED_PALETTE, DESATURATED_PALETTE):
        palette.append([
            int(s * saturation + d * (1.0 - saturation)) for s, d in zip(sat, desat)
        ])
    return palette


def palette_image(palette):
    pal = Image.new("P", (1, 1))
    # Exactly 7 entries: padding the palette would let quantize() pick duplicate indices.
    pal.putpalette([c for rgb in palette for c in rgb])
    return pal


def wrap_text_display(text, width, font):
    """Verbatim port of wrap_text_display() in pi/hadistory.py."""
    text_lines = []
    text_line = []
    text = text.replace("\n", " [br] ")
    words = text.split()

    for word in words:
        if word == "[br]":
            text_lines.append(" ".join(text_line))
            text_line = []
            continue
        text_line.append(word)
        w = int(font.getlength(" ".join(text_line)))
        if w > width:
            text_line.pop()
            text_lines.append(" ".join(text_line))
            text_line = [word]

    if len(text_line) > 0:
        text_lines.append(" ".join(text_line))

    return text_lines


def natural_key(path):
    return [int(s) if s.isdigit() else s for s in re.split(r"(\d+)", path.name)]


def list_pages(story_dir):
    """Return [(page_number, txt_path, img_path), ...] sorted by page number."""
    txt_dir = story_dir / "txt"
    img_dir = story_dir / "img"
    pages = []
    for txt in sorted(txt_dir.glob("p*.txt"), key=natural_key):
        m = re.fullmatch(r"p(\d+)\.txt", txt.name)
        if not m:
            continue
        n = int(m.group(1))
        img = img_dir / f"p{n}.png"
        if not img.exists():
            print(f"  WARNING: {img} missing, page {n} skipped", file=sys.stderr)
            continue
        pages.append((n, txt, img))
    return pages


def render_page(txt_path, img_path, page_number, page_count, font, pal_img,
                sharpness=IMAGE_SHARPNESS):
    """Return the page as a 600x448 'P' image whose pixel values are UC8159 indices."""
    story_text = txt_path.read_text(encoding="utf-8").replace("\n", " ")
    lines = wrap_text_display(story_text, TEXT_WIDTH, font)
    text_height = 0
    for line in lines:
        left, top, right, bottom = font.getbbox(line)
        text_height += int((bottom - top) * 1.1)
    text_height += TEXT_GAP
    story_text = "\n".join(lines)

    canvas_w, canvas_h = CANVAS_SIZE
    if canvas_h - text_height >= canvas_w:
        sizing = canvas_w
    else:
        sizing = canvas_h - text_height

    # Illustration: resize, sharpen (as omni-epd does), dither to the 7-colour palette.
    canvas = Image.new("RGB", CANVAS_SIZE, "white")
    illustration = Image.open(img_path).convert("RGB")
    illustration = illustration.resize((sizing, sizing), Image.LANCZOS)
    if sharpness != 1.0:
        illustration = ImageEnhance.Sharpness(illustration).enhance(sharpness)
    center_x = (canvas_w - sizing) // 2
    canvas.paste(illustration, (center_x, 0))
    page = canvas.quantize(palette=pal_img, dither=Image.Dither.FLOYDSTEINBERG)

    # Text and page counter are drawn after dithering, directly in palette indices,
    # so glyph edges stay crisp black on white instead of dithered grey.
    draw = ImageDraw.Draw(page)
    draw.text((TEXT_ORIGIN_X, sizing + TEXT_GAP), story_text, font=font, fill=BLACK)

    counter = f"{page_number}/{page_count}"
    left, top, right, bottom = draw.textbbox(PAGE_COUNTER_ORIGIN, counter, font=font)
    draw.rectangle((left - 5, top - 5, right + 5, bottom + 5), fill=WHITE)
    draw.text(PAGE_COUNTER_ORIGIN, counter, font=font, fill=BLACK)

    page = page.rotate(90, expand=1)
    assert page.size == (OUTPUT_WIDTH, OUTPUT_HEIGHT), page.size
    return page


def pack_frame(page):
    """Pack a 600x448 'P' image into the UC8159 4-bit wire format (top row first)."""
    idx = np.asarray(page, dtype=np.uint8).flatten()
    if idx.max() > ORANGE:
        raise ValueError(f"palette index {idx.max()} out of range, dithering went wrong")
    packed = ((idx[0::2] << 4) & 0xF0) | (idx[1::2] & 0x0F)
    data = packed.astype(np.uint8).tobytes()
    assert len(data) == FRAME_BYTES, len(data)
    return data


def bmp_bytes(frame, display_palette=SATURATED_PALETTE):
    """Wrap a raw UC8159 frame into a top-down 4 bpp indexed BMP.

    The pixel data is written unchanged: a BMP at 4 bpp stores 2 pixels per byte,
    high nibble first, and a 600-pixel row is 300 bytes, already 4-byte aligned.
    A negative height makes the rows top-down, so the file is the raw frame plus a
    118-byte header. The palette only affects how computers display the file.
    """
    assert len(frame) == FRAME_BYTES
    file_size = BMP_PIXEL_OFFSET + FRAME_BYTES
    file_header = struct.pack("<2sIHHI", b"BM", file_size, 0, 0, BMP_PIXEL_OFFSET)
    info_header = struct.pack(
        "<IiiHHIIiiII",
        BMP_INFO_HEADER,          # header size
        OUTPUT_WIDTH,             # width
        -OUTPUT_HEIGHT,           # negative height: top-down rows
        1,                        # colour planes
        4,                        # bits per pixel
        0,                        # BI_RGB, uncompressed
        FRAME_BYTES,              # image size
        2835, 2835,               # pixels per metre (72 dpi), informative only
        BMP_PALETTE_ENTRIES,      # colours used
        0,                        # important colours (all)
    )
    palette = bytearray()
    for i in range(BMP_PALETTE_ENTRIES):
        r, g, b = display_palette[i] if i < len(display_palette) else (0, 0, 0)
        palette += bytes((b, g, r, 0))  # BMP palette entries are BGR0
    data = file_header + info_header + bytes(palette) + frame
    assert len(data) == file_size
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--stories", type=Path, default=REPO_ROOT / "stories",
                        help="stories folder (default: <repo>/stories)")
    parser.add_argument("--out", type=Path, default=REPO_ROOT / "tools" / "out",
                        help="output folder (default: <repo>/tools/out)")
    parser.add_argument("--story", action="append",
                        help="only render this story (repeatable)")
    parser.add_argument("--saturation", type=float, default=0.5,
                        help="palette saturation 0..1, as in the inky library (default 0.5)")
    parser.add_argument("--sharpness", type=float, default=IMAGE_SHARPNESS,
                        help=f"illustration sharpness factor (default {IMAGE_SHARPNESS})")
    args = parser.parse_args()

    if not FONT_FILE.exists():
        sys.exit(f"font not found: {FONT_FILE}")
    font = ImageFont.truetype(str(FONT_FILE), FONT_SIZE)
    pal_img = palette_image(blend_palette(args.saturation))

    story_dirs = sorted((d for d in args.stories.iterdir() if (d / "txt").is_dir()),
                        key=natural_key)
    if args.story:
        story_dirs = [d for d in story_dirs if d.name in args.story]
        missing = set(args.story) - {d.name for d in story_dirs}
        if missing:
            sys.exit(f"story not found: {', '.join(sorted(missing))}")

    total = 0
    for story_dir in story_dirs:
        pages = list_pages(story_dir)
        if not pages:
            print(f"{story_dir.name}: no pages, skipped")
            continue
        out_dir = args.out / "stories" / story_dir.name
        out_dir.mkdir(parents=True, exist_ok=True)
        print(f"{story_dir.name}: {len(pages)} pages")
        for n, txt, img in pages:
            page = render_page(txt, img, n, len(pages), font, pal_img, args.sharpness)
            (out_dir / f"p{n}.bmp").write_bytes(bmp_bytes(pack_frame(page)))
            total += 1

    size_mb = total * (BMP_PIXEL_OFFSET + FRAME_BYTES) / 1e6
    print(f"done: {total} pages, {size_mb:.1f} MB in {args.out}")


if __name__ == "__main__":
    main()
