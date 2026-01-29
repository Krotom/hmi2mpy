"""
HMI2 Serial Example for MicroPython
Demonstrates how to use the HMI2 library with UART serial communication.
"""

from machine import UART, Pin
from hmi2 import Hmi2

# Initialize HMI2 object
hmi2 = Hmi2()

# Set up UART (adjust pins and baudrate for your board)
# For ESP32: UART(1, baudrate=9600, tx=17, rx=16)
# For ESP8266: UART(0, baudrate=9600)
# For Raspberry Pi Pico: UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))
uart = UART(1, baudrate=9600, tx=17, rx=16)

# Initialize HMI2 with serial connection
hmi2.init(uart)

# Example: Set up GPIO pins (adjust for your board)
led_pin = Pin(2, Pin.OUT)
button_pin = Pin(0, Pin.IN)

ledState = False

while True:
    # Read boolean from HMI app (word 1, bit 2)
    ledState = hmi2.getBoolean(1, 2)
    
    # Write analog values to HMI app
    # Note: MicroPython doesn't have analogRead like Arduino
    # You'll need to use ADC if available on your board
    # hmi2.setInt(4, adc.read())  # N File
    # hmi2.setDInt(4, adc.read())  # D File
    # hmi2.setFloat(6, adc.read())  # F File
    
    # Write digital input to HMI app
    hmi2.setBoolean(2, 10, button_pin.value())
    hmi2.setBoolean(0, 0, button_pin.value())
    
    # Read value from HMI app and control output
    # analogWrite equivalent would be PWM
    # pwm.duty(hmi2.getDInt(12))
    
    # Control LED based on HMI app state
    if ledState:
        led_pin.on()
    else:
        led_pin.off()
    
    # Check another boolean from HMI app
    if hmi2.getBoolean(3, 0):
        # Do something
        pass
    
    # Display text on HMI
    hmi2.setDisplayID(1)
    hmi2.setCursor(0, 0)
    hmi2.print("Hello World!!!")
    
    hmi2.setCursor(0, 1)
    hmi2.print("-> ")
    # hmi2.print(adc.read())  # Print analog value
    
    # Note: update() is now called automatically! No need to call it manually.
    # Updates happen automatically when reading/writing data (throttled to every 50ms by default)
    # You can still call hmi2.update() manually if needed, or disable auto-update:
    # hmi2.enableAutoUpdate(False)  # Disable auto-update
    # hmi2.enableAutoUpdate(True, interval_ms=100)  # Enable with custom interval
