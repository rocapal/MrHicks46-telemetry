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


#include <Adafruit_GPS.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <DallasTemperature.h>
#include <OneWire.h>

// Temperature
int DS18S20_Pin = 2; 
OneWire ds(DS18S20_Pin);
DallasTemperature sensors(&ds);



#define GPSECHO false
#define chipSelect 4

SdFat sd;
SdFile myFile;

float temp;
char data_file[13];

void init_mygps (Adafruit_GPS* GPS)
{
  Serial.print("Initializing GPS... ");    
  GPS->begin(9600);
  GPS->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS->sendCommand(PGCMD_ANTENNA);
  Serial.println("OK");

}


void init_camera(Adafruit_VC0706* cam)
{

  Serial.print("Initializing Camera... ");

  if (cam->begin()) {
    Serial.println("OK!");
  } 
  else {
    Serial.println("No camera found?");
    return;
  }


  
  char *reply = cam->getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } 
  else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  

  cam->setImageSize(VC0706_640x480);
  uint8_t imgsize = cam->getImageSize();
  /*
  Serial.print("Image size: ");
   if (imgsize == VC0706_640x480) Serial.println("640x480");
   if (imgsize == VC0706_320x240) Serial.println("320x240");
   if (imgsize == VC0706_160x120) Serial.println("160x120");
   */

}



void setup()
{

  Serial.begin(9600);

  Serial.print("Initializing SD card... ");

  pinMode(chipSelect, OUTPUT);

  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  else
    Serial.println("OK"); 


  delay(2000);  

}

boolean snap_photo(Adafruit_VC0706* cam)
{   

  if (! cam->takePicture()) 
  {
    Serial.println("Failed to snap!");
    return false;
  }
  
  data_file[11]='I';
    
  if (!myFile.open(data_file, O_RDWR | O_CREAT ))
  {
    //sd.errorHalt("opening file image");
    return false;
  }

  delay(20);
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam->frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image .");

  if (jpglen == 0)
    return false;

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam->readPicture(bytesToRead);
    myFile.write(buffer, bytesToRead);

    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }

    jpglen -= bytesToRead;
  }  

  myFile.close();

  time = millis() - time;
  Serial.println("done!");   

  return true;

}

void save_data(Adafruit_GPS *GPS)
{   

  // Build fileName
  // 48 is the ASCII code for 0

  data_file[0] = 48 + GPS->year/10;
  data_file[1] = 48 + GPS->year%10;
  data_file[2] = 48 + GPS->month/10;
  data_file[3] = 48 + GPS->month%10;
  data_file[4] = 48 + GPS->day/10;
  data_file[5] = 48 + GPS->day%10;
  data_file[6] = 48 + GPS->hour/10;
  data_file[7] = 48 + GPS->hour%10;
  data_file[8] ='.';
  data_file[9] = 48 + GPS->minute/10;
  data_file[10]= 48 + GPS->minute%10;
  data_file[11]='D';
  data_file[12]='\0';  

  if (!myFile.open(data_file, O_RDWR | O_CREAT ))
    sd.errorHalt("opening data file");  
  else
  {

    myFile.println(temp);
    myFile.print(GPS->latitude, 4); 
    myFile.println(GPS->lat);
    myFile.print(GPS->longitude, 4); 
    myFile.println(GPS->lon);
    myFile.println(GPS->altitude);
    myFile.println(GPS->speed);
    myFile.close();
  }  

}

void get_temperature()
{
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);  
  //Serial.print("Temp: "); 
  //Serial.println(temp);
}

boolean camera = false;
boolean gps = false;

void loop() 
{
  // Only take a picture if we have GPS location

  SoftwareSerial* cameraConnection = new SoftwareSerial(7,8);
  Adafruit_VC0706 *cam = new Adafruit_VC0706(cameraConnection); 
  cameraConnection->listen();
  delay(20);
  init_camera(cam);   

  if (gps) { 
    
    // We have this loop becaouse sometimes the
    // snap_photo fails and we try again
    uint32_t timer_camera = millis();
    for (;;)
    {
      if (millis() - timer_camera > 10000)
        break;
       
      // Take picture and save in SD card 
      boolean res = snap_photo(cam);
      if (res)
      {
        camera = true;
        break;
      }
    }
  }

  free(cameraConnection);
  free(cam);
  
  delay(1000);

  if (!gps)
  {
    SoftwareSerial* gpsConnection = new SoftwareSerial(3,6);
    Adafruit_GPS *GPS = new Adafruit_GPS(gpsConnection);
    gpsConnection->listen();
    init_mygps(GPS);
  
    uint32_t timer = millis();
  
    for(;;)
    {
  
      if (millis() - timer > 10000) 
      {
        Serial.println("\nTimeout GPS");
        break;
      }
  
      char c = GPS->read();
      if (GPSECHO)
        if (c)   Serial.print(c);
  
      if (GPS->newNMEAreceived())    
        if (GPS->parse(GPS->lastNMEA()))      
        {
          Serial.print(".");
  
          if ( GPS->fix ) {
  
            Serial.print("Location: ");
            Serial.print(GPS->latitude, 4); 
            Serial.print(GPS->lat);
            Serial.print(", "); 
            Serial.print(GPS->longitude, 4);             
            Serial.println(GPS->lon);                    
            //Serial.println(GPS->speed);                    
            //Serial.println(GPS->altitude);
  
            get_temperature();
  
            save_data(GPS);          
  
            gps = true;
  
            break;
          }
        }
    }
  
    free(gpsConnection);
    free(GPS);   
  }
  
  

  if (gps && camera)
  {
    gps = false;
    camera = false;        
    
    uint32_t timer2 = millis();
    for(;;)
    {      
      if (millis() - timer2 > 120000)
      {
        return;
      }
    }
    
  }



}
