/*
Fast GPS data logger.
(c) 1st January 2016 A.G.Doswell

Released to all and sundry under GNU License.

The sketch uses a U-Blox 5 GPS satellite module connected to the hardware serial interface, 
a 128x64 SDD1306 OLED display module connected as an I2C device to pins A4 (SDA)& A5 (SCL) and an SD card interface 
connected to pins 4 (CS), 11 (MOSI), 12 (MISO) & 13 (SCK) The arduino used is a Nano with 3.3v ATMEGA328P.
Warning - Many SD card interfaces are not 5V. 
There's a toggle switch which connects pin 9 to GND for Record.
Data is written to the card an impressive 4 times per second, in real-time. 

Brake light is connected to pin 9 via suitable circuitry. (See andydoz.blogspot.com for details)

The OzOled library is not quite "right" for this application, but works, and is lightweight and fast enough. Thanks Oscar!
It is available here : http://blog.oscarliang.net/arduino-oled-display-library/
TinyGPSPlus is available here :https://github.com/mikalhart/TinyGPSPlus

*/

#include <SPI.h>
#include <Wire.h>
#include <OzOLED.h>
#include <TinyGPS++.h>
#include <SD.h>

static const uint32_t GPSBaud = 57600;
int SatVal;                        // Number of satellites locked
int Speed;                         // Speed in MPH
char SpeedA [4];                   // Speed as char
TinyGPSPlus gps;                   // Feed gps data to TinySGPSPlus
File myFile;                       // Start SD
int speedDigit;                    // Number of digits in Speedo display 
boolean card;                      // Is there a card present?
boolean rec = false;               // Is the record switch low? (false = high)
int recPin = 9;                    // Record pin (Pull low to record data) 
int brakePin = 8;                  // Input pin connected to brake light circuit.

void setup()   {                

  Serial.begin(GPSBaud);           // Start GPS serial comms
  pinMode (recPin, INPUT_PULLUP);  // recPin as input and pulled high
  pinMode (brakePin, INPUT_PULLUP);  // recPin as input and pulled high
  OzOled.init();                   // initialze SDD1306 OLED display
  OzOled.sendCommand(0x8d);        // Set displays inbuilt inverter on
  OzOled.sendCommand(0x14);          
  OzOled.setBrightness(0xFF);      // ... and brightness to max
  OzOled.clearDisplay();           // Clear the screen
  OzOled.setNormalDisplay();       // Set display to Normal mode
  pinMode(10, OUTPUT);             // CS for SD card, wether it likes it, or not.
  OzOled.clearDisplay();
  
  
  if (!SD.begin(4)) {              //Check SD card is present
      OzOled.printString("SD card fail    ",0,7);
      card = false;  
      }
      else {
      OzOled.printString("SD card OK      ",0,7);
      card = true;
      }
  
  OzOled.printString("Sats:",0,0);  // Set up display
  OzOled.printString("Speed:",0,1);
  OzOled.printString("MPH",7,6);
}

void loop() {
    while (Serial.available() > 0) //While GPS message received
    if (gps.encode(Serial.read()))
      displayInfo();
}

void displayInfo() {               // Display the data
  SatVal = int(gps.satellites.value());
  Speed = int(gps.speed.mph());
  itoa (Speed,SpeedA,10);
  OzOled.printNumber ((long)SatVal,6,0);
  OzOled.printBigNumber (SpeedA, 7,1);
  speedDigit = strlen(SpeedA);    // get length of Speed , and delete left behind zero if req'd
  if (speedDigit == 1) OzOled.printBigNumber(" ",10,1);
  if (digitalRead(recPin) == HIGH && card == true) { //If the record switch is high and the card is OK, write card data
    rec = true;
    OzOled.printString("SD card OK REC",0,7);
  }
  if (digitalRead(recPin) == LOW && card == true) {
    rec = false;
    OzOled.printString("SD card OK    ",0,7);
  }
  
  if (card==true && rec==true) {
    writeInfo(); //write the data to the SD card
  }
  smartDelay(200);

}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}

void writeInfo() { //Write the data to the SD card Date,Time,Lat,Long,Spped (MPH) and number of satellites locked
  
  myFile = SD.open("gps.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(gps.date.month());
    myFile.print(F("/"));
    myFile.print(gps.date.day());
    myFile.print(F("/"));
    myFile.print(gps.date.year());
    myFile.print(F(","));
    if (gps.time.hour() < 10) myFile.print(F("0"));
    myFile.print(gps.time.hour());
    myFile.print(F(":"));
    if (gps.time.minute() < 10) myFile.print(F("0"));
    myFile.print(gps.time.minute());
    myFile.print(F(":"));
    if (gps.time.second() < 10) myFile.print(F("0"));
    myFile.print(gps.time.second());
    myFile.print(F(","));
    myFile.print(gps.location.lat(), 6);
    myFile.print(F(","));
    myFile.print(gps.location.lng(), 6);
    myFile.print(F(","));
    myFile.print(gps.speed.mph(),1);
    myFile.print(F(","));
    myFile.println(gps.satellites.value());
	// close the file:
    myFile.close();
  }
}

