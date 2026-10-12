# hadistory — desktop tools

Scripts that run on a computer (not on the device), mainly to prepare content for the Pico version.

## `render_pages.py` — pre-render story pages for the Pico SD card

Reads the shared stories in `stories/` and produces one 4-bit BMP per page, laid out exactly like the Pi version (illustration on top, text below in Cormorant Garamond 18, page counter top-left), dithered to the 7 colours of the Inky Impression 5.7".

Requirements: Python 3, Pillow, numpy.

```bash
python3 tools/render_pages.py                  # all stories -> tools/out/
python3 tools/render_pages.py --story story_3  # a single story (repeatable)
python3 tools/render_pages.py --saturation 0.5 --sharpness 1.2
python3 tools/render_pages.py --help
```

Then copy `tools/out/stories/` to the root of the microSD card.

Rendering takes about 6 seconds for 97 pages on a laptop. `tools/out/` is ignored by git; it is rebuilt from `stories/`. The files open in any image viewer (Preview, Finder thumbnails), which is the only preview needed.

### Output format

One file per page, `stories/<story>/pN.bmp`, exactly 134 518 bytes. It is a standard uncompressed 4 bits per pixel indexed BMP, built so that its pixel data is byte for byte the raw frame the UC8159 controller expects:

| Offset | Size | Content |
|---|---|---|
| 0 | 14 | BMP file header |
| 14 | 40 | BITMAPINFOHEADER: 600 x **-448** (negative height = rows stored top-down), 4 bpp, BI_RGB |
| 54 | 64 | 16 palette entries (BGR0), the first 7 being the panel colours, display only |
| 118 | 134 400 | Pixel data: 448 rows of 300 bytes, top row first, 2 pixels per byte, first pixel in the high nibble |

Pixel values are UC8159 colour indices:

| Index | Colour |
|---|---|
| 0 | black |
| 1 | white |
| 2 | green |
| 3 | blue |
| 4 | red |
| 5 | yellow |
| 6 | orange |

Index 7 ("clean") is never used.

On the Pico, loading a page is `fseek(118)` then one `fread` of 134 400 bytes straight into the frame buffer.

### Rendering notes

- Palette and dithering follow the Pimoroni `inky` library: the saturated and desaturated palettes are blended with `--saturation` (default 0.5, the inky default), then Floyd–Steinberg dithering is applied. Other dithering algorithms may be added later as an option, to be compared on the real panel.
- The illustration gets the same sharpness factor as `pi/omni-epd.ini` (1.2).
- Text and page counter are drawn after dithering, directly as palette indices, so glyphs stay crisp black on white instead of dithered grey.
- The landscape image is the portrait layout rotated 90 degrees counter-clockwise, as `pi/hadistory.py` does before display.
