import RPi.GPIO as GPIO
from time import sleep

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(11, GPIO.OUT, initial=GPIO.LOW)

while True:
    GPIO.output(11, GPIO.HIGH)
    sleep(1)
    GPIO.output(11, GPIO.LOW)
    sleep(1)
    
