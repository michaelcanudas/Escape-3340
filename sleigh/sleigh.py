from gpiozero import DigitalInputDevice, DigitalOutputDevice
from time import sleep

hall_pin = 14
relay_pin = 2

hall_sensor = DigitalInputDevice(hall_pin, pull_up=True)
relay = DigitalOutputDevice(relay_pin)

def setup():
    relay.off()
    

# Main loop
try:
    setup()
    while True:
        print(f"Hall Pin 1 State: {hall_sensor.value}")
        if (hall_sensor.value == 1):
            print("Magnet attached")
            relay.on()
        else:
            relay.off()
        sleep(0.5)
except KeyboardInterrupt:
    print("Program interrupted")
