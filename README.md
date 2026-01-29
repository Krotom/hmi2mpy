# HMI2 MicroPython Library

A MicroPython port of the Arduino/ESP HMI2 control panel library. This library provides communication with HMI control panel applications via serial (UART) or LAN (socket) connections.

## Features

- **Boolean Data (B File)**: Read/write boolean values stored as bits in 16-bit words
- **Integer Data (N File)**: Read/write 16-bit unsigned integers
- **Double/32-bit Integer Data (D File)**: Read/write 32-bit unsigned integers
- **Float Data (F File)**: Read/write floating-point values
- **Display/LCD Support**: Text output with cursor positioning
- **Multiple Connection Types**: Hardware serial (UART) and LAN (socket) support
- **Automatic Updates**: No need to call `update()` manually - synchronization happens automatically!

## Installation

Copy `hmi2.py` to your MicroPython device. The library uses only standard MicroPython modules:
- `machine.UART` for serial communication
- `socket` and `network` for LAN communication (optional)

## Usage

### Serial (UART) Connection

```python
from machine import UART
from hmi2 import Hmi2

# Create HMI2 instance
hmi2 = Hmi2()

# Initialize UART (adjust pins for your board)
uart = UART(1, baudrate=9600, tx=17, rx=16)

# Initialize HMI2 with serial connection
# Auto-update is enabled by default (updates every 50ms)
hmi2.init(uart)

# Use the library - updates happen automatically!
hmi2.setBoolean(0, 0, True)
value = hmi2.getBoolean(1, 2)  # Automatically syncs before reading
hmi2.setInt(4, 1234)  # Automatically syncs after writing
hmi2.setFloat(6, 3.14)

# No need to call update() - it's automatic!
# You can still call hmi2.update() manually if needed
```

### LAN Connection

```python
from hmi2 import Hmi2
import network

# Connect to WiFi first (ESP32/ESP8266)
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect('SSID', 'PASSWORD')

# Create HMI2 instance
hmi2 = Hmi2()

# Initialize with server IP and memory bank (1-6)
# Auto-update is enabled by default (updates every 50ms)
hmi2.init("192.168.1.66", 1)

# Use the library - updates happen automatically!
hmi2.setBoolean(0, 0, True)
value = hmi2.getBoolean(1, 2)  # Automatically syncs before reading

# No need to call update() - it's automatic!
# You can still call hmi2.update() manually if needed
```

## API Reference

### Initialization

- `init(uart, auto_update=True, update_interval_ms=50)` - Initialize with UART object for serial communication
  - `auto_update`: Enable automatic background updates (default: True)
  - `update_interval_ms`: Minimum time between automatic updates in milliseconds (default: 50)
- `init(ip_address, lan_memory_bank, auto_update=True, update_interval_ms=50)` - Initialize with IP address (string or tuple) and memory bank (1-6) for LAN
- `enableAutoUpdate(enabled, interval_ms=50)` - Enable/disable automatic updates and set interval

### Boolean Operations (B File)

- `getBoolean(word, bit)` - Get boolean value from word, bit position
- `setBoolean(word, bit, value)` - Set boolean value at word, bit position
- `getBFileBit(word, bit)` - Alias for `getBoolean`
- `setBFileBit(word, bit, value)` - Alias for `setBoolean`

### Integer Operations (N File)

- `getInt(word)` - Get 16-bit unsigned integer
- `setInt(word, value)` - Set 16-bit unsigned integer
- `getNFile(word)` - Alias for `getInt`
- `setNFile(word, value)` - Alias for `setInt`

### Double/32-bit Integer Operations (D File)

- `getDInt(word)` - Get 32-bit unsigned integer
- `setDInt(word, value)` - Set 32-bit unsigned integer
- `getDouble(word)` - Alias for `getDInt`
- `setDouble(word, value)` - Alias for `setDInt`

### Float Operations (F File)

- `getFloat(word)` - Get float value
- `setFloat(word, value)` - Set float value
- `getFFile(word)` - Alias for `getFloat`
- `setFFile(word, value)` - Alias for `setFloat`

### Display Operations

- `setDisplayID(lcdID)` - Set current display ID (1-10)
- `setCursor(x, y)` - Set cursor position (x: 0-15, y: 0-1)
- `print(value)` - Print value at current cursor position
- `clearLine0()` - Clear line 0
- `clearLine1()` - Clear line 1

### Communication

- `update()` - Manually update communication and synchronize data (usually not needed - automatic updates handle this)
- `enableAutoUpdate(enabled, interval_ms=50)` - Enable/disable automatic background updates

## Examples

See `example_serial.py` and `example_lan.py` for complete usage examples.

## Notes

- **Automatic Updates**: The library automatically calls `update()` when you read or write data, throttled to prevent excessive communication. No need to call `update()` manually!
- **Manual Updates**: You can still call `update()` manually if needed, or disable auto-update with `enableAutoUpdate(False)`
- File sizes are fixed: B File has 60 words, N/D/F Files have 50 words
- Each B File word contains 16 boolean bits (0-15)
- LAN connection automatically reconnects if disconnected
- The library maintains the same API as the original C++ Arduino library for easy porting
- Auto-update throttling: Updates are throttled to prevent calling `update()` more than once per `update_interval_ms` (default 50ms)

## Platform Compatibility

Tested and designed for:
- ESP32
- ESP8266
- Raspberry Pi Pico (with MicroPython)
- Other MicroPython-compatible boards

## License

This is a port of the original HMI2 Arduino library. Please refer to the original library's license terms.
