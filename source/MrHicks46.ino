/*
 *
 *  Copyright (C) Roberto Calvo Palomino
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 *  Author : Roberto Calvo Palomino <rocapal at gmail dot com>
 *
 */

#include <FreqCounter.h>
#include <Wire.h>
#include <OneWire.h> 
#include "RTClib.h"

#include <SD.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>  

#include <DallasTemperature.h>

// sdcard (not used)
#define chipSelect 4

// Humidity
int freq, offset, sens;

// DateTime
RTC_DS1307 RTC;

// Temperature
int DS18S20_Pin = 2; 
OneWire ds(DS18S20_Pin);

DallasTemperature sensors(&ds);

//RGB LED
#define red 3  
#define green 6 
#define blue 9 

// Camera
SoftwareSerial cameraconnection = SoftwareSerial(7,8);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


char fileName[13];
float RH;
float temp;

int i2cRead2bytes(int deviceaddress, byte address){
   // SET ADDRESS
   Wire.beginTransmission(deviceaddress);
   Wire.write(address); // address for sensitivity
   Wire.endTransmission();
  
   // REQUEST RETURN VALUE
   Wire.requestFrom(deviceaddress, 2);
   // COLLECT RETURN VALUE
   int rv = 0;
   for (int c = 0; c < 2; c++ )
   if (Wire.available()) rv = (rv << 8) | Wire.read();
   return rv;
}


void readHumidity ()
{
  FreqCounter::f_comp=8;
  FreqCounter::start(1000);
  while (FreqCounter::f_ready == 0)
    freq=FreqCounter::f_freq;
   
  //Serial.println("---------------"); 
    
  //Serial.print("offset: "); Serial.println(offset);
  //Serial.print("freq: "); Serial.println(freq);
  //Serial.print("sens: "); Serial.println(sens);
    
  RH = (offset-freq)*(sens/4096.0);
        
  //Serial.print("RH: "); Serial.println(RH);
}

void readTemp()
{

  sensors.requestTemperatures();
  //Serial.print("Temperature: ");
  //Serial.println(sensors.getTempCByIndex(0));
  temp = sensors.getTempCByIndex(0);
  
}


String readDateTime ()
{
    DateTime now = RTC.now();
    int year = now.year();
    int month = now.month();
    int day = now.day();
    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();
    
    Serial.print(year, DEC);
    Serial.print('/');
    Serial.print(month, DEC);
    Serial.print('/');
    Serial.print(day, DEC);
    Serial.print(' ');
    Serial.print(hour, DEC);
    Serial.print(':');
    Serial.print(minute, DEC);
    Serial.print(':');
    Serial.print(second, DEC);
    Serial.println();  
       
}

void setColor (int color, int value)
{
  analogWrite ( color , value );
}


void snapPhoto ()
{   
  
  cam.setImageSize(VC0706_320x240);
  
  if (! cam.takePicture()) 
  {
    Serial.println("Failed to snap!");
    return;
  }
  else 
    Serial.println("Picture taken!");
      
   
  File imgFile = SD.open(fileName, FILE_WRITE);
  if (imgFile == false)
  {
     Serial.println("No puedo abrir el fichero");
     return;
  }
    // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image. ");
  Serial.print(fileName);
  Serial.print(" ");
  
  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }

    jpglen -= bytesToRead;
  }  
    
  imgFile.close();

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); Serial.println(" ms elapsed");   
  
 
}

void getImageName ()
{
  
  // FilenameFormat:  8.3  
  // http://arduino.cc/en/Reference/SDCardNotes
  
  //This method leave in fileName variable the name of file depending of time
  // YYMMDDMM.SSI
  // The image file ends with 'I' and the data file ends with 'D'
  
  DateTime now = RTC.now();
  int year = now.year() - 2000; 
  int month = now.month();
  int day = now.day();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();
      
  sprintf(fileName,"%.2d%.2d%.2d%.2d.%.2dI", year,month,day,hour,second);
  Serial.println(fileName);
}

void saveData()
{
  fileName[11]='D';
  Serial.println(fileName);
  
  File imgFile = SD.open(fileName, FILE_WRITE);
  if (imgFile == false)
  {
     Serial.println("saveData:: Can't open the file");
     return;
  }
  
  char data[20];
   
  // Temperature
  dtostrf(temp,1,2,data);
  Serial.print("temp: "); Serial.println(data);
  imgFile.print(data);
  imgFile.println("");
  data[0]='\0';
  
  // RH
  dtostrf(RH,1,2,data);
  Serial.print("RH: "); Serial.println(data);
  imgFile.print(data);
  imgFile.println("");
  data[0]='\0';
  
  
  
  imgFile.close();
  
}

void setup()
{
  
  // Setup humidity
  Wire.begin();
  Serial.begin(9600);
  sens = i2cRead2bytes(81,10);
  offset = i2cRead2bytes(81,12);
  
  //Setup DateTime
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");    
  }
  else
  {
    //RTC.adjust(DateTime(2013, 2, 19, 15, 47, 20));
    DateTime now = RTC.now();
  }  
  
  // Setup SD FAT dateTime
  SdFile::dateTimeCallback(dateTime); 


  //Serial.println(sens);
  //Serial.println(offset);
  
  sensors.begin();
  
  // Setup RGB Led
  pinMode ( red , OUTPUT );
  pinMode ( green , OUTPUT );
  pinMode ( blue , OUTPUT );  
  
  // SdCard
  Serial.print("Initializing SD card... ");
  pinMode(4, OUTPUT);
  
  
  if (!SD.begin(4)) {
    Serial.println("Error");
    return;
  }
  Serial.println("Ok!");    
  
  // Camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  
}


void loop()
{
      
  
  SdFile::dateTimeCallback(dateTime); 
  getImageName();
     
  snapPhoto();  

  //delay(1000);
  //readDateTime();  
  
  delay(1000);
  readTemp();  
  
  delay(1000);
  readHumidity();
  
  delay(1000);
  saveData();
  
  delay(2000);
}


