import RPi.GPIO as GPIO
import threading
from threading import Thread, Event
import time

GPIO.setmode(GPIO.BCM)

led_pin = 26
GPIO.setup(led_pin, GPIO.OUT)
GPIO.output(led_pin, GPIO.LOW)

GPIO.output(led_pin, GPIO.HIGH)

time.sleep(3)

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

event = threading.Event()
t_fade = threading.Thread(target=fade_leds, args=(event,))
t_fade.start()
time.sleep(5)

event.set()
time.sleep(3)

GPIO.output(led_pin, GPIO.HIGH)
time.sleep(3)


