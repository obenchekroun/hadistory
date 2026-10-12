# hadistory

A storybook device that shows illustrated children's stories on a 7-colour e-paper display, one page per button press. Stories are either pre-generated and stored locally, or created on the fly with generative models.

| ![Hadistory !](/img/hadistory.png?raw=true) |
|:--:|
| *Hadistory !* |

Based on [tvldz's storybook](https://github.com/tvldz/storybook).

## Repository layout

The project targets two platforms that share the same stories and prompts:

| Folder | Content |
|---|---|
| [`pi/`](pi/) | Raspberry Pi version (RPi 5 / Zero 2W, Python, omni-epd, local AI with Ollama + OnnxStream or OpenAI). See [`pi/README.md`](pi/README.md). |
| [`pico/`](pico/) | Raspberry Pi Pico 2 version (RP2350, C on the Pico SDK). Stories on microSD or embedded in flash, OpenAI mode optional. Work in progress. See [`pico/README.md`](pico/README.md). |
| [`tools/`](tools/) | Desktop scripts: pre-rendering of the story pages for the Pico version (microSD or flash). See [`tools/README.md`](tools/README.md). |
| `stories/` | Shared pre-generated stories: one folder per story, `txt/pN.txt` and `img/pN.png` per page. |
| `stories_not_used/` | Stories kept aside, not loaded by the device. |
| `prompts/` | Shared theme files used to build the story-generation prompt. |
| `ressources/` | Shared assets: font and loading image. |
| `img/` | Images used by the READMEs. |

## License

See [LICENSE](LICENSE).
