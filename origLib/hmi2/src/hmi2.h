#ifndef HMI2_H
#define HMI2_H


#include <Arduino.h>
#include <HardwareSerial.h>


#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#elif defined(__SAM3X8E__)
    #include <Ethernet2.h>
#else
    #include <SoftwareSerial.h>
    #include <Ethernet.h>
#endif




#define BYTE6MASK0 0x3F
#define BYTE6MASK1 0xFC0
#define BYTE6MASK2 0X3F000
#define BYTE6MASK3 0xFC0000
#define BYTE6MASK4 0x3F000000
#define BYTE6MASK5 0xC0000000


#define BYTEMASK0 0xFF
#define BYTEMASK1 0xFF00
#define BYTEMASK2 0xFF0000
#define BYTEMASK3 0xFF000000

#define LDMASK 0x3F
#define MDMASK 0XFC0
#define HDMASK 0xF000

#define HDMASKLDMASK32 0x3F000
#define LDMASK32 0xFC0000
#define MDMASK32 0x3F000000
#define HDMASK32 0xC0000000

#define gmask8 0xFF;
#define gmask16 0xFFFF;

using namespace std;

typedef union{
    float val;
    uint8_t bytes[4];
}floatValue;

enum ConnectionType{HARD_SERIAL, SOFT_SERIAL, LAN};

class Hmi2{
    public:
    

        #if defined(__AVR_ATmega328P__)     //UNO, NANO, LEAONARDO
            int8_t bSize = 20;
            int8_t ndfSize = 20;

            int16_t bFile[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileOver[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileUpdate[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            uint16_t nFile[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean nFileOver[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            boolean nFileUpdate[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            uint32_t dFile[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean dFileOver[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            boolean dFileUpdate[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            float   fFile[20] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
            boolean fFileOver[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            boolean fFileUpdate[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
            int16_t sFile[8] = {0,0,0,0,0,0,0,0};
        #elif defined(__AVR_ATmega2560__)  //MEGA
            int8_t bSize = 60;
            int8_t ndfSize = 50;

            int16_t bFile[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileOver[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileUpdate[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            uint16_t nFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean nFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean nFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            uint32_t dFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean dFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean dFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            float   fFile[50] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
            boolean fFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean fFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            int16_t sFile[8] = {0,0,0,0,0,0,0,0};
        #elif defined(__SAM3X8E__)      //ARDUINO DUE
            int8_t bSize = 60;
            int8_t ndfSize = 50;

            int16_t bFile[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileOver[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileUpdate[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            uint16_t nFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean nFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean nFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            uint32_t dFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean dFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean dFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            float   fFile[50] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
            boolean fFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean fFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            int16_t sFile[8] = {0,0,0,0,0,0,0,0};
        #else                           //ESP32 ESP8266
            int8_t bSize = 60;
            int8_t ndfSize = 50;

            int16_t bFile[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileOver[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            int16_t bFileUpdate[60] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            uint16_t nFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean nFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean nFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            uint32_t dFile[50] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            boolean dFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean dFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            float   fFile[50] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
            boolean fFileOver[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            boolean fFileUpdate[50] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
                                    false, false, false, false, false, false, false, false, false, false};
            int16_t sFile[8] = {0,0,0,0,0,0,0,0};
        #endif

    public:

        #if defined(ESP8266) || defined(ESP32)
            void init(HardwareSerial& mySerial);
            void init(IPAddress server_ip, int lanMemoryBank);
        #elif defined(__SAM3X8E__)
            void init(HardwareSerial& mySerial);
            void init(IPAddress server_ip, int lanMemoryBank);
        #else
            void init(SoftwareSerial& mySerial);
            void init(HardwareSerial& mySerial);
            void init(IPAddress server_ip, int lanMemoryBank);
        #endif


        boolean getBoolean(int word, int bit);
        void setBoolean(int word, int bit, boolean value);

        boolean getBFileBit(int word, int bit);
        void setBFileBit(int word, int bit, boolean value);


        uint16_t getInt(int word);
        void setInt(int word, uint16_t value);

        uint16_t getNFile(int word);
        void setNFile(int word, uint16_t value);
 

        uint32_t getDouble(int word);
        void setDouble(int word, uint32_t value);

        uint32_t getDInt(int word);
        void setDInt(int word, uint32_t value);


        float getFloat(int word);
        void setFloat(int word, float value);

        float getFFile(int word);
        void setFFile(int word, float value);

        void setCursor(uint8_t x, uint8_t y);
        void setDisplayID(uint8_t lcdID);

        void clearLine0();
        void clearLine1();

        void update();

        void print(const char* value);
        void print(boolean value);
        void print(char value);
        void print(unsigned char value);
        void print(unsigned int value);
        void print(int value);
        void print(unsigned long value);
        void print(long value);
        void print(float value);

    
    private:
        boolean inCount;
        boolean syncro;
        boolean overrideSend;

        uint8_t ld;
        uint8_t md;
        uint8_t hd;
        uint8_t ld32;
        uint8_t md32;
        uint8_t hd32;

        uint16_t tempValue;
        uint32_t tempValue32;

        char bufferSerial[128];

        char tempBuffData;

        char lineA[16];
        char lineB[16];
        char lineAPost[16];
        char lineBPost[16];

        int xCursor;
        int yCursor;

        boolean overDisplay;

        uint8_t displayID;

        unsigned long reconectTime;
        unsigned long startTime;
        unsigned long serverTime;
    
        HardwareSerial* myHard;

        #if defined(ESP8266) || defined(ESP32)
            WiFiClient* myLAN;   
        #elif defined(__SAM3X8E__)
            EthernetClient* myLAN;
        #else
            SoftwareSerial* mySoft;
            EthernetClient* myLAN;
        #endif

        
        
        ConnectionType connectionType;

        boolean readBFile(int word, int bit);
        void writeBFile(int word, int bit, boolean value);
        void writeBFile2(int word, int bit, boolean value);

        uint16_t readNFile(int word);
        void writeNFile(int word, uint16_t value);
        void writeNFile2(int word, uint16_t value);

        uint32_t readDFile(int word);
        void writeDFile(int word, uint32_t value);
        void writeDFile2(int word, uint32_t value);

        float readFFile(int word);
        void writeFFile(int word, float value);
        void writeFFile2(int word, float value);

        void fragmentData32(uint32_t tempInt32);
        void fragmentData16(uint16_t tempInt16);
        void fragmentData8(uint32_t tempInt16);
        void fragmentDataFloat(float preFloat);
        uint8_t joinInt8(char tempMd, char tempLd);
        uint16_t joinInt16(char tempHd, char tempMd, char tempLd);
        uint32_t joinInt32(char temp6, char temp5, char temp4, char temp3, char temp2, char temp1);
        float joinFloat(uint32_t tempInt32);

        void setBitWord(int wordPos, int bitPos, boolean value);
        boolean getBitWord(int wordPos, int bitPos);
        void resetBitWordOver(int wordPos, int bitPos);
        boolean getBitWordOver(int wordPos, int bitPos);
        void setBitWordUpdate(int wordPos, int bitPos);
        boolean getBitWordUpdate(int wordPos, int bitPos);
        boolean getNWordOver(int wordPos);
        boolean getDWordOver(int wordPos);
        boolean getFWordOver(int wordPos);
        int setBitToInt(int bitPos);
        int clearBitToInt(int bitPos);

        boolean checkSoftResponse();
        boolean checkHardResponse();
        boolean checkLANResponse();

        boolean sendBasicCommand(char command);

        boolean lanConnectionStatus;

        IPAddress myServer_ip;
        int myPort;
        int myLanSlot;

        boolean lanTimeCount;

        boolean reconnectServer;

        boolean connect2Server();

        void initLCD();
        void writeText2Line(String value);
        void resetPostLines();

        void cleanSoftSerial();
        void cleanHardSerial();
        void cleanLan();
};

//extern Hmi2 hmi2;

#endif
