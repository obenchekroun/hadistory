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


GENERATION_INTERVAL = 1800 #seconds
DISPLAY_RESOLUTION = (448, 600)
#TOTAL_LINES = 8
OLLAMA_API = 'http://localhost:11434/api/generate'
OLLAMA_MODEL = 'mistral'
OLLAMA_PROMPT = '''Create text from the page of an illustrated children\'s fantasy book.
This text should be around 20 words. If you desire, you can include a hero, monster, mythical
creature or artifact. You can choose a random mood or theme. Be creative. Do not forget a happy ending to the story.'''.replace("\n", "")
# OLLAMA_PROMPT = '''Peux-tu me créer un texte issue d'une page d'un livre illustré de fantasy pour enfants.
# Ce texte doit comporter environ 20 mots. Si tu le souhaites, tu peux inclure un héros, un monstre, une créature mythique ou un artefact. Tu peux choisir une ambiance ou un thème au hasard. N'oublie pas d'inclure une conclusion à l'histoire. Fais preuve de créativité.'''.replace("\n", "")
SD_LOCATION = '/home/pi/OnnxStream/src/build/sd'
SD_MODEL_PATH = '/home/pi/OnnxStream/src/build/stable-diffusion-xl-turbo-1.0-onnxstream'
SD_PROMPT = 'an illustration in a children\'s book for the following scene: '
#SD_PROMPT = 'une illustration style bande dessinée pour l\'histoire suivante : '
SD_STEPS = 3
TEMP_IMAGE_FILE = '/home/pi/hadistory/image.png' # for temp image storage
LOADING_IMAGE_FILE = '/home/pi/hadistory/ressources/story_creation.png' # for loading image while creating story
FONT_FILE = '/home/pi/hadistory/ressources/CormorantGaramond-Regular.ttf'
FONT_SIZE = 18
DISPLAY_TYPE = "waveshare_epd.epd5in65f" # Set to the name of your e-ink device (https://github.com/robweber/omni-epd#displays-implemented)

font = ImageFont.truetype(FONT_FILE, FONT_SIZE)
epd = displayfactory.load_display_driver(DISPLAY_TYPE)

GPIO.setmode(GPIO.BCM)
button_pin = 16
GPIO.setup(button_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP) #pin for the button
led_pin = 26
GPIO.setup(led_pin, GPIO.OUT)
GPIO.output(led_pin, GPIO.LOW)

def get_story():
    r = requests.post(OLLAMA_API, timeout=600,
        json={
            'model': OLLAMA_MODEL,
            'prompt': OLLAMA_PROMPT,
            'stream':False
                      })
    data = r.json()
    return data['response'].lstrip()

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

def generate_page():
    global fade_leds_bool
    # Generating text
    print("\nCreating a new story...")
    generated_text = get_story()
    #generated_text = "Luna's moonbeam cloak rustled like whispers as she crept through Whispering Wood. The gnarled branches of the Elder Willow seemed to hold their breath, afraid of disturbing the slumbering Moon Sphinx. The moonstone amulet, passed down through generations, glowed in her palm, guiding her to its rightful place atop the Sphinx's head. With a soft click, the ancient slumber ended, and the woods were filled with the melodious hum of a newly awakened moon. And this is some added text randomly so i can test the dynamic resizing of the text and complete de story randomly. blablalballbalbl Je continue ici mn texte pour voir la capacité de mon script à calculer la bonne hauteur de texte et l'afficher correctement."
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
    print("\nCreating the image, may take a while ...")
    translationTable = str.maketrans("éàèùâêîôûç", "eaeuaeiouc")
    text_image_prompt = generated_text.replace('\n',' ').translate(translationTable)
    subprocess.run([SD_LOCATION, '--xl', '--turbo', '--rpi', '--models-path', SD_MODEL_PATH,\
                    '--prompt', SD_PROMPT+f'"{text_image_prompt}"',\
                    '--steps', f'{SD_STEPS}', '--output', TEMP_IMAGE_FILE], check=False)


    print("\nShowing image ...")
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
    time.sleep(10)
    epd.prepare()
    #epd.clear()
    epd.display(canvas)
    epd.sleep()

    print("\nThe end.")

def fade_leds(event):
    pwm = GPIO.PWM(led_pin, 200)

    event.clear()
    #GPIO.output(led_pin, GPIO.LOW)
    while not event.is_set():
        #print("\nfading")
        pwm.start(0)
        for dc in range(0, 101, 5):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.05)
        time.sleep(0.75)
        for dc in range(100, -1, -5):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.05)
        time.sleep(0.75)


if __name__ == '__main__':

    print("Welcome to 𝕙𝕒𝕕𝕚𝕤𝕥𝕠𝕣𝕪 !")

    starting_pic = Image.open(LOADING_IMAGE_FILE)
    starting_canvas = Image.new(mode="RGB", size=DISPLAY_RESOLUTION, color="white")
    starting_canvas.paste(starting_pic, (0,0))

    event = threading.Event()

    print("\nWaiting for button press...")
    GPIO.output(led_pin, GPIO.HIGH)

    while True:
        input_state = GPIO.input(button_pin)
        if input_state == False:
            print("Let's go !")

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
