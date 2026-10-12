# hadistory — Raspberry Pi Pico version

Work in progress. Port of hadistory to the RP2350 (Raspberry Pi Pico 2 W / Pimoroni Pico Plus 2 / Pico LiPo 2) driving the Inky Impression 5.7" (600 x 448, 7 colours, UC8159).

Scope compared to the Pi version:
- Story mode: pre-rendered pages read from a microSD card (see `tools/`).
- AI mode: OpenAI text + image generation over WiFi. No embedded models.
- 4 buttons (execute, reset, change mode, go back) and a status LED, as on the Pi version.

Wiring, toolchain and build instructions will be documented here.
