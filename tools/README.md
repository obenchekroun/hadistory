# hadistory — desktop tools

Scripts that run on a computer (not on the device), mainly to prepare content for the Pico version.

## `prepare_pages.py` — prepare story pages for the Pico version

Reads the shared stories in `stories/` and writes, for each page, a JPEG illustration and the page text. The Pico firmware composes the page itself, illustration on top and text below in its bitmap font, dithered to the 7 colours of the panel, exactly as it composes the pages of its AI mode. So the stories and the AI pages look the same, and editing a text does not require any rendering.

Requirements: Python 3, Pillow.

```bash
python3 tools/prepare_pages.py                  # all stories -> tools/out/
python3 tools/prepare_pages.py --story story_3  # a single story (repeatable)
python3 tools/prepare_pages.py --quality 70     # smaller JPEGs (default 85)
python3 tools/prepare_pages.py --embed DIR      # also write the files for the flash variant (see below)
python3 tools/prepare_pages.py --help
```

For the microSD variant of the firmware (`STORAGE=SDCARD`), copy `tools/out/stories/` to the root of the card. `tools/out/` is ignored by git; it is rebuilt from `stories/` in a few seconds.

### Output format

One folder per story, `stories/<story>/`, with for each page:

| File | Content |
|---|---|
| `pN.jpg` | Illustration, square, 448 x 448 by default (`--size`), baseline JPEG, sharpness 1.2 applied as on the Pi version |
| `pN.txt` | Page text, UTF-8, one paragraph (newlines replaced by spaces) |

Sizes for the 12 stories, 97 pages, with the illustrations of the repository:

| `--quality` | Total |
|---|---|
| 85 (default) | 4.5 MB |
| 75 | 3.4 MB |
| 70 | 3.1 MB |
| 60 | 2.6 MB |

The firmware holds one JPEG at a time in a 160 KB buffer (`IMAGE_MAX_BYTES`); the largest page at quality 85 is about 95 KB.

### Flash variant (`--embed DIR`)

For the firmware variant with the stories embedded in flash (`STORAGE=FLASH`), `--embed DIR` additionally writes into `DIR`:

- `stories_data.S`: one `.incbin` per JPEG and per text (the text is NUL terminated), 4-byte aligned.
- `stories_table.c`: the catalogue of embedded stories (name, page count, per page the JPEG pointer and size and the text pointer), declared in `pico/src/stories_embedded.h`.

The Pico CMake build calls this itself when `STORAGE=FLASH`, with the quality given by its `EMBED_QUALITY` variable (default 85) and the selection given by `EMBED_STORIES`; there is no need to run it by hand for that variant.
