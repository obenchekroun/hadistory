#!/usr/bin/env python3

"""
A program that generates storybook pages with Ollama and Stable Diffusion for the Inky Impression
"""

import math
import requests
import subprocess
import time
from PIL import Image, ImageDraw, ImageFont
from omni_epd import displayfactory, EPDNotFoundError
#from inky.auto import auto #for use of Pimoroni library

#display = auto() #for use of Pimoroni library

GENERATION_INTERVAL = 1800 #seconds
DISPLAY_RESOLUTION = (448, 600)
TOTAL_LINES = 8
OLLAMA_API = 'http://localhost:11434/api/generate'
OLLAMA_MODEL = 'gemma:7b'
# OLLAMA_PROMPT = '''Create text from the page of an illustrated children\'s fantasy book.
# This text should be around 20 words. If you desire, you can include a hero, monster, mythical
# creature or artifact. You can choose a random mood or theme. Be creative. Do not forget an ending to the story.'''.replace("\n", "")
OLLAMA_PROMPT = '''Créez un texte à partir d'une page d'un livre illustré de fantasy pour enfants.
Ce texte doit comporter environ 20 mots. Si vous le souhaitez, vous pouvez inclure un héros, un monstre, une créature
mythique ou un artefact. Vous pouvez choisir une ambiance ou un thème au hasard. Faites preuve de créativité. N'oubliez pas la fin de l'histoire.'''.replace("\n", "")
SD_LOCATION = '/home/pi/OnnxStream/src/build/sd'
SD_MODEL_PATH = '/home/pi/OnnxStream/src/build/stable-diffusion-xl-turbo-1.0-onnxstream'
#SD_PROMPT = 'an illustration in a children\'s book for the following scene: '
SD_PROMPT = 'une illustration dans un livre pour enfants pour la scène suivante : '
SD_STEPS = 3
TEMP_IMAGE_FILE = '/home/pi/hadistory/image.png' # for temp image storage
FONT_FILE = '/home/pi/hadistory/CormorantGaramond-Regular.ttf'
FONT_SIZE = 18

DISPLAY_TYPE = "waveshare_epd.epd5in65f" # Set to the name of your e-ink device (https://github.com/robweber/omni-epd#displays-implemented)

def get_story():
    r = requests.post(OLLAMA_API, timeout=600,
        json={
            'model': OLLAMA_MODEL,
            'prompt': OLLAMA_PROMPT,
            'stream':False
                      })
    data = r.json()
    return data['response'].lstrip()

# naive function to replace with newline next space after the offset
def replace_next_space_with_newline(text, offset):
    next_space = text.find(' ', offset)
    if next_space != -1:
        return text[:next_space] + '\n' + text[next_space + 1:]
    return text

# naive function to split text into TOTAL_LINES number of lines
def split_text(text):
    char_total = len(text)
    approx_line_len = math.ceil(char_total/TOTAL_LINES)
    for i in range(TOTAL_LINES):
        text = replace_next_space_with_newline(text,approx_line_len*(i+1))
    return text

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
    # Generating text
    print("\nCreating a new story...")
    generated_text = get_story()
    #generated_text = "Luna's moonbeam cloak rustled like whispers as she crept through Whispering Wood. The gnarled branches of the Elder Willow seemed to hold their breath, afraid of disturbing the slumbering Moon Sphinx. The moonstone amulet, passed down through generations, glowed in her palm, guiding her to its rightful place atop the Sphinx's head. With a soft click, the ancient slumber ended, and the woods were filled with the melodious hum of a newly awakened moon. And this is some added text randomly so i can test the dynamic resizing of the text and complete de story randomly. blablalballbalbl blalbalbalballblalbalblalablbalabllba lbalballbalbalballbalbalballbalbalbalb al lbalbalbal b allbalb albalballbal ba"
    print("Here is a story: ")
    print(f'{generated_text}')
    #generated_text = split_text(generated_text)
    generated_text = wrap_text_display(generated_text, 448, font)
    TOTAL_LINES = len(generated_text)
    left, top, right, bottom = font.getbbox(generated_text[0])
    text_height = int((bottom - top)*TOTAL_LINES)
    generated_text = "\n".join(generated_text)

    # Generating image
    print("\nCreating the image, may take a while ...")
    subprocess.run([SD_LOCATION, '--xl', '--turbo', '--rpi', '--models-path', SD_MODEL_PATH,\
                    '--prompt', SD_PROMPT+f'"{generated_text}"',\
                    '--steps', f'{SD_STEPS}', '--output', TEMP_IMAGE_FILE], check=False)


    print("\nShowing image ...")
    canvas = Image.new(mode="RGB", size=DISPLAY_RESOLUTION, color="white")
    im2 = Image.open(TEMP_IMAGE_FILE)
    if (600 - text_height >=448):
        sizing = 448
    else:
        sizing = 600 - text_height - int((bottom - top))*2

    #im2 = im2.resize((448,448))
    im2 = im2.resize((sizing,sizing))
    #exit()
    center_x = int((DISPLAY_RESOLUTION[0]-sizing)/2)
    canvas.paste(im2, (center_x,0))
    im3 = ImageDraw.Draw(canvas)
    #font = ImageFont.truetype(FONT_FILE, FONT_SIZE)
    #im3.text((7, 450), generated_text, font=font, fill=(0, 0, 0))
    im3.text((7, sizing + 2), generated_text, font=font, fill=(0, 0, 0))
    #canvas.show()
    canvas.save('output.png') # save a local copy for closer inspection
    canvas = canvas.rotate(90,expand=1)
    epd.prepare()
    epd.clear()
    epd.display(canvas)
    epd.sleep()
    print("\nThe end.")
    #display.set_image(canvas) #for use of Pimoroni library
    #display.show() #for use of Pimoroni library


if __name__ == '__main__':
    epd = displayfactory.load_display_driver(DISPLAY_TYPE)
    font = ImageFont.truetype(FONT_FILE, FONT_SIZE)
    #font = ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', 18)
    print("Welcome to 𝕙𝕒𝕕𝕚𝕤𝕥𝕠𝕣𝕪 !\n")
    #while True:
    generate_page()
    #time.sleep(GENERATION_INTERVAL)
