/*
  Fast GPS data logger.
  (c) 12th August 2016 A.G.Doswell  Web Page http://andydoz.blogspot.co.uk/2016/07/arduino-fast-gps-datalogger-updated-now.html

  License: The MIT License (See full license at the bottom of this file)

  The sketch uses a U-Blox 6M GPS satellite module connected to the hardware serial interface,
  a 128x64 SDD1306 OLED display module connected as an I2C device to pins A4 (SDA)& A5 (SCL) and an SD card interface
  connected to pins 4 (CS), 11 (MOSI), 12 (MISO) & 13 (SCK) The arduino used is a Nano with 3.3v ATMEGA328P.
  Warning - Many SD card interfaces are not 5V.
  There's a toggle switch which connects pin 9 to GND for Record.
  Data is written to the card an impressive 4 times per second, in real-time.
  A pushbutton can be used, connected between pin A3 and GND to toggle through the available speed units, Knots, Kilometres per hour, or miles per hour.

  The OzOled library is not quite "right" for this application, but works, and is lightweight and fast enough. Thanks Oscar!
  It is available here : http://blog.oscarliang.net/arduino-oled-display-library/
  TinyGPSPlus is available here :https://github.com/mikalhart/TinyGPSPlus

*/

//#include <SPI.h>
#include <Wire.h>
#include <OzOLED.h>
#include <TinyGPS++.h>
#include <EEPROM.h>

int satVal;                        // Number of satellites locked
int MPH;                         // Speed in
                 
TinyGPSPlus gps;                   // Feed gps data to TinySGPSPlus
int speedDigit;                    // Number of digits in Speedo display
int milesUnit = 0;
int milesTen = 3;
int milesHundred =8;
int milesThousand =9;
int milesTenThousand = 9;
char milesUnitA [2];
char milesTenA [2];
char  milesHundredA [2];
char milesThousandA [2];
char milesTenThousandA [2];

void setup()   {

  Serial.begin(9600);// start the comms with the GPS Rx
  delay (6000); // allow the u-blox receiver to come up
  // send serial to update u-blox rate to 200mS
  Serial.write(0xB5);
  Serial.write(0x62);
  Serial.write(0x06);
  Serial.write(0x08);
  Serial.write(0x06);
  Serial.write(0x00);
  Serial.write(0xC8);
  Serial.write(0x00);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0xDE);
  Serial.write(0x6A);
  Serial.write(0xB5);
  Serial.write(0x62);
  Serial.write(0x06);
  Serial.write(0x08);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x0E);
  Serial.write(0x30);
  delay (100);
  Serial.flush();
  // set 57,600 baud on u-blox
  Serial.write(0xB5);
  Serial.write(0x62);
  Serial.write(0x06);
  Serial.write(0x00);
  Serial.write(0x14);
  Serial.write(0x00);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0xD0);
  Serial.write(0x08);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0xE1);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x07);
  Serial.write(0x00);
  Serial.write(0x02);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0x00);
  Serial.write(0xDD);
  Serial.write(0xC3);
  Serial.write(0xB5);
  Serial.write(0x62);
  Serial.write(0x06);
  Serial.write(0x00);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0x01);
  Serial.write(0x08);
  Serial.write(0x22);
  delay (100);
  Serial.end();// stop serial coms at 9,600 baud
  delay (100);
  Serial.begin (56700); // start serial coms at 56,700 baud.

  OzOled.init();                   // initialze SDD1306 OLED display
  OzOled.sendCommand(0x8d);        // Set displays inbuilt inverter on
  OzOled.sendCommand(0x14);
  OzOled.setBrightness(0xFF);      // ... and brightness to max
  OzOled.clearDisplay();           // Clear the screen
  OzOled.setNormalDisplay();       // Set display to Normal mode
}

void loop() {
//  while (Serial.available() > 0) //While GPS message received
  //  if (gps.encode(Serial.read())) {
      updateDisplay();
      incrementMileage();
      delay (250);
     
   // }
}

void updateDisplay() {               // Display the data
  int satVal = int(gps.satellites.value());
  itoa (milesTenThousand,milesTenThousandA, 10);
  itoa (milesThousand,milesThousandA, 10);
  itoa (milesHundred,milesHundredA, 10);
  itoa (milesTen,milesTenA, 10);
  itoa (milesUnit,milesUnitA, 10);
  
  //OzOled.printNumber ((long)satVal, 0, 0);
  OzOled.printBigNumber (milesTenThousandA, 0, 0);
  OzOled.printBigNumber (milesThousandA, 3, 0);
  OzOled.printBigNumber (milesHundredA, 6, 0);
  OzOled.printBigNumber (milesTenA, 9, 0);
  OzOled.printBigNumber (milesUnitA, 12, 0);
  

  //smartDelay(200);
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

void incrementMileage () {
  milesUnit ++;
  if (milesUnit == 10) {
    milesTen++;
    milesUnit =0;
  }
  if (milesTen == 10) {
    milesHundred++;
    milesTen =0;
  }
   if (milesHundred == 10) {
    milesThousand++;
    milesHundred =0;
  }
   if (milesThousand == 10) {
    milesTenThousand++;
    milesThousand =0;
  }
   if (milesTenThousand == 10) {
    milesUnit =0;
    milesTen= 0;
    milesHundred =0;
    milesThousand =0;
    milesTenThousand =0;
  }
 
 
 
 
}


/*
   Copyright (c) 2016 Andrew Doswell

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/
