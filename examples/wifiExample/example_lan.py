"""
HMI2 LAN Example for MicroPython
Demonstrates how to use the HMI2 library with LAN/Ethernet communication.
Requires network connectivity (WiFi or Ethernet).
"""

from hmi2 import Hmi2
import network
import time

# Initialize HMI2 object
hmi2 = Hmi2()

# Connect to WiFi (for ESP32/ESP8266)
# Uncomment and configure for your network:
"""
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect('YOUR_SSID', 'YOUR_PASSWORD')

# Wait for connection
while not wlan.isconnected():
    time.sleep(0.1)

print('WiFi connected:', wlan.ifconfig())
"""

# Initialize HMI2 with LAN connection
# IP address can be string "192.168.1.66" or tuple (192, 168, 1, 66)
# Second parameter is LAN memory bank (1-6)
server_ip = "192.168.1.66"  # Change to your server IP
hmi2.init(server_ip, 1)

# Example: Set up GPIO pins (adjust for your board)
led_pin = None  # Pin(2, Pin.OUT)  # Uncomment and adjust
button_pin = None  # Pin(0, Pin.IN)  # Uncomment and adjust

ledState = False

while True:
    # Read boolean from HMI app (word 1, bit 2)
    ledState = hmi2.getBoolean(1, 2)
    
    # Write values to HMI app
    # hmi2.setInt(4, some_value)  # N File
    # hmi2.setDInt(4, some_value)  # D File
    # hmi2.setFloat(6, some_value)  # F File
    
    # Write digital input to HMI app
    # if button_pin:
    #     hmi2.setBoolean(2, 10, button_pin.value())
    #     hmi2.setBoolean(0, 0, button_pin.value())
    
    # Control LED based on HMI app state
    # if led_pin:
    #     if ledState:
    #         led_pin.on()
    #     else:
    #         led_pin.off()
    
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
    hmi2.print("LAN Mode")
    
    # Note: update() is now called automatically! No need to call it manually.
    # Updates happen automatically when reading/writing data (throttled to every 50ms by default)
    # You can still call hmi2.update() manually if needed, or disable auto-update:
    # hmi2.enableAutoUpdate(False)  # Disable auto-update
    # hmi2.enableAutoUpdate(True, interval_ms=100)  # Enable with custom interval
    
    # Small delay to prevent tight loop (optional, auto-update handles timing)
    time.sleep(0.01)
