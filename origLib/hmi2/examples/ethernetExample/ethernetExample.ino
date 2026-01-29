#include <Arduino.h>
#include <hmi2.h>                                       //HMI 2 Library


Hmi2 hmi2;                                              //HMI 2 object


// Enter a MAC address and IP address for your controller below.
// The IP address, gateway and subnet will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,70);                             //Arduino board IP
IPAddress gateway(192,168,1,254);                       //Arduino board gateway
IPAddress subnet(255,255,255,0);                        //Arduino subnet

IPAddress server(192,168,1,66);                         //Server IP              



boolean ledState = false;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


  Ethernet.begin(mac,ip,gateway,subnet);                //Initializes the ethernet library and network settings.

  hmi2.init(server, 1);                                 //Initializes HMI 2 object with the ethernet connection. LanMemoryBank 1 is selected)

  pinMode(13, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  
  ledState = hmi2.getBoolean(1,2);                      //Get boolean value from word 1, bit 2. (B File)
  
  hmi2.setInt(4, analogRead(A0));                       //Set uint16_t value to word 4 with the analog value of pin A0. (N File)
  hmi2.setDInt(4, analogRead(A1));                      //Set uint32_t value to word 4 with the analog value of pin A1. (D File)
  hmi2.setFloat(6, analogRead(A2));                     //Set float value to word 6 with the analog value of pin A2. (F File)
  
  hmi2.setBoolean(2,10, digitalRead(5));                //Set boolean value to word 2, bit 10 with the digital value of pin 5. (B File)
  hmi2.setBoolean(0,0, digitalRead(6));                 //Set boolean value to word 0, bit 0 with the digital value of pin 6. (B File)
  analogWrite(3, hmi2.getDInt(12));                     //Write in analog pin 3 (PWM) with the value of the word 12 from the D File.
  

  if(ledState)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);


  if(hmi2.getBoolean(3, 0))                             //Get boolean value from word 3, bit 0 (B File)
    digitalWrite(2, HIGH);
  else
    digitalWrite(2, LOW);

 
  hmi2.setDisplayID(1);                                 //Set display 1 as current display.
  hmi2.setCursor(0,0);                                  //Set cursor position of display 1 to position x = 0, y = 0.
  hmi2.print("Hello World!!!");                         //Print text to display 1.

  hmi2.setCursor(0,1);                                  //Set crusor position of display 1 to position x = 0, y = 1.
  hmi2.print("-> ");                                    //Print text to display 1.
  hmi2.print(analogRead(A0));                           //Print value of analog pin A0 to display 1.
  

  hmi2.update();                                        //Manages the connection between the Arduino board and the app.

}
