#include "hmi2.h"
#include "Arduino.h"

#if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
void Hmi2::init(SoftwareSerial& mySerial){
    initLCD();
    connectionType = SOFT_SERIAL;
    mySoft = &mySerial;
    syncro = true;
    overrideSend = false;
    overDisplay = false;
}
#endif
    
void Hmi2::init(HardwareSerial& mySerial){
    initLCD();
    connectionType = HARD_SERIAL;
    myHard = &mySerial;
    syncro = true;
    overrideSend = false;
    overDisplay = false;
}

void Hmi2::init(IPAddress server_ip, int lanMemoryBank){

    initLCD();
    myServer_ip = server_ip;
    myPort = 1030;

    if(lanMemoryBank < 1)
        myLanSlot = 1;
    else if(lanMemoryBank > 6)
        myLanSlot = 6;
    else
        myLanSlot = lanMemoryBank;

    connectionType = LAN;

    #if defined(ESP8266) || defined(ESP32)
        myLAN = new WiFiClient();
        myLAN->setTimeout(200);  
    #elif defined(__SAM3X8E__)
        myLAN = new EthernetClient();
        myLAN->setTimeout(200);
    #else
        myLAN = new EthernetClient();
        myLAN->setConnectionTimeout(200);
    #endif

    
    lanConnectionStatus = false;
    reconnectServer = true;


    connect2Server();

    syncro = true;
    overrideSend = false;
    overDisplay = false;
}

boolean Hmi2::connect2Server(){
    if(!lanConnectionStatus){

        if(!reconnectServer){
            if((millis() - serverTime) > 3000)    
                reconnectServer = true;
        }

        if(reconnectServer){
            reconnectServer = false;
            if(myLAN->connect(myServer_ip, myPort)){
                lanTimeCount = false;
                lanConnectionStatus = true;
                return true;
            }
            else{
                serverTime = millis();
                return false;
            }
            
        }
        else
            return false;

    }
    else
        return true;
}

boolean Hmi2::getBoolean(int word, int bit){
    return readBFile(word, bit);
}

boolean Hmi2::getBFileBit(int word, int bit){
    return readBFile(word, bit);
}

void Hmi2::setBoolean(int word, int bit, boolean value){
    writeBFile(word, bit, value);
}

void Hmi2::setBFileBit(int word, int bit, boolean value){
    writeBFile(word, bit, value);
}

uint16_t Hmi2::getInt(int word){
    return readNFile(word);
}

uint16_t Hmi2::getNFile(int word){
    return readNFile(word);
}

void Hmi2::setInt(int word, uint16_t value){
    writeNFile(word, value);
}

void Hmi2::setNFile(int word, uint16_t value){
    writeNFile(word, value);
}

uint32_t Hmi2::getDouble(int word){
    return readDFile(word);
}

uint32_t Hmi2::getDInt(int word){
    return readDFile(word);
}

void Hmi2::setDouble(int word, uint32_t value){
    writeDFile(word, value);
}

void Hmi2::setDInt(int word, uint32_t value){
    writeDFile(word, value);
}

float Hmi2::getFloat(int word){
    return readFFile(word);
}

float Hmi2::getFFile(int word){
    return readFFile(word);
}

void Hmi2::setFloat(int word, float value){
    writeFFile(word, value);
}

void Hmi2::setFFile(int word, float value){
    writeFFile(word, value);
}


boolean Hmi2::readBFile(int word, int bit){
    boolean temp = false;

    if((word >= 0 && word < bSize) && (bit >= 0 && bit <16))
        temp = getBitWord(word, bit);

    return temp;
}

void Hmi2::writeBFile(int word, int bit, boolean value){

    if((word >= 0 && word < bSize) && (bit >= 0 && bit <16)){
        if((getBitWord(word, bit) != value) || getBitWordOver(word, bit)){ 

            setBitWord(word, bit, value);
            setBitWordUpdate(word, bit);

            writeBFile2(word, bit, value);

        }//END if
    }//END if(word)

}

uint16_t Hmi2::readNFile(int word){
    uint16_t temp = 0;

    if(word >= 0 && word < ndfSize)
        temp = nFile[word];

    return temp;
}

void Hmi2::writeNFile(int word, uint16_t value){

    if(word >= 0 && word < ndfSize){
        if((nFile[word] != value) || (getNWordOver(word))){

            nFile[word] = value;
            nFileUpdate[word] = true;

            writeNFile2(word, value);
        }
    }//END if(word)
}

uint32_t Hmi2::readDFile(int word){
    uint32_t temp = 0;

    if(word >= 0 && word < ndfSize)
        temp = dFile[word];

    return temp;
}

void Hmi2::writeDFile(int word, uint32_t value){

    if(word >= 0 && word < ndfSize){
        if((dFile[word] != value) || (getDWordOver(word))){

            dFile[word] = value;
            dFileUpdate[word] = true;

            writeDFile2(word, value);

        }
    }//ENF if(word)
}

float Hmi2::readFFile(int word){
    float temp = 0;

    if(word >= 0 && word < ndfSize)
        temp = fFile[word];
    
    return temp;
}

void Hmi2::writeFFile(int word, float value){

    if(word >= 0 && word < ndfSize){
        if((fFile[word] != value) || (getFWordOver(word))){

            fFile[word] = value;
            fFileUpdate[word] = true;

            writeFFile2(word, value);
        }
    }//END if(word)
}


void Hmi2::update(){

    boolean okData = false;
    boolean update2Android = false;

    if(syncro)
        okData = sendBasicCommand('a');
    else
        okData = sendBasicCommand('e');


    if(okData){
        if(bufferSerial[0] == 'c'){

            boolean readingData =  true;

            while(readingData){
                okData = sendBasicCommand('c');

                if(okData){
                    switch(bufferSerial[0]){
                        case 65:            //BINARY
                            if(bufferSerial[1] < bSize){
                                if(bufferSerial[3] == '1')
                                    setBitWord(bufferSerial[1], bufferSerial[2], true);
                                else
                                    setBitWord(bufferSerial[1], bufferSerial[2], false);
                            }
                        break;
                        case 75:            //INT
                            if(bufferSerial[1] < ndfSize)
                                nFile[bufferSerial[1]] = joinInt16(bufferSerial[2], bufferSerial[3], bufferSerial[4]);
                        break;
                        case 77:            //DINT
                            if(bufferSerial[1] < ndfSize)
                                dFile[bufferSerial[1]] = joinInt32(bufferSerial[2], bufferSerial[3], bufferSerial[4], bufferSerial[5], bufferSerial[6], bufferSerial[7]);
                        break;
                        case 79:            //REAL
                            if(bufferSerial[1] < ndfSize){
                                uint32_t preFloat = joinInt32(bufferSerial[2], bufferSerial[3], bufferSerial[4], bufferSerial[5], bufferSerial[6], bufferSerial[7]);
                                fFile[bufferSerial[1]] = joinFloat(preFloat);
                            }
                        break;
                        case 100:
                            readingData = false;
                            syncro = false;
                        break;
                        case 102:
                            overrideSend = true;
                        break;
                        case 103:
                            readingData = false;
                            syncro = false;
                            update2Android = true;
                        break;
                    }
                }
                else
                    readingData = false;
            }
        }
        else if(bufferSerial[0] == 'd')
            syncro = false;
    }

    if(overDisplay)
        overDisplay = false;

    if(overrideSend){
        overrideSend = false;
        overDisplay = true;

        for(int i = 0; i < bSize; i++)
            bFileOver[i] = -1;
        
        for(int i = 0; i < ndfSize; i++){
            nFileOver[i] = true;
            dFileOver[i] = true;
            fFileOver[i] = true;
        }
    }

    if(update2Android){
        
        for(int i = 0; i < bSize; i++){
            if(bFileUpdate[i] != 0){
                for(int j = 0; j < 16; j++){
                    if(getBitWordUpdate(i, j))
                        writeBFile2(i, j, getBitWord(i,j));
                }//END for(int j)
            }//END if
        }//END for(int i)

        for(int i = 0; i < ndfSize; i++){
            if(nFileUpdate[i])
                writeNFile2(i, nFile[i]);
 
            if(dFileUpdate[i])
                writeDFile2(i, dFile[i]);

            if(fFileUpdate[i])
                writeFFile2(i, fFile[i]);
    
        }//END for(int i)

    }
}

void Hmi2::setCursor(uint8_t x, uint8_t y){
    xCursor = x;
    yCursor = y;
}

void Hmi2::setDisplayID(uint8_t lcdID){
    if(lcdID < 1)
        displayID = 1;
    else if(lcdID > 10)
        displayID = 10;
    else
        displayID = lcdID;
}


void Hmi2::clearLine0(){
    for(int i = 0; i < 16; i++)
        lineA[i] = 32;
}


void Hmi2::clearLine1(){
    for(int i = 0; i < 16; i++)
        lineB[i] = 32;
}

void Hmi2::resetPostLines(){
    for(int i = 0; i < 16; i++){
        lineAPost[i] = 32;
        lineBPost[i] = 32;
    }
}

void Hmi2::print(const char* value){
    writeText2Line(String(value));
}

void Hmi2::print(boolean value){
    writeText2Line(String(value));
}
    
void Hmi2::print(char value){
    writeText2Line(String(value));
}

void Hmi2::print(unsigned char value){
    writeText2Line(String(value));
}

void Hmi2::print(unsigned int value){
    writeText2Line(String(value));
}

void Hmi2::print(int value){
    writeText2Line(String(value));
}

void Hmi2::print(unsigned long value){
    writeText2Line(String(value));
}
    
void Hmi2::print(long value){
    writeText2Line(String(value));
}

void Hmi2::print(float value){
    writeText2Line(String(value));
}

void Hmi2::writeText2Line(String value){

    boolean okRX = false;

    if(xCursor >= 0 && xCursor < 16){
        if(yCursor == 0 || yCursor == 1){
             int tempSize = value.length();
             if(tempSize > 0){
                 for(int i = 0; i < tempSize; i++){
                    if(yCursor == 0)
                        lineA[xCursor] = value.charAt(i);
                    else
                        lineB[xCursor] = value.charAt(i);

                    xCursor++;

                    if(xCursor >= 16)                    
                        break;

                 }//END for

                for(int i = 0; i < 16; i++){
                    if(yCursor == 0){
                        if(lineA[i] != lineAPost[i]){
                            okRX = true;
                            lineAPost[i] = lineA[i];
                        }
                    }
                    else{
                        if(lineB[i] != lineBPost[i]){
                            okRX = true;
                            lineBPost[i] = lineB[i];
                        }
                    }
                }
             }//END if(tempSize > 0)
        }//END if(yCursor)
    }//END if(xCursor)

    if(okRX || overDisplay){
        switch(connectionType){
            case SOFT_SERIAL:
                #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                    mySoft->write(64);
                    mySoft->write('k');
                
                    for(int i = 0; i < 16; i++){
                        if(yCursor == 0){
                            fragmentData8(lineA[i]);
                            mySoft->write(md);
                            mySoft->write(ld);
                        }
                        else{
                            fragmentData8(lineB[i]);
                            mySoft->write(md);
                            mySoft->write(ld);
                        }
                    }

                    mySoft->write((int8_t)(displayID));

                    if(yCursor == 0)
                        mySoft->write(49);
                    else
                        mySoft->write(48);

                    mySoft->write(98);

                    mySoft->flush();

                    checkSoftResponse();
                #endif
            break;
            case HARD_SERIAL:
                myHard->write(64);
                myHard->write('k');

                for(int i = 0; i < 16; i++){
                    if(yCursor == 0){
                        fragmentData8(lineA[i]);
                        myHard->write(md);
                        myHard->write(ld);
                    }
                    else{
                        fragmentData8(lineB[i]);
                        myHard->write(md);
                        myHard->write(ld);
                    }
                }

                myHard->write((int8_t)(displayID));

                if(yCursor == 0)
                    myHard->write(49);
                else
                    myHard->write(48);

                myHard->write(98);

                myHard->flush();

                checkHardResponse();
            break;
            case LAN:
                if(connect2Server()){
                    myLAN->write((int8_t)myLanSlot);
                    myLAN->write(64);
                    myLAN->write('k');
    
                    for(int i = 0; i < 16; i++){
                        if(yCursor == 0){
                            fragmentData8(lineA[i]);
                            myLAN->write(md);
                            myLAN->write(ld);
                        }
                        else{
                            fragmentData8(lineB[i]);
                            myLAN->write(md);
                            myLAN->write(ld);
                        }
                    }

                    myLAN->write((int8_t)(displayID));

                    if(yCursor == 0)
                        myLAN->write(49);
                    else
                        myLAN->write(48);

                    myLAN->write(98);

                    myLAN->flush();

                    checkLANResponse();
                }
            break;
        }//END switch
    }//END if(okRX)

}

void Hmi2::initLCD(){
    clearLine0();
    clearLine1();
    xCursor = 0;
    yCursor = 0;
    displayID = 1;
}

void Hmi2::writeBFile2(int word, int bit, boolean value){
    switch(connectionType){
        case SOFT_SERIAL:
            #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                mySoft->write(64);
                mySoft->write('C');
                mySoft->write((int8_t)word);
                mySoft->write((int8_t)bit);

                if(value)
                    mySoft->write(49);
                else
                    mySoft->write(48);

                mySoft->write(98);

                mySoft->flush();

                checkSoftResponse();
            #endif
            break;
        case HARD_SERIAL:
            myHard->write(64);
            myHard->write('C');
            myHard->write((int8_t)word);
            myHard->write((int8_t)bit);

            if(value)
                myHard->write(49);
            else
                myHard->write(48);

            myHard->write(98);

            myHard->flush();

            checkHardResponse();
            break;
        case LAN:
            if(connect2Server()){
                myLAN->write((int8_t)myLanSlot);
                myLAN->write(64);
                myLAN->write('C');
                myLAN->write((int8_t)word);
                myLAN->write((int8_t)bit);

                if(value)
                    myLAN->write(49);
                else
                    myLAN->write(48);

                myLAN->write(98);

                myLAN->flush();

                checkLANResponse();
            }

            break;
    }//END switch
}

void Hmi2::writeNFile2(int word, uint16_t value){
    fragmentData16(value);

    switch(connectionType){
        case SOFT_SERIAL:
            #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                mySoft->write(64);
                mySoft->write('L');
                mySoft->write((int8_t)word);
                mySoft->write(hd);
                mySoft->write(md);
                mySoft->write(ld);
                mySoft->write(98);

                mySoft->flush();

                checkSoftResponse();
            #endif
            break;
        case HARD_SERIAL:
            myHard->write(64);
            myHard->write('L');
            myHard->write((int8_t)word);
            myHard->write(hd);
            myHard->write(md);
            myHard->write(ld);
            myHard->write(98);

            myHard->flush();

            checkHardResponse();
            break;
        case LAN:
            if(connect2Server()){
                myLAN->write((int8_t)myLanSlot);
                myLAN->write(64);
                myLAN->write('L');
                myLAN->write((int8_t)word);
                myLAN->write(hd);
                myLAN->write(md);
                myLAN->write(ld);
                myLAN->write(98);

                myLAN->flush();

                checkLANResponse();
            }
            break;
    }
}
        
void Hmi2::writeDFile2(int word, uint32_t value){
    fragmentData32(value);

    switch(connectionType){
        case SOFT_SERIAL:
            #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                mySoft->write(64);
                mySoft->write('N');
                mySoft->write((int8_t)word);
                mySoft->write(hd32);
                mySoft->write(md32);
                mySoft->write(ld32);
                mySoft->write(hd);
                mySoft->write(md);
                mySoft->write(ld);
                mySoft->write(98);

                mySoft->flush();

                checkSoftResponse();
            #endif
            break;
        case HARD_SERIAL:
            myHard->write(64);
            myHard->write('N');
            myHard->write((int8_t)word);
            myHard->write(hd32);
            myHard->write(md32);
            myHard->write(ld32);
            myHard->write(hd);
            myHard->write(md);
            myHard->write(ld);
            myHard->write(98);

            myHard->flush();

            checkHardResponse();
            break;
        case LAN:
            if(connect2Server()){
                myLAN->write((int8_t)myLanSlot);
                myLAN->write(64);
                myLAN->write('N');
                myLAN->write((int8_t)word);
                myLAN->write(hd32);
                myLAN->write(md32);
                myLAN->write(ld32);
                myLAN->write(hd);
                myLAN->write(md);
                myLAN->write(ld);
                myLAN->write(98);

                myLAN->flush();

                checkLANResponse();
            }
            break;
    }
}
        
void Hmi2::writeFFile2(int word, float value){
    fragmentDataFloat(value);

    switch(connectionType){
        case SOFT_SERIAL:
            #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                mySoft->write(64);
                mySoft->write('P');
                mySoft->write((int8_t)word);
                mySoft->write(hd32);
                mySoft->write(md32);
                mySoft->write(ld32);
                mySoft->write(hd);
                mySoft->write(md);
                mySoft->write(ld);
                mySoft->write(98);

                mySoft->flush();

                checkSoftResponse();
            #endif
            break;
        case HARD_SERIAL:
            myHard->write(64);
            myHard->write('P');
            myHard->write((int8_t)word);
            myHard->write(hd32);
            myHard->write(md32);
            myHard->write(ld32);
            myHard->write(hd);
            myHard->write(md);
            myHard->write(ld);
            myHard->write(98);

            myHard->flush();

            checkHardResponse();
            break;
        case LAN:
            if(connect2Server()){
                myLAN->write((int8_t)myLanSlot);
                myLAN->write(64);
                myLAN->write('P');
                myLAN->write((int8_t)word);
                myLAN->write(hd32);
                myLAN->write(md32);
                myLAN->write(ld32);
                myLAN->write(hd);
                myLAN->write(md);
                myLAN->write(ld);
                myLAN->write(98);

                myLAN->flush();

                checkLANResponse();
            }
            break;
    }
}


boolean Hmi2::sendBasicCommand(char command){

    boolean okData = false;

    switch(connectionType){
        case SOFT_SERIAL:
            #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
                mySoft->write(64);
                mySoft->write(command);
                mySoft->write(98);

                mySoft->flush();

                okData = checkSoftResponse();
            #endif
        break;
        case HARD_SERIAL:
            myHard->write(64);
            myHard->write(command);
            myHard->write(98);

            myHard->flush();

            okData = checkHardResponse();
        break;
        case LAN:
            if(connect2Server()){
                myLAN->write((int8_t)myLanSlot);
                myLAN->write(64);
                myLAN->write(command);
                myLAN->write(98);

                myLAN->flush();

                okData = checkLANResponse();
            }
        break;
    } 

    return okData;
}

void Hmi2::fragmentData32(uint32_t tempInt32){
  ld = (tempInt32 & (BYTE6MASK0));

  tempValue32 = (tempInt32 & (BYTE6MASK1));
  md = (tempValue32 >> 6);

  tempValue32 = (tempInt32 & (BYTE6MASK2));
  hd = (tempValue32 >> 12);

  tempValue32 = (tempInt32 & (BYTE6MASK3));
  ld32 = (tempValue32 >>18);

  tempValue32 = (tempInt32 & (BYTE6MASK4));
  md32 = (tempValue32 >> 24);

  tempValue32 = (tempInt32 & (BYTE6MASK5));
  hd32 = (tempValue32 >> 30);

}

void Hmi2::fragmentData16(uint16_t tempInt16){
  ld = (tempInt16 & (LDMASK));

  tempValue = (tempInt16 & (MDMASK));
  md = (tempValue >> 6);

  tempValue = (tempInt16 & (HDMASK));
  hd = (tempValue >> 12);
}


void Hmi2::fragmentData8(uint32_t tempInt16){
  ld = (tempInt16 & (LDMASK));

  tempValue = (tempInt16 & (MDMASK));
  md = (tempValue >> 6);
}

void Hmi2::fragmentDataFloat(float preFloat){
    floatValue tempFloat;
    tempFloat.val = preFloat;

    uint32_t tempUint32 = (uint32_t)tempFloat.bytes[0];
    tempUint32 |= ((uint32_t)tempFloat.bytes[1] << 8);
    tempUint32 |= ((uint32_t)tempFloat.bytes[2] << 16);
    tempUint32 |= ((uint32_t)tempFloat.bytes[3] << 24);

    fragmentData32(tempUint32);
}

uint8_t Hmi2::joinInt8(char tempMd, char tempLd){
  uint8_t temp = 0;

  uint8_t tempLd1 = (int)tempLd;
  uint8_t tempMd1 = (int)tempMd;

  tempMd1 = (tempMd1 << 6);

  temp = (tempLd1 | tempMd1);
  temp = temp & gmask8;

  return temp;
}

uint32_t Hmi2::joinInt32(char temp6, char temp5, char temp4, char temp3, char temp2, char temp1){
  uint32_t tempA = 0;
  uint32_t tempB = 0;

  tempB = (((int32_t)temp6) << 30);
  tempA = (((int32_t)temp5) << 24);
  tempB = tempB | tempA;
  tempA = (((int32_t)temp4) << 18);
  tempB = tempB | tempA;
  tempA = (((int32_t)temp3) << 12);
  tempB = tempB | tempA;
  tempA = (((int32_t)temp2) << 6);
  tempB = tempB | tempA | ((int32_t)temp1);

  return tempB;
}

float Hmi2::joinFloat(uint32_t tempInt32){
  float temp = 0;
  floatValue tempFloat;

  tempFloat.bytes[0] = (uint8_t)(tempInt32 & BYTEMASK0);
  tempFloat.bytes[1] = (uint8_t)((tempInt32 & BYTEMASK1) >> 8);
  tempFloat.bytes[2] = (uint8_t)((tempInt32 & BYTEMASK2) >> 16);
  tempFloat.bytes[3] = (uint8_t)((tempInt32 & BYTEMASK3) >> 24);

  temp = tempFloat.val;

  return temp;
}


uint16_t Hmi2::joinInt16(char tempHd, char tempMd, char tempLd){
  uint16_t temp = 0;

  uint16_t tempLd1 = (int)tempLd;
  uint16_t tempMd1 = (int)tempMd;
  uint16_t tempHd1 = (int)tempHd;

  tempMd1 = (tempMd1 << 6);
  tempHd1 = (tempHd1 << 12);

  temp = (tempLd1 | tempMd1 | tempHd1);
  temp = temp & gmask16;

  return temp;
}

void Hmi2::setBitWord(int wordPos, int bitPos, boolean value){

    int temp = bFile[wordPos];

    if(value)
        temp |= setBitToInt(bitPos);
    else
        temp &= clearBitToInt(bitPos);

    temp &= gmask16;

    bFile[wordPos] = temp;

}

boolean Hmi2::getBitWord(int wordPos, int bitPos){
    bool temp = false;
    int tempInt = bFile[wordPos];

    tempInt >>= bitPos;

    tempInt &= 1;

    if(tempInt == 1)  
        temp = true;

    return temp;  

}

void Hmi2::setBitWordUpdate(int wordPos, int bitPos){
    int temp = bFileUpdate[wordPos];

    temp |= setBitToInt(bitPos);

    temp &= gmask16;

    bFileUpdate[wordPos] = temp;
}

boolean Hmi2::getBitWordUpdate(int wordPos, int bitPos){
    bool temp = false;
    int tempInt = bFileUpdate[wordPos];

    tempInt >>= bitPos;

    tempInt &= 1;

    if(tempInt == 1)  
        temp = true;

    return temp; 
}

void Hmi2::resetBitWordOver(int wordPos, int bitPos){
    int temp = bFileOver[wordPos];

    temp &= clearBitToInt(bitPos);

    temp &= gmask16;

    bFileOver[wordPos] = temp;  
}

boolean Hmi2::getBitWordOver(int wordPos, int bitPos){
    bool temp = false;
    int tempInt = bFileOver[wordPos];

    tempInt >>= bitPos;

    tempInt &= 1;

    if(tempInt == 1){
        temp = true;
        resetBitWordOver(wordPos, bitPos);
    }

    return temp; 
}

boolean Hmi2::getNWordOver(int wordPos){
    boolean temp = nFileOver[wordPos];

    if(temp)
        nFileOver[wordPos] = false;

    return temp;
}

boolean Hmi2::getDWordOver(int wordPos){
      boolean temp = dFileOver[wordPos];

    if(temp)
        dFileOver[wordPos] = false;

    return temp;  
}

boolean Hmi2::getFWordOver(int wordPos){
    boolean temp = fFileOver[wordPos];

    if(temp)
        fFileOver[wordPos] = false;

    return temp;
}

int Hmi2::setBitToInt(int bitPos){
    int temp = 1;

    if(bitPos > 0)
        temp <<= bitPos;
  
    return temp;
}

int Hmi2::clearBitToInt(int bitPos){
    int temp = 0xFFFE;

    if(bitPos > 0){
        for(int i=0; i<bitPos; i++){
          temp <<= 1;
          temp += 1;
        }//END for
    }//END if

    return temp;
}

boolean Hmi2::checkSoftResponse(){
    boolean okData = false;
    inCount = true;

    startTime = millis();

    while(inCount){
        #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
        if(mySoft->available()){
            inCount = false;
            if(mySoft->readBytesUntil('b', bufferSerial, sizeof(bufferSerial)) != 0)
                okData = true;
        }
        #endif

        if((millis() - startTime) > 900)
            inCount = false;
    }

    cleanSoftSerial();

    return okData;
}
        
boolean Hmi2::checkHardResponse(){
    boolean okData = false;
    inCount = true;

    startTime = millis();

    while(inCount){
        if(myHard->available()){
            inCount = false;
            if(myHard->readBytesUntil('b', bufferSerial, sizeof(bufferSerial)) != 0)
                okData = true;
        }

        if((millis() - startTime) > 900)
            inCount = false;
    }

    cleanHardSerial();

    return okData;
}


boolean Hmi2::checkLANResponse(){
    boolean okData = false;
    inCount = true;

    startTime = millis();

    while(inCount){
        if(myLAN->available()){
            inCount = false;
            if(myLAN->readBytesUntil('b', bufferSerial, sizeof(bufferSerial)) != 0){
                lanTimeCount = false;
                okData = true;
            }
        }

        if((millis() - startTime) > 900){
            inCount = false;

            if(!lanTimeCount){
                lanTimeCount = true;
                reconectTime = millis();
            }
        }
    }

    cleanLan();

    if(lanTimeCount){
        if((millis() - reconectTime) > 3000){
            myLAN->stop();
            lanConnectionStatus = false;
            lanTimeCount = false;
        }
    }

    return okData;
}

void Hmi2::cleanSoftSerial(){
    #if !defined(ESP8266) && !defined(ESP32) && !defined(__SAM3X8E__)
    if(mySoft->available()){
        while(mySoft->read() != -1){

        }
    }
    #endif
}
    
void Hmi2::cleanHardSerial(){
    if(myHard->available()){
        while(myHard->read() != -1){
        
        }
    }
}

void Hmi2::cleanLan(){

   if(myLAN->available()){
       while(myLAN->read() != -1){
       }
   }
}
