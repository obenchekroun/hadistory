# storybook
A program that uses generative models on a Raspberry Pi to create fantasy storybook pages on the Inky Impression e-ink display

![Storybook example](https://github.com/tvldz/storybook/blob/main/examples/storybook.png?raw=true)

## Hardware
- [Raspberry Pi 5 8GB](https://www.raspberrypi.com/products/raspberry-pi-5/). Certainly possible with other hardware, but may be slower and require simpler models.
- [Inky Impression 5.7"](https://shop.pimoroni.com/products/inky-impression-5-7). Code can be modified to support other resolutions.
- SD Card. 32GB is probably the minimum. Use a bigger one to support experimenting with multiple models and installing desktop components if desired.

## Setup
1. Image the SD card with RPi OS Bookworm 64bit lite, then boot and update the OS

2. Set locale correctly using the following :
``` bash
locale #to see locales
sudo update-locale "LC_ALL=en_GB.UTF-8"
sudo update-locale "LANGUAGE=en_GB:en"
```
then reboot.

3. Go to `sudo raspi-config` and enable the following
 - `I2C` interface 
 - `SPI` interface
 - [OPTIONAL] `ssh`
 - [OPTIONAL] set up wifi

3. Install required libraries
``` bash
sudo apt update
sudo apt -y upgrade
sudo apt install cmake
sudo apt-get install git
sudo apt-get install python3-dev
```

4. [Install Ollama](https://ollama.com/download/linux)
``` bash
cd ~
curl -fsSL https://ollama.com/install.sh | sh
```
 - Pull and serve an Ollama model. I find that Mistral and Gemma models work well. `ollama run gemma:7b`

5. [Build/install XNNPACK and Onnxstream](https://github.com/vitoplantamura/OnnxStream?tab=readme-ov-file#how-to-build-the-stable-diffusion-example-on-linuxmacwindowstermux)
 - First install XNNPACK :
``` bash
cd ~
git clone https://github.com/google/XNNPACK.git
cd XNNPACK
git checkout 579de32260742a24166ecd13213d2e60af862675
mkdir build
cd build
cmake -DXNNPACK_BUILD_TESTS=OFF -DXNNPACK_BUILD_BENCHMARKS=OFF ..
cmake --build . --config Release
```
 - and then ONXXSTREAM :
```bash
cd ~
git clone https://github.com/vitoplantamura/OnnxStream.git
cd OnnxStream
cd src
mkdir build
cd build
cmake -DMAX_SPEED=ON -DOS_LLM=OFF -DOS_CUDA=OFF -DXNNPACK_DIR=/home/pi/XNNPACK .. #path to be changed for where XNNPACK has been cloned
cmake --build . --config Release
```
 - Download an SD model. I find that [Stable Diffusion XL Turbo 1.0](https://github.com/vitoplantamura/OnnxStream?tab=readme-ov-file#stable-diffusion-xl-turbo-10) works well. First launch should download the model so running `./sd --turbo --rpi should download the XL Turbo 1.0`
 
6. Clone this repository. `git clone https://github.com/obenchekroun/hadistory.git`
 - Create a Python virtual environment: `cd hadistory && mkdir .venv && python -m venv .venv`
 - Activate the environment: `source .venv/bin/activate`
 - Install the [Inky libraries](https://github.com/pimoroni/inky). Follow these instructions for RPi 5 compatibility: https://github.com/pimoroni/inky/pull/182
 - Install requests and pillow: `pip install requests pillow`
7. Modify the constants (paths) at the top of `main.py` to match your own environment.
8. execute main.py: `python3 main.py`. Execution takes ~5 minutes.

## ISSUES/IDEAS/TODO
- Currently, the program just renders a single page at a set interval. It would certainly possible to ask Ollama to generate multiple pages for a complete "story", and then generate illustrations for each page. The entire "story" could be saved locally and "flipped" through more rapidly than discrete page generation.
- The output lacks some diversity, with many of the same characters and themes. This may be improved with a higher quality prompt, modifying the model temperature, or creating a prompt generator that randomly generates prompts from a set of themes, characters, creatures, artifacts, etc.
- The current font doesn't look great on the display. Finding a better font, or perhaps rendering the page horizontally instead of rotating it might have a better result.
- Fitting the text on the screen doesn't always work, since I'm requesting that the model limit itself and naively splitting the output programmatically.
- This would be easily modifiable to create other things like sci-fi stories, weird New Yorker cartoons or off-brand Pokemon.
- This may be thermally taxing on the RPi. Inferrence consumes all CPUs for many minutes, then sits idle for the set interval.
- The code isn't very reslilient but seems to work reliably.
