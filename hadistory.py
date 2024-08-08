#!/usr/bin/env python3

"""
A program that generates storybook pages with Ollama and Stable Diffusion for the Inky Impression/Waveshare 7 color, all embedded in a RPi 5
"""

import math
import requests
import subprocess
import time
from PIL import Image, ImageDraw, ImageFont
from omni_epd import displayfactory, EPDNotFoundError
import RPi.GPIO as GPIO
import threading
from threading import Thread, Event
from os import listdir
from os.path import isfile, join
import random
import re
import json
import signal
import sys

############## ##############################################################################
############## Constants
############## ##############################################################################
SETTINGS_FILE = 'currently.json'

# Display
DISPLAY_TYPE = "waveshare_epd.epd5in65f" # Set to the name of your e-ink device (https://github.com/robweber/omni-epd#displays-implemented)
DISPLAY_RESOLUTION = (448, 600)

# Ollama API
OLLAMA_API = 'http://localhost:11434/api/generate'

# Ollama model
OLLAMA_MODEL = 'mistral'
#OLLAMA_MODEL = 'gurubot/tinystories-656k-q8' # Works with RPI Zero 2W
#OLLAMA_MODEL = 'gemma:7b'

# Prompt for story
OLLAMA_PROMPT = '''Créer une histoire d'un livre fantasy pour enfant, d'environ 30 mots. Tu peux inclure un héros, un monstre, une créature mythique ou un artefact. Choisis une ambiance ou un thème au hasard. Sois créatif. Inclus une conclusion.'''.replace("\n", "")
# OLLAMA_PROMPT_INCIPIT = '''Peux-tu me créer une histoire issue d'une page d'un livre illustré de fantasy pour enfants. Ce texte doit comporter environ 30 mots. Créer l'histoire basée sur l'instruction suivante : '''.replace("\n", "")
# OLLAMA_PROMPT_EXCIPIT = '''Tu peux choisir une ambiance ou un thème au hasard. N'oublie pas d'inclure une conclusion à l'histoire. Fais preuve de créativité.'''.replace("\n", "")
OLLAMA_PROMPT_INCIPIT = '''Créer une histoire d'un livre fantasy pour enfant, d'environ 30 mots, selon le thème : '''.replace("\n", "")
OLLAMA_PROMPT_EXCIPIT = '''Choisis une ambiance ou un thème au hasard. Sois créatif. Inclus une conclusion.'''.replace("\n", "")
OLLAMA_PROMPT_FILE = "prompts/prompts.txt"

# Stable diffusion
SD_LOCATION = '/home/pi/OnnxStream/src/build/sd'
SD_MODEL_PATH = '/home/pi/OnnxStream/src/build/stable-diffusion-xl-turbo-1.0-onnxstream'
SD_PROMPT = 'une illustration issu d\'un livre pour enfant, style bande dessinée, pour la scène suivante : '
SD_STEPS = 3

# Graphics
TEMP_IMAGE_FILE = '/home/pi/hadistory/image.png' # for temp image storage
LOADING_IMAGE_FILE = '/home/pi/hadistory/ressources/story_creation.png' # for loading image while creating story
FONT_FILE = '/home/pi/hadistory/ressources/CormorantGaramond-Regular.ttf'
FONT_SIZE = 18

# Prompt file parsing
TEXT_PARSE_BRACKETS_LIST = ["()", "[]", "{}"]
CONNECTOR = " "

############## ##############################################################################
############## Init of variables
############## ##############################################################################


chosen_story = "NON_STORY_CHOSEN"
current_page = 0

font = ImageFont.truetype(FONT_FILE, FONT_SIZE)
epd = displayfactory.load_display_driver(DISPLAY_TYPE)

GPIO.setmode(GPIO.BCM) # use GPIO numbering for buttons

execute_pin = 16 #pin for the button execute
GPIO.setup(execute_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

reset_pin = 12 #pin for the button reset
GPIO.setup(reset_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

switch_pin = 21 #pin for the switch
GPIO.setup(switch_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

led_pin = 26 #pin for the led execute
GPIO.setup(led_pin, GPIO.OUT)
GPIO.output(led_pin, GPIO.LOW)

previous_reset_state = True
previous_execute_state = True

############## ##############################################################################
############## Functions
############## ##############################################################################

def signal_handler(sig, frame):
    print('Cleaning up before exiting')
    GPIO.cleanup()
    sys.exit(0)

##### Creating and displaying stories
##### ############## ##############################################################################

def wrap_text_display(text, width, font):
    text_lines = []
    text_line = []
    text = text.replace('\n', ' [br] ')
    words = text.split()

    for word in words:
        if word == '[br]':
            text_lines.append(' '.join(text_line))
            text_line = []
            continue
        text_line.append(word)
        w = int(font.getlength(' '.join(text_line)))
        if w > width:
            text_line.pop()
            text_lines.append(' '.join(text_line))
            text_line = [word]

    if len(text_line) > 0:
        text_lines.append(' '.join(text_line))

    return text_lines

def get_story(prompt = OLLAMA_PROMPT):
    r = requests.post(OLLAMA_API, timeout=600,
        json={
            'model': OLLAMA_MODEL,
            'prompt': prompt,
            'stream':False
                      })
    data = r.json()
    return data['response'].lstrip()

def get_story_no_prompt():
    r = requests.post(OLLAMA_API, timeout=600,
        json={
            'model': OLLAMA_MODEL,
            'prompt': "Once upon a time, ",
            'stream':False
                      })
    data = r.json()
    return "Once upon a time, " + data['response'].lstrip()

def generate_page():
    # Generating text
    print("Creating a new story...")

    prompt = create_prompt(OLLAMA_PROMPT_FILE)
    print("Here is the prompt : " + prompt)

    generated_text = get_story(prompt) # COMMENT AND USE NEXT LINE FOR RPI ZERO 2W USING TINYSTORIES MODEL
    #generated_text = get_story_no_prompt() # UNCOMMENT AND USE THIS LINE FOR RPI ZERO 2W USING TINYSTORIES MODEL

    print("Here is a story: ")
    print(f'{generated_text}')
    generated_text = generated_text.replace("\n", " ")
    generated_text = wrap_text_display(generated_text, 448, font)
    text_height = 0
    for line in generated_text:
        left, top, right, bottom = font.getbbox(line)
        text_height = text_height + int((bottom - top)*1.1)
    text_height = text_height + 2

    generated_text = "\n".join(generated_text)

    # Generating image
    print("Creating the image, may take a while ...")
    translationTable = str.maketrans("éàèùâêîôûç", "eaeuaeiouc")
    text_image_prompt = generated_text.replace('\n',' ').translate(translationTable)
    subprocess.run([SD_LOCATION, '--xl', '--turbo', '--rpi', '--models-path', SD_MODEL_PATH,\
                    '--prompt', SD_PROMPT+f'"{text_image_prompt}"',\
                    '--steps', f'{SD_STEPS}', '--output', TEMP_IMAGE_FILE], check=False)

    # subprocess.run([SD_LOCATION, '--xl', '--turbo', '--rpi-lowmem', '--models-path', SD_MODEL_PATH,\
    #                 '--prompt', SD_PROMPT+f'"{text_image_prompt}"',\
    #                 '--steps', f'{SD_STEPS}', '--output', TEMP_IMAGE_FILE], check=False)

    print("Showing image ...")
    canvas = Image.new(mode="RGB", size=DISPLAY_RESOLUTION, color="white")
    im2 = Image.open(TEMP_IMAGE_FILE)
    if (600 - text_height >=448):
        sizing = 448
    else:
        sizing = 600 - text_height


    im2 = im2.resize((sizing,sizing))

    center_x = int((DISPLAY_RESOLUTION[0]-sizing)/2)
    canvas.paste(im2, (center_x,0))
    im3 = ImageDraw.Draw(canvas)

    im3.text((7, sizing + 2), generated_text, font=font, fill=(0, 0, 0))

    canvas.save('output.png') # save a local copy for closer inspection
    canvas = canvas.rotate(90,expand=1)

    epd.prepare()
    #epd.clear()
    epd.display(canvas)
    epd.sleep()

    print("The end.")

def show_story_page():
    global chosen_story, current_page, story_length

    text_page_path = "stories/"+ chosen_story + "/txt/p" + str(current_page) + ".txt"
    image_page_path = "stories/"+ chosen_story + "/img/p" + str(current_page) + ".png"

    with open(text_page_path) as f: story_text = f.read()
    story_text = story_text.replace("\n", " ")
    story_text = wrap_text_display(story_text, 448, font)
    text_height = 0
    for line in story_text:
        left, top, right, bottom = font.getbbox(line)
        text_height = text_height + int((bottom - top)*1.1)
    text_height = text_height + 2
    story_text = "\n".join(story_text)

    canvas = Image.new(mode="RGB", size=DISPLAY_RESOLUTION, color="white")
    im2 = Image.open(image_page_path)
    if (600 - text_height >=448):
        sizing = 448
    else:
        sizing = 600 - text_height

    im2 = im2.resize((sizing,sizing))

    center_x = int((DISPLAY_RESOLUTION[0]-sizing)/2)
    canvas.paste(im2, (center_x,0))
    im3 = ImageDraw.Draw(canvas)

    im3.text((7, sizing + 2), story_text, font=font, fill=(0, 0, 0))

    page_text = str(current_page) + "/" + str(story_length)
    left, top, right, bottom = im3.textbbox((5, 5), page_text, font=font)
    im3.rectangle((left-5, top-5, right+5, bottom+5), fill="white")
    im3.text((5, 5), page_text, font=font, fill=(0, 0, 0))

    canvas.save('output.png') # save a local copy for closer inspection
    canvas = canvas.rotate(90,expand=1)

    epd.prepare()
    #epd.clear()
    epd.display(canvas)
    epd.sleep()

    if current_page >= story_length:
        current_page = 0
        chosen_story = "NON_STORY_CHOSEN"
        print("The end.")
    else:
        current_page = current_page + 1
        print("To be continued...")

##### Parsing files for creating prompt
##### ############## ##############################################################################

def get_lines(path, encoding='utf-8'):
    lines = []
    with open(path, encoding=encoding) as file:
        for line in file:
            lines.append(line.strip())
    return lines

def parse_weighted_lines(weighted_lines):
    lines = []
    # Find any colons at the start of the line, use preceding text if it's an integer
    for line in weighted_lines:
        amount = 1
        split = line.split(':', maxsplit=1)
        # If there is a valid colon, split it on the first one, add it that many times to list
        if len(split) > 1 and split[0].isdigit():
            amount = int(split[0])
            line = split[1]
        for i in range(amount):
            lines.append(line)
    return lines

def get_random_line(path):
    lines = get_lines(path)
    lines = parse_weighted_lines(lines)
    size = len(lines)
    if size == 0:
        print(f"\nNo lines to parse found in file {path}")
        return
    random.seed()
    r = random.randint(0, size - 1)
    return lines[r]

def parse_multiple_brackets(text, bracket_pairs=TEXT_PARSE_BRACKETS_LIST):
    pairs = bracket_pairs.copy()
    pairs.reverse()
    for brackets in pairs:
        text = parse_text(text, brackets[0], brackets[1])
    return text

def parse_text(text, bracket_one="(", bracket_two=")"):
    # Get everything inside brackets
    regex = fr"\{bracket_one}.*?\{bracket_two}"
    brackets = re.findall(regex, text)
    for bracket in brackets:
        # Get random item
        bracket = bracket.replace(bracket_one, '').replace(bracket_two, '')
        random.seed()
        options = parse_weighted_lines(bracket.split('|'))
        option = random.choice(options)
        # Substitute brackets
        text = re.sub(regex, option, text, 1)
    return text

def create_prompt(path):

    prompt_text = get_random_line(path)
    prompt_text = parse_multiple_brackets(prompt_text, TEXT_PARSE_BRACKETS_LIST)
    prompt_text = re.sub('\s{2,}', ' ', prompt_text)

    full_prompt = OLLAMA_PROMPT_INCIPIT + prompt_text + CONNECTOR + OLLAMA_PROMPT_EXCIPIT

    return full_prompt


##### Led status
##### ############## ##############################################################################

def fade_leds(event):
    pwm = GPIO.PWM(led_pin, 200)

    event.clear()

    while not event.is_set():
        pwm.start(0)
        for dc in range(0, 101, 5):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.05)
        time.sleep(0.75)
        for dc in range(100, -1, -5):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.05)
        time.sleep(0.75)

##### ############## ##############################################################################
##### Main function call
##### ############## ##############################################################################

# chosen_story = "NON_STORY_CHOSEN"
# current_page = 0

if __name__ == '__main__':

    print("Welcome to 𝕙𝕒𝕕𝕚𝕤𝕥𝕠𝕣𝕪 !")

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    try:
        print('Restoring the settings...', end=' ')
        with open(SETTINGS_FILE, 'r') as f:
            d = json.loads(f.read())
    except (OSError, ValueError):
        print('Error reading settings, reverting to defaults.')
    else:
        current_page = d.get('current_page', 0)
        chosen_story = d.get('chosen_story', 0)
        print('Settings loaded !')

    starting_pic = Image.open(LOADING_IMAGE_FILE)
    starting_canvas = Image.new(mode="RGB", size=DISPLAY_RESOLUTION, color="white")
    starting_canvas.paste(starting_pic, (0,0))

    event = threading.Event()

    print("\nWaiting for button press...")
    GPIO.output(led_pin, GPIO.HIGH)

    while True:
        input_state_execute = GPIO.input(execute_pin)
        input_state_reset = GPIO.input(reset_pin)
        switch_state = GPIO.input(switch_pin)
        #switch_state = False # False for AI, True for Story mode

        if input_state_execute == False and switch_state == False: # AI Mode
            if(previous_execute_state):
                current_page = 0
                chosen_story = "NON_STORY_CHOSEN"

                # Saving where we are in the stories
                with open(SETTINGS_FILE, 'w') as f:
                    f.write(json.dumps({'current_page': current_page, 'chosen_story': chosen_story}))

                t_fade = threading.Thread(target=fade_leds, args=(event,))
                t_fade.start()

                epd.prepare()
                #epd.clear()
                epd.display(starting_canvas)
                epd.sleep()

                generate_page()
                event.set()

                time.sleep(3)
                print("\nWaiting for next button press...")
                GPIO.output(led_pin, GPIO.HIGH)
        elif input_state_execute == False and switch_state == True: # Story Mode
            if(previous_execute_state):
                t_fade = threading.Thread(target=fade_leds, args=(event,))
                t_fade.start()

                if chosen_story == "NON_STORY_CHOSEN":
                    stories = [f for f in listdir("stories/") if not isfile(join("stories/", f))]
                    sys_random = random.SystemRandom()
                    chosen_story = sys_random.choice(stories)

                if current_page == 0:
                    current_page = 1

                story_length = len([f for f in listdir("stories/"+ chosen_story + "/txt") if isfile(join("stories/"+ chosen_story + "/txt", f))])

                print("Let's show the following story : " + chosen_story + ", on page " + str(current_page) + "/" + str(story_length))

                show_story_page()

                event.set()
                time.sleep(3)

                # Saving where we are in the stories
                with open(SETTINGS_FILE, 'w') as f:
                    f.write(json.dumps({'current_page': current_page, 'chosen_story': chosen_story}))

                print("\nWaiting for next button press...")
                GPIO.output(led_pin, GPIO.HIGH)
        elif (input_state_reset == False):
                if(previous_reset_state):
                    current_page = 0
                    chosen_story = "NON_STORY_CHOSEN"
                    print("Story reset. Press button to launch a new one")

                    # Saving where we are in the stories
                    with open(SETTINGS_FILE, 'w') as f:
                        f.write(json.dumps({'current_page': current_page, 'chosen_story': chosen_story}))

        previous_reset_state = input_state_reset
        previous_execute_state = input_state_execute
        time.sleep(0.1)


################################################################################################
## END
## #############################################################################################


        #time.sleep(GENERATION_INTERVAL)


    #display.set_image(canvas) #for use of Pimoroni library
    #display.show() #for use of Pimoroni library

# # naive function to replace with newline next space after the offset
# def replace_next_space_with_newline(text, offset):
#     next_space = text.find(' ', offset)
#     if next_space != -1:
#         return text[:next_space] + '\n' + text[next_space + 1:]
#     return text

# # naive function to split text into TOTAL_LINES number of lines
# def split_text(text):
#     char_total = len(text)
#     approx_line_len = math.ceil(char_total/TOTAL_LINES)
#     for i in range(TOTAL_LINES):
#         text = replace_next_space_with_newline(text,approx_line_len*(i+1))
#     return text

############## Unused Constants

#GENERATION_INTERVAL = 1800 #seconds
#TOTAL_LINES = 8
# OLLAMA_PROMPT = '''Create text from the page of an illustrated children\'s fantasy book.
# This text should be around 20 words. If you desire, you can include a hero, monster, mythical
# creature or artifact. You can choose a random mood or theme. Be creative. Do not forget a happy ending to the story.'''.replace("\n", "")

#SD_PROMPT = 'an illustration in a children\'s book for the following scene: '
