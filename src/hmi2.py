"""
HMI2 MicroPython Library
A MicroPython port of the Arduino/ESP HMI2 control panel library.
Supports serial (UART) and LAN (socket) communication with HMI control panel app.
"""

import struct
import time
from machine import UART
try:
    import socket
    import network
    LAN_AVAILABLE = True
except ImportError:
    LAN_AVAILABLE = False

# Constants
BYTE6MASK0 = 0x3F
BYTE6MASK1 = 0xFC0
BYTE6MASK2 = 0x3F000
BYTE6MASK3 = 0xFC0000
BYTE6MASK4 = 0x3F000000
BYTE6MASK5 = 0xC0000000

BYTEMASK0 = 0xFF
BYTEMASK1 = 0xFF00
BYTEMASK2 = 0xFF0000
BYTEMASK3 = 0xFF000000

LDMASK = 0x3F
MDMASK = 0xFC0
HDMASK = 0xF000

HDMASKLDMASK32 = 0x3F000
LDMASK32 = 0xFC0000
MDMASK32 = 0x3F000000
HDMASK32 = 0xC0000000

gmask8 = 0xFF
gmask16 = 0xFFFF

# Connection types
HARD_SERIAL = 0
SOFT_SERIAL = 1
LAN = 2


class Hmi2:
    """
    HMI2 Control Panel Library for MicroPython
    
    Supports communication via UART (hardware serial) or LAN (socket).
    Manages boolean, integer, double, and float data files with synchronization.
    Provides display/LCD functionality for text output.
    """
    
    def __init__(self):
        """Initialize HMI2 object with default values."""
        # File sizes - using ESP32 defaults (largest)
        self.bSize = 60
        self.ndfSize = 50
        
        # B File (Boolean) - stored as 16-bit words, each bit is a boolean
        self.bFile = [0] * 60
        self.bFileOver = [0] * 60
        self.bFileUpdate = [0] * 60
        
        # N File (16-bit unsigned integer)
        self.nFile = [0] * 50
        self.nFileOver = [False] * 50
        self.nFileUpdate = [False] * 50
        
        # D File (32-bit unsigned integer)
        self.dFile = [0] * 50
        self.dFileOver = [False] * 50
        self.dFileUpdate = [False] * 50
        
        # F File (float)
        self.fFile = [0.0] * 50
        self.fFileOver = [False] * 50
        self.fFileUpdate = [False] * 50
        
        # S File (16-bit signed integer) - not fully implemented in original
        self.sFile = [0] * 8
        
        # Internal state
        self.inCount = False
        self.syncro = True
        self.overrideSend = False
        self.overDisplay = False
        
        # Data fragmentation variables
        self.ld = 0
        self.md = 0
        self.hd = 0
        self.ld32 = 0
        self.md32 = 0
        self.hd32 = 0
        
        self.tempValue = 0
        self.tempValue32 = 0
        
        # Communication buffer
        self.bufferSerial = bytearray(128)
        
        # Display/LCD state
        self.lineA = bytearray(16)
        self.lineB = bytearray(16)
        self.lineAPost = bytearray(16)
        self.lineBPost = bytearray(16)
        for i in range(16):
            self.lineA[i] = 32  # Space
            self.lineB[i] = 32
            self.lineAPost[i] = 32
            self.lineBPost[i] = 32
        
        self.xCursor = 0
        self.yCursor = 0
        self.displayID = 1
        
        # LAN connection state
        self.reconectTime = 0
        self.startTime = 0
        self.serverTime = 0
        self.lanConnectionStatus = False
        self.lanTimeCount = False
        self.reconnectServer = True
        
        # Connection objects
        self.myHard = None
        self.myLAN = None
        self.connectionType = None
        
        # LAN settings
        self.myServer_ip = None
        self.myPort = 1030
        self.myLanSlot = 1
        
        # Auto-update settings
        self.autoUpdateEnabled = True
        self.updateTimer = None
        self.lastUpdateTime = 0
        self.updateInterval = 50  # milliseconds between updates
        
        # Initialize LCD
        self.initLCD()
    
    def init(self, serial_or_ip, lan_memory_bank=None, auto_update=True, update_interval_ms=50):
        """
        Initialize HMI2 connection.
        
        Args:
            serial_or_ip: UART object for serial, or tuple/string for IP address (LAN)
            lan_memory_bank: Memory bank number (1-6) for LAN connection, ignored for serial
            auto_update: Enable automatic background updates (default: True)
            update_interval_ms: Interval between automatic updates in milliseconds (default: 50)
        """
        if isinstance(serial_or_ip, UART):
            # Hardware serial (UART)
            self.initLCD()
            self.connectionType = HARD_SERIAL
            self.myHard = serial_or_ip
            self.syncro = True
            self.overrideSend = False
            self.overDisplay = False
        elif isinstance(serial_or_ip, (str, tuple, list)):
            # LAN connection
            if not LAN_AVAILABLE:
                raise RuntimeError("LAN support not available. socket module required.")
            
            self.initLCD()
            
            # Parse IP address
            if isinstance(serial_or_ip, str):
                parts = serial_or_ip.split('.')
                self.myServer_ip = tuple(int(x) for x in parts)
            elif isinstance(serial_or_ip, (tuple, list)):
                self.myServer_ip = tuple(serial_or_ip)
            else:
                raise ValueError("Invalid IP address format")
            
            self.myPort = 1030
            
            # Set memory bank
            if lan_memory_bank is None:
                self.myLanSlot = 1
            elif lan_memory_bank < 1:
                self.myLanSlot = 1
            elif lan_memory_bank > 6:
                self.myLanSlot = 6
            else:
                self.myLanSlot = lan_memory_bank
            
            self.connectionType = LAN
            self.myLAN = None
            self.lanConnectionStatus = False
            self.reconnectServer = True
            
            self.connect2Server()
            
            self.syncro = True
            self.overrideSend = False
            self.overDisplay = False
        else:
            raise ValueError("Invalid initialization parameter. Use UART object or IP address.")
        
        # Setup auto-update
        self.autoUpdateEnabled = auto_update
        self.updateInterval = update_interval_ms
        if self.autoUpdateEnabled:
            self._startAutoUpdate()
    
    def connect2Server(self):
        """Connect to LAN server. Returns True if connected."""
        if not self.lanConnectionStatus:
            if not self.reconnectServer:
                if (time.ticks_ms() - self.serverTime) > 3000:
                    self.reconnectServer = True
            
            if self.reconnectServer:
                self.reconnectServer = False
                try:
                    # Create socket address tuple directly
                    addr = ('.'.join(map(str, self.myServer_ip)), self.myPort)
                    self.myLAN = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.myLAN.settimeout(0.2)
                    self.myLAN.connect(addr)
                    self.lanTimeCount = False
                    self.lanConnectionStatus = True
                    return True
                except Exception as e:
                    self.serverTime = time.ticks_ms()
                    if self.myLAN:
                        try:
                            self.myLAN.close()
                        except:
                            pass
                        self.myLAN = None
                    return False
            else:
                return False
        else:
            return True
    
    # Boolean (B File) methods
    def getBoolean(self, word, bit):
        """Get boolean value from B File at word, bit position."""
        self._autoUpdate()
        return self.readBFile(word, bit)
    
    def getBFileBit(self, word, bit):
        """Get boolean value from B File at word, bit position."""
        self._autoUpdate()
        return self.readBFile(word, bit)
    
    def setBoolean(self, word, bit, value):
        """Set boolean value in B File at word, bit position."""
        self.writeBFile(word, bit, value)
        self._autoUpdate()
    
    def setBFileBit(self, word, bit, value):
        """Set boolean value in B File at word, bit position."""
        self.writeBFile(word, bit, value)
        self._autoUpdate()
    
    def readBFile(self, word, bit):
        """Read boolean from B File."""
        if (word >= 0 and word < self.bSize) and (bit >= 0 and bit < 16):
            return self.getBitWord(word, bit)
        return False
    
    def writeBFile(self, word, bit, value):
        """Write boolean to B File."""
        if (word >= 0 and word < self.bSize) and (bit >= 0 and bit < 16):
            if (self.getBitWord(word, bit) != value) or self.getBitWordOver(word, bit):
                self.setBitWord(word, bit, value)
                self.setBitWordUpdate(word, bit)
                self.writeBFile2(word, bit, value)
    
    # Integer (N File) methods
    def getInt(self, word):
        """Get 16-bit unsigned integer from N File."""
        self._autoUpdate()
        return self.readNFile(word)
    
    def getNFile(self, word):
        """Get 16-bit unsigned integer from N File."""
        self._autoUpdate()
        return self.readNFile(word)
    
    def setInt(self, word, value):
        """Set 16-bit unsigned integer in N File."""
        self.writeNFile(word, value)
        self._autoUpdate()
    
    def setNFile(self, word, value):
        """Set 16-bit unsigned integer in N File."""
        self.writeNFile(word, value)
        self._autoUpdate()
    
    def readNFile(self, word):
        """Read 16-bit unsigned integer from N File."""
        if word >= 0 and word < self.ndfSize:
            return self.nFile[word]
        return 0
    
    def writeNFile(self, word, value):
        """Write 16-bit unsigned integer to N File."""
        if word >= 0 and word < self.ndfSize:
            if (self.nFile[word] != value) or self.getNWordOver(word):
                self.nFile[word] = value
                self.nFileUpdate[word] = True
                self.writeNFile2(word, value)
    
    # Double/32-bit Integer (D File) methods
    def getDouble(self, word):
        """Get 32-bit unsigned integer from D File."""
        self._autoUpdate()
        return self.readDFile(word)
    
    def getDInt(self, word):
        """Get 32-bit unsigned integer from D File."""
        self._autoUpdate()
        return self.readDFile(word)
    
    def setDouble(self, word, value):
        """Set 32-bit unsigned integer in D File."""
        self.writeDFile(word, value)
        self._autoUpdate()
    
    def setDInt(self, word, value):
        """Set 32-bit unsigned integer in D File."""
        self.writeDFile(word, value)
        self._autoUpdate()
    
    def readDFile(self, word):
        """Read 32-bit unsigned integer from D File."""
        if word >= 0 and word < self.ndfSize:
            return self.dFile[word]
        return 0
    
    def writeDFile(self, word, value):
        """Write 32-bit unsigned integer to D File."""
        if word >= 0 and word < self.ndfSize:
            if (self.dFile[word] != value) or self.getDWordOver(word):
                self.dFile[word] = value
                self.dFileUpdate[word] = True
                self.writeDFile2(word, value)
    
    # Float (F File) methods
    def getFloat(self, word):
        """Get float value from F File."""
        self._autoUpdate()
        return self.readFFile(word)
    
    def getFFile(self, word):
        """Get float value from F File."""
        self._autoUpdate()
        return self.readFFile(word)
    
    def setFloat(self, word, value):
        """Set float value in F File."""
        self.writeFFile(word, value)
        self._autoUpdate()
    
    def setFFile(self, word, value):
        """Set float value in F File."""
        self.writeFFile(word, value)
        self._autoUpdate()
    
    def readFFile(self, word):
        """Read float from F File."""
        if word >= 0 and word < self.ndfSize:
            return self.fFile[word]
        return 0.0
    
    def writeFFile(self, word, value):
        """Write float to F File."""
        if word >= 0 and word < self.ndfSize:
            if (self.fFile[word] != value) or self.getFWordOver(word):
                self.fFile[word] = value
                self.fFileUpdate[word] = True
                self.writeFFile2(word, value)
    
    # Display/LCD methods
    def setCursor(self, x, y):
        """Set cursor position for display."""
        self.xCursor = x
        self.yCursor = y
    
    def setDisplayID(self, lcdID):
        """Set current display ID (1-10)."""
        if lcdID < 1:
            self.displayID = 1
        elif lcdID > 10:
            self.displayID = 10
        else:
            self.displayID = lcdID
    
    def clearLine0(self):
        """Clear line 0 of display."""
        for i in range(16):
            self.lineA[i] = 32  # Space
    
    def clearLine1(self):
        """Clear line 1 of display."""
        for i in range(16):
            self.lineB[i] = 32  # Space
    
    def resetPostLines(self):
        """Reset post lines for change detection."""
        for i in range(16):
            self.lineAPost[i] = 32
            self.lineBPost[i] = 32
    
    def print(self, value):
        """Print value to display at current cursor position."""
        self.writeText2Line(str(value))
        self._autoUpdate()
    
    def initLCD(self):
        """Initialize LCD display state."""
        self.clearLine0()
        self.clearLine1()
        self.xCursor = 0
        self.yCursor = 0
        self.displayID = 1
    
    def writeText2Line(self, value):
        """Write text to display line."""
        okRX = False
        
        if self.xCursor >= 0 and self.xCursor < 16:
            if self.yCursor == 0 or self.yCursor == 1:
                tempSize = len(value)
                if tempSize > 0:
                    for i in range(tempSize):
                        if self.yCursor == 0:
                            if self.xCursor < 16:
                                self.lineA[self.xCursor] = ord(value[i]) if i < len(value) else 32
                        else:
                            if self.xCursor < 16:
                                self.lineB[self.xCursor] = ord(value[i]) if i < len(value) else 32
                        
                        self.xCursor += 1
                        if self.xCursor >= 16:
                            break
                    
                    # Check for changes
                    for i in range(16):
                        if self.yCursor == 0:
                            if self.lineA[i] != self.lineAPost[i]:
                                okRX = True
                                self.lineAPost[i] = self.lineA[i]
                        else:
                            if self.lineB[i] != self.lineBPost[i]:
                                okRX = True
                                self.lineBPost[i] = self.lineB[i]
        
        if okRX or self.overDisplay:
            if self.connectionType == HARD_SERIAL:
                data = bytearray([64, ord('k')])
                for i in range(16):
                    if self.yCursor == 0:
                        self.fragmentData8(self.lineA[i])
                    else:
                        self.fragmentData8(self.lineB[i])
                    data.append(self.md)
                    data.append(self.ld)
                data.append(self.displayID)
                if self.yCursor == 0:
                    data.append(49)  # '1'
                else:
                    data.append(48)  # '0'
                data.append(98)  # 'b'
                self.myHard.write(data)
                self.checkHardResponse()
            
            elif self.connectionType == LAN:
                if self.connect2Server():
                    data = bytearray([self.myLanSlot, 64, ord('k')])
                    for i in range(16):
                        if self.yCursor == 0:
                            self.fragmentData8(self.lineA[i])
                        else:
                            self.fragmentData8(self.lineB[i])
                        data.append(self.md)
                        data.append(self.ld)
                    data.append(self.displayID)
                    if self.yCursor == 0:
                        data.append(49)
                    else:
                        data.append(48)
                    data.append(98)
                    self.myLAN.send(data)
                    self.checkLANResponse()
    
    # Auto-update methods
    def _startAutoUpdate(self):
        """Start automatic background updates using a timer."""
        try:
            # Try to use Timer for periodic updates
            # Note: Timer implementation varies by platform
            # For ESP32/ESP8266, we'll use a different approach
            self.lastUpdateTime = time.ticks_ms()
        except:
            pass
    
    def _autoUpdate(self):
        """Automatically call update() if enough time has passed."""
        if not self.autoUpdateEnabled:
            return
        
        currentTime = time.ticks_ms()
        if time.ticks_diff(currentTime, self.lastUpdateTime) >= self.updateInterval:
            self.lastUpdateTime = currentTime
            try:
                self.update()
            except:
                pass  # Silently fail to avoid breaking user code
    
    def enableAutoUpdate(self, enabled=True, interval_ms=50):
        """
        Enable or disable automatic updates.
        
        Args:
            enabled: True to enable auto-update, False to disable
            interval_ms: Update interval in milliseconds (only used when enabling)
        """
        self.autoUpdateEnabled = enabled
        self.updateInterval = interval_ms
        if enabled:
            self._startAutoUpdate()
    
    # Update and synchronization
    def update(self):
        """Update communication and synchronize data with HMI app."""
        okData = False
        update2Android = False
        
        if self.syncro:
            okData = self.sendBasicCommand('a')
        else:
            okData = self.sendBasicCommand('e')
        
        if okData:
            if self.bufferSerial[0] == ord('c'):
                readingData = True
                
                while readingData:
                    okData = self.sendBasicCommand('c')
                    
                    if okData:
                        cmd = self.bufferSerial[0]
                        if cmd == 65:  # BINARY
                            if self.bufferSerial[1] < self.bSize:
                                if self.bufferSerial[3] == ord('1'):
                                    self.setBitWord(self.bufferSerial[1], self.bufferSerial[2], True)
                                else:
                                    self.setBitWord(self.bufferSerial[1], self.bufferSerial[2], False)
                        elif cmd == 75:  # INT
                            if self.bufferSerial[1] < self.ndfSize:
                                self.nFile[self.bufferSerial[1]] = self.joinInt16(
                                    self.bufferSerial[2], self.bufferSerial[3], self.bufferSerial[4])
                        elif cmd == 77:  # DINT
                            if self.bufferSerial[1] < self.ndfSize:
                                self.dFile[self.bufferSerial[1]] = self.joinInt32(
                                    self.bufferSerial[2], self.bufferSerial[3], self.bufferSerial[4],
                                    self.bufferSerial[5], self.bufferSerial[6], self.bufferSerial[7])
                        elif cmd == 79:  # REAL
                            if self.bufferSerial[1] < self.ndfSize:
                                preFloat = self.joinInt32(
                                    self.bufferSerial[2], self.bufferSerial[3], self.bufferSerial[4],
                                    self.bufferSerial[5], self.bufferSerial[6], self.bufferSerial[7])
                                self.fFile[self.bufferSerial[1]] = self.joinFloat(preFloat)
                        elif cmd == 100:
                            readingData = False
                            self.syncro = False
                        elif cmd == 102:
                            self.overrideSend = True
                        elif cmd == 103:
                            readingData = False
                            self.syncro = False
                            update2Android = True
                    else:
                        readingData = False
            elif self.bufferSerial[0] == ord('d'):
                self.syncro = False
        
        if self.overDisplay:
            self.overDisplay = False
        
        if self.overrideSend:
            self.overrideSend = False
            self.overDisplay = True
            
            for i in range(self.bSize):
                self.bFileOver[i] = -1
            
            for i in range(self.ndfSize):
                self.nFileOver[i] = True
                self.dFileOver[i] = True
                self.fFileOver[i] = True
        
        if update2Android:
            for i in range(self.bSize):
                if self.bFileUpdate[i] != 0:
                    for j in range(16):
                        if self.getBitWordUpdate(i, j):
                            self.writeBFile2(i, j, self.getBitWord(i, j))
            
            for i in range(self.ndfSize):
                if self.nFileUpdate[i]:
                    self.writeNFile2(i, self.nFile[i])
                
                if self.dFileUpdate[i]:
                    self.writeDFile2(i, self.dFile[i])
                
                if self.fFileUpdate[i]:
                    self.writeFFile2(i, self.fFile[i])
    
    # Communication methods
    def sendBasicCommand(self, command):
        """Send basic command to HMI."""
        okData = False
        
        if self.connectionType == HARD_SERIAL:
            self.myHard.write(bytearray([64, ord(command), 98]))  # '@', command, 'b'
            okData = self.checkHardResponse()
        elif self.connectionType == LAN:
            if self.connect2Server():
                self.myLAN.send(bytearray([self.myLanSlot, 64, ord(command), 98]))
                okData = self.checkLANResponse()
        
        return okData
    
    def writeBFile2(self, word, bit, value):
        """Write boolean to HMI via communication."""
        if self.connectionType == HARD_SERIAL:
            data = bytearray([64, ord('C'), word, bit])
            if value:
                data.append(49)  # '1'
            else:
                data.append(48)  # '0'
            data.append(98)  # 'b'
            self.myHard.write(data)
            self.checkHardResponse()
        elif self.connectionType == LAN:
            if self.connect2Server():
                data = bytearray([self.myLanSlot, 64, ord('C'), word, bit])
                if value:
                    data.append(49)
                else:
                    data.append(48)
                data.append(98)
                self.myLAN.send(data)
                self.checkLANResponse()
    
    def writeNFile2(self, word, value):
        """Write integer to HMI via communication."""
        self.fragmentData16(value)
        
        if self.connectionType == HARD_SERIAL:
            self.myHard.write(bytearray([64, ord('L'), word, self.hd, self.md, self.ld, 98]))
            self.checkHardResponse()
        elif self.connectionType == LAN:
            if self.connect2Server():
                self.myLAN.send(bytearray([self.myLanSlot, 64, ord('L'), word, self.hd, self.md, self.ld, 98]))
                self.checkLANResponse()
    
    def writeDFile2(self, word, value):
        """Write double/32-bit integer to HMI via communication."""
        self.fragmentData32(value)
        
        if self.connectionType == HARD_SERIAL:
            self.myHard.write(bytearray([
                64, ord('N'), word, self.hd32, self.md32, self.ld32, self.hd, self.md, self.ld, 98
            ]))
            self.checkHardResponse()
        elif self.connectionType == LAN:
            if self.connect2Server():
                self.myLAN.send(bytearray([
                    self.myLanSlot, 64, ord('N'), word, self.hd32, self.md32, self.ld32,
                    self.hd, self.md, self.ld, 98
                ]))
                self.checkLANResponse()
    
    def writeFFile2(self, word, value):
        """Write float to HMI via communication."""
        self.fragmentDataFloat(value)
        
        if self.connectionType == HARD_SERIAL:
            self.myHard.write(bytearray([
                64, ord('P'), word, self.hd32, self.md32, self.ld32, self.hd, self.md, self.ld, 98
            ]))
            self.checkHardResponse()
        elif self.connectionType == LAN:
            if self.connect2Server():
                self.myLAN.send(bytearray([
                    self.myLanSlot, 64, ord('P'), word, self.hd32, self.md32, self.ld32,
                    self.hd, self.md, self.ld, 98
                ]))
                self.checkLANResponse()
    
    def checkHardResponse(self):
        """Check response from hardware serial."""
        okData = False
        self.inCount = True
        self.startTime = time.ticks_ms()
        
        while self.inCount:
            if self.myHard.any():
                self.inCount = False
                data = self.myHard.read(128)
                if data:
                    idx = 0
                    for byte in data:
                        if idx < len(self.bufferSerial):
                            self.bufferSerial[idx] = byte
                            idx += 1
                            if byte == 98:  # 'b'
                                break
                    if idx > 0:
                        okData = True
            
            if (time.ticks_ms() - self.startTime) > 900:
                self.inCount = False
        
        self.cleanHardSerial()
        return okData
    
    def checkLANResponse(self):
        """Check response from LAN connection."""
        okData = False
        self.inCount = True
        self.startTime = time.ticks_ms()
        
        while self.inCount:
            try:
                if self.myLAN:
                    self.myLAN.settimeout(0.1)
                    data = self.myLAN.recv(128)
                    if data:
                        self.inCount = False
                        idx = 0
                        for byte in data:
                            if idx < len(self.bufferSerial):
                                self.bufferSerial[idx] = byte
                                idx += 1
                                if byte == 98:  # 'b'
                                    break
                        if idx > 0:
                            self.lanTimeCount = False
                            okData = True
            except:
                pass
            
            if (time.ticks_ms() - self.startTime) > 900:
                self.inCount = False
                if not self.lanTimeCount:
                    self.lanTimeCount = True
                    self.reconectTime = time.ticks_ms()
        
        self.cleanLan()
        
        if self.lanTimeCount:
            if (time.ticks_ms() - self.reconectTime) > 3000:
                try:
                    if self.myLAN:
                        self.myLAN.close()
                except:
                    pass
                self.myLAN = None
                self.lanConnectionStatus = False
                self.lanTimeCount = False
        
        return okData
    
    def cleanHardSerial(self):
        """Clean hardware serial buffer."""
        if self.myHard.any():
            self.myHard.read(self.myHard.any())
    
    def cleanLan(self):
        """Clean LAN buffer."""
        try:
            if self.myLAN:
                self.myLAN.settimeout(0.01)
                try:
                    self.myLAN.recv(128)
                except:
                    pass
        except:
            pass
    
    # Bit manipulation methods
    def setBitWord(self, wordPos, bitPos, value):
        """Set bit in word."""
        temp = self.bFile[wordPos]
        
        if value:
            temp |= self.setBitToInt(bitPos)
        else:
            temp &= self.clearBitToInt(bitPos)
        
        temp &= gmask16
        self.bFile[wordPos] = temp
    
    def getBitWord(self, wordPos, bitPos):
        """Get bit from word."""
        tempInt = self.bFile[wordPos]
        tempInt >>= bitPos
        tempInt &= 1
        return tempInt == 1
    
    def setBitWordUpdate(self, wordPos, bitPos):
        """Set update flag for bit."""
        temp = self.bFileUpdate[wordPos]
        temp |= self.setBitToInt(bitPos)
        temp &= gmask16
        self.bFileUpdate[wordPos] = temp
    
    def getBitWordUpdate(self, wordPos, bitPos):
        """Get update flag for bit."""
        tempInt = self.bFileUpdate[wordPos]
        tempInt >>= bitPos
        tempInt &= 1
        return tempInt == 1
    
    def resetBitWordOver(self, wordPos, bitPos):
        """Reset override flag for bit."""
        temp = self.bFileOver[wordPos]
        temp &= self.clearBitToInt(bitPos)
        temp &= gmask16
        self.bFileOver[wordPos] = temp
    
    def getBitWordOver(self, wordPos, bitPos):
        """Get override flag for bit."""
        tempInt = self.bFileOver[wordPos]
        tempInt >>= bitPos
        tempInt &= 1
        
        if tempInt == 1:
            self.resetBitWordOver(wordPos, bitPos)
            return True
        
        return False
    
    def getNWordOver(self, wordPos):
        """Get override flag for N word."""
        temp = self.nFileOver[wordPos]
        if temp:
            self.nFileOver[wordPos] = False
        return temp
    
    def getDWordOver(self, wordPos):
        """Get override flag for D word."""
        temp = self.dFileOver[wordPos]
        if temp:
            self.dFileOver[wordPos] = False
        return temp
    
    def getFWordOver(self, wordPos):
        """Get override flag for F word."""
        temp = self.fFileOver[wordPos]
        if temp:
            self.fFileOver[wordPos] = False
        return temp
    
    def setBitToInt(self, bitPos):
        """Convert bit position to integer mask."""
        temp = 1
        if bitPos > 0:
            temp <<= bitPos
        return temp
    
    def clearBitToInt(self, bitPos):
        """Create mask to clear bit."""
        temp = 0xFFFE
        if bitPos > 0:
            for i in range(bitPos):
                temp <<= 1
                temp += 1
        return temp
    
    # Data fragmentation and joining methods
    def fragmentData32(self, tempInt32):
        """Fragment 32-bit integer into 6-bit parts."""
        self.ld = tempInt32 & BYTE6MASK0
        
        self.tempValue32 = tempInt32 & BYTE6MASK1
        self.md = self.tempValue32 >> 6
        
        self.tempValue32 = tempInt32 & BYTE6MASK2
        self.hd = self.tempValue32 >> 12
        
        self.tempValue32 = tempInt32 & BYTE6MASK3
        self.ld32 = self.tempValue32 >> 18
        
        self.tempValue32 = tempInt32 & BYTE6MASK4
        self.md32 = self.tempValue32 >> 24
        
        self.tempValue32 = tempInt32 & BYTE6MASK5
        self.hd32 = self.tempValue32 >> 30
    
    def fragmentData16(self, tempInt16):
        """Fragment 16-bit integer into 6-bit parts."""
        self.ld = tempInt16 & LDMASK
        
        self.tempValue = tempInt16 & MDMASK
        self.md = self.tempValue >> 6
        
        self.tempValue = tempInt16 & HDMASK
        self.hd = self.tempValue >> 12
    
    def fragmentData8(self, tempInt16):
        """Fragment 8-bit value into 6-bit parts."""
        self.ld = tempInt16 & LDMASK
        
        self.tempValue = tempInt16 & MDMASK
        self.md = self.tempValue >> 6
    
    def fragmentDataFloat(self, preFloat):
        """Fragment float into 32-bit integer parts."""
        # Convert float to bytes
        float_bytes = struct.pack('>f', preFloat)
        tempUint32 = struct.unpack('>I', float_bytes)[0]
        
        self.fragmentData32(tempUint32)
    
    def joinInt8(self, tempMd, tempLd):
        """Join two 6-bit parts into 8-bit integer."""
        tempLd1 = tempLd
        tempMd1 = tempMd << 6
        temp = tempLd1 | tempMd1
        temp &= gmask8
        return temp
    
    def joinInt16(self, tempHd, tempMd, tempLd):
        """Join three 6-bit parts into 16-bit integer."""
        tempLd1 = tempLd
        tempMd1 = tempMd << 6
        tempHd1 = tempHd << 12
        temp = tempLd1 | tempMd1 | tempHd1
        temp &= gmask16
        return temp
    
    def joinInt32(self, temp6, temp5, temp4, temp3, temp2, temp1):
        """Join six 6-bit parts into 32-bit integer."""
        tempB = (temp6 << 30) | (temp5 << 24) | (temp4 << 18) | (temp3 << 12) | (temp2 << 6) | temp1
        return tempB
    
    def joinFloat(self, tempInt32):
        """Join 32-bit integer into float."""
        float_bytes = struct.pack('>I', tempInt32)
        temp = struct.unpack('>f', float_bytes)[0]
        return temp
