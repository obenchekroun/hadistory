# storybook for Hadi
A program that uses generative models on a Raspberry Pi to create fantasy storybook pages on the Inky Impression e-ink display

| ![Hadistory !](/img/hadistory.png?raw=true) | 
|:--:| 
| *Hadistory !* |


## Acknowledgments
Based on storybook : [tvldz's storybook](https://github.com/tvldz/storybook). This project has been largely based on storybook with a few tweaks for my needs. All credit goes to them for making this awesome project.

## Hardware
- [Raspberry Pi 5 8GB](https://www.raspberrypi.com/products/raspberry-pi-5/). It can also be installed on a [Raspberry Pi Zero 2W](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/). but the AI mode takes ~1h to generate a story Certainly possible with other hardware, but may be slower and require simpler AI models. 
- [Inky Impression 5.7"](https://shop.pimoroni.com/products/inky-impression-5-7) or [Waveshare 7 color 5.65"](https://www.waveshare.com/5.65inch-e-paper-module-f.htm). Code can be modified to support other resolutions. 
- SD Card. 32GB is probably the minimum. Use a bigger one to support experimenting with multiple models and installing desktop components if desired.
- 2 buttons, 1 switch, 1 LED and a 220 Ohms resistors

## Use
Execute main.py: `python3 hadistory.py`., or enable autostart of the project.
The project has two modes : 
- `Story mode` where the script will chose a story from `stories/` subfolders at random and goes through each page in order, after each press, 
- `AI mode`, an AI mode where the script will use stable diffusion and ollma-infered model to creat a one page novel story, with execution taking ~10 minutes on a pi5 and 40min on a RPi2. 
A switch (or simply grounding the switch pin) allows to switch from one mode to the other
The LED act as a status indicator : the LED is fully lit when waiting for button press, and fading regularly when generating a story. 

There is also a reset button to reset the current story in `Story mode` and make it chose a new one after next execution.

### Prompt generation
The prompt system creation uses a specific file  `prompts/prompts.txt` to pick random themes from it, and uses brackets and weight to add possibilites to theme creations :
- The final prompt is created by appending instructions to a random line from this file, that gives the theme of the (see `hadistory.py` and variables `OLLAMA_PROMPT_INCIPIT` and `OLLAMA_PROMPT_EXCIPIT` for the preamble and ending of the prompt resp.)
- Thus each line gives the theme of the story
- A positive integer followed by a colon at the beginnig of a line gives a weigth when randomly choosing a theme (higher number = higher chance to pick this line)
- each prompt can be personnalised with multiple options using brackets `( | ) or [ | ] or { |  }` and items inside separated by a pipe operator `|`. When choosing a line, the script will also pick randomly from options inside the brackets. Please note that the brackets can be casacading, eg. `(A farmer upset with the speed of his tractor|A happy farmer driving a [sports|foreign|totally {awesome|great}] car)`

## Setup
1. Image the SD card with RPi OS Bookworm 64bit lite, then boot and update the OS.

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
 - Pull and serve an Ollama model. I find that Mistral and Gemma models work well : `ollama run gemma:7b`,  `ollama run mistral`  or  `ollama run qwen2:0.5b` (really small model for RPi Zero 2W)

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
 - Download a Stable Diffusion model. I find that [Stable Diffusion XL Turbo 1.0](https://github.com/vitoplantamura/OnnxStream?tab=readme-ov-file#stable-diffusion-xl-turbo-10) works well. First launch should download the model so running `./sd --turbo --rpi should download the XL Turbo 1.0`
 
6. Clone this repository. `git clone https://github.com/obenchekroun/hadistory.git`
 - Create a Python virtual environment: `cd hadistory && mkdir .venv && python -m venv .venv`
 - Activate the environment: `source .venv/bin/activate`
   - *NB: you can proceeed without a virtual environnment by using `--break-system-packages` argument for every pip install command. If not using a virtual environment, adapt the `autostart/autostart.sh script` by commenting or uncommenting the line `source .venv/bin/activate`* 
 - Install the libraries for the screen. The project uses [omni-epd](https://github.com/robweber/omni-epd). To install :
   ```bash
   pip3 install git+https://github.com/robweber/omni-epd.git#egg=omni-epd
   ```
     - Configure screen in `main.py`, with the variable `DISPLAY_TYPE`. Note that omni-epd uses `omni-epd.ini` as a config file, see its contents for options
     - Note that there is an issue with the RPi.GPIO library required by omni-epd or waveshare libraries. Raspberry Pi OS Bookworm includes a pre-installed 'RPi.GPIO' which is not compatible with Bookworm or a Pi 5. One option is to use a drop-in replacement which should work :
   ```bash
   sudo apt remove python3-rpi.gpio
   pip3 install rpi-lgpio
   ```
      See the following links for reference : https://forums.raspberrypi.com/viewtopic.php?t=362657 and https://forums.raspberrypi.com/viewtopic.php?p=2160578#p2160578.
 - Install requests and pillow: `pip install requests pillow`
7. Modify the constants (paths) at the top of `hadistory.py` to match your own environment.
8. Set AI model to be used with the variable `OLLAMA_MODEL = 'mistral'` 
9. Execute main.py: `python3 hadistory.py`. The project has two modes : `Story mode` where the script will chose a story from `stories/` subfolders at random and goes through each page in order, after each press, and an `AI mode` where the script will use stable diffusion and ollma-infered model to creat a one page novel story, with execution taking ~5 minute. The LED is fully lit when waiting for button press, and fading regularly when generating a story. There is also a reset button to reset the current story in `MODE=0` and make it chose a new one after next execution.
  - In order to force only one mode, change the line `switch_state = GPIO.input(switch_pin)`, to either :
    - `switch_state = False` for AI mode
    - `switch_state = True` for Story mode
    
9. Personnalize prompts used to generate stories by modifying `prompts/prompts.txt` as explained in the previous section.

### Connect EPD to Pi
* CAREFULLY plug EPD into Raspberry Pi, or on top of pijuice HAT, following instructions from the vendor.

In the case of the Waveshare e-paper 5.65inch 7colors display used in this case, the connection is as follows :
![Pin connection to Raspberry Pi](/img/pin_waveshare_epd.epd5in65f.png?raw=true)

The RPi 5 pin out is as follows : 
![Pin connection of Raspberry Pi](/img/Raspberry-Pi-5-Pinout.jpg?raw=true)

* Connect power directly to Raspberry Pi (or PiJuice unit) once done.

* Enable SPI interface

### Connect LED and buttons
The project uses a LED as a status indicator, a button to trigger the creation of a story, a button to reset current story and a switch to go form . The corresponding GPIO pin can be customised in `main.py` with the variables `button_pin` and `led_pin`. By default they are to be wired as follows :
- *button execute* : GPIO 16 (pin 36) and Ground (pin 39) for example
- *button reset* : GPIO 12 (pin 32) and Ground (pin 39) for example
- *switch* : GPIO 21 (pin 40) and 3v3 (pin 1 or 17) for example
- *Led* : with a 220 Ohms resistor, to GPIO 26 (pin 37) and Ground (pin 39) for example

### Running on startup
  - Lastly we just want to make this run at boot :

``` bash
sudo raspi-config nonint do_boot_behaviour B2 #enables auto login as command line
```

- then put corresponding scripts in folders :
  - in `/etc/profile.d` the script `15-hadistory.sh` that test if not ssh and not x11 and then execute script
  
``` bash
sudo cp /home/pi/hadistory/autostart/put_in_etc_profile.d/15-hadistory.sh /etc/profile.d
```

Note that in `/home/pi/hadistory/autostart/` the script with actual commands to be executed after start up `autostart.sh`, and called by `15-hadistory.sh`.

- then give correct permissions :

``` bash
sudo chown pi:pi /home/pi/hadistory/autostart/autostart.sh
```

- If not using a virtual environment, adapt the `autostart/autostart.sh` script by commenting or uncommenting the line `source .venv/bin/activate`

NB : The user created is named pi, if another user or project put in another folder, change the `cd /home/pi/hadistory` command in `autostart.sh` and the path in `15-hadistory.sh`.


## Miscellaneous

### Stablediffusion shell command example

``` bash
/home/pi/OnnxStream/src/build/sd --xl --turbo --rpi --models-path /home/pi/OnnxStream/src/build/stable-diffusion-xl-turbo-1.0-onnxstream --prompt "an illustration in a children's book for the following scene: Luna's moonbeam cloak rustled as she crept through Whispering Wood, the silver moon casting her path in an ethereal glow. The Whispering Flowers whispered secrets of forgotten stars, guiding her towards the obsidian tower where the Moon Weaver resided." --steps 3
```

### ChatGPT prompt example to create a story

``` bash
Peux-tu s'il te plait créer une histoire pour enfant fantastique qui tient sur 4 pages de 70 mots chacune. Si tu veux tu peux inclure un héros, un artefact magique et un monstre. Tu peux choisir un thème ou une ambiance au hasard. Fais preuve de créativité et n'oublie pas d'inclure une fin heureuse. Créé ensuite 4 illustrations, une pour chaque page, style bande dessinée, en ayant de la cohérence dans les images
```


## ISSUES/IDEAS/TODO
- Currently, the program just renders a single page at a set interval. It would certainly possible to ask Ollama to generate multiple pages for a complete "story", and then generate illustrations for each page. The entire "story" could be saved locally and "flipped" through more rapidly than discrete page generation.
- The output lacks some diversity, with many of the same characters and themes. This may be improved with a higher quality prompt, modifying the model temperature, or creating a prompt generator that randomly generates prompts from a set of themes, characters, creatures, artifacts, etc.
- The current font doesn't look great on the display. Finding a better font, or perhaps rendering the page horizontally instead of rotating it might have a better result.
- Fitting the text on the screen doesn't always work, since I'm requesting that the model limit itself and naively splitting the output programmatically.
- This would be easily modifiable to create other things like sci-fi stories, weird New Yorker cartoons or off-brand Pokemon.
- This may be thermally taxing on the RPi. Inferrence consumes all CPUs for many minutes, then sits idle for the set interval.
- The code isn't very reslilient but seems to work reliably.

## Miscellaneous

Can be used with official libraries from pimoroni : [Inky libraries](https://github.com/pimoroni/inky). Follow these instructions for RPi 5 compatibility: https://github.com/pimoroni/inky/pull/182
