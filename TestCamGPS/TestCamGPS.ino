
#include <Adafruit_GPS.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <SD.h> 


#define GPSECHO true
#define chipSelect 4


SoftwareSerial gpsConnection(3,6);
Adafruit_GPS GPS(&gpsConnection);

SoftwareSerial cameraConnection(7,8);
Adafruit_VC0706 cam(&cameraConnection); 


void init_gps()
{
  Serial.print("Initializing GPS... ");    
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_NOANTENNA);
  Serial.println("OK");
   
}


void init_camera()
{

  Serial.print("Initializing Camera... ");

  if (cam.begin()) {
    Serial.println("OK!");
  } 
  else {
    Serial.println("No camera found?");
    return;
  }

  cam.setImageSize(VC0706_320x240);

  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } 
  else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  
  pinMode(8, OUTPUT);

}


void setup()
{

  Serial.begin(9600);

  Serial.print("Initializing SD card... ");

  pinMode(chipSelect, OUTPUT);

  if (!SD.begin(chipSelect)) 
    Serial.println("Error");
  else
    Serial.println("Ok!"); 

  init_gps();

  init_camera();

  delay(2000);  

}

void snap_photo()
{   

  Serial.println("SnapPhoto");
  
  File imgfile = SD.open("file2.jpg", FILE_WRITE);

  if (!imgfile)
  {
    Serial.println("Can't open the file");
    return;
  } 
  

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image. ");
  

  int32_t time = millis();
  
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    uint8_t wbytes = imgfile.write(buffer, bytesToRead);

    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }

    jpglen -= bytesToRead;
  }  

  imgfile.close();

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); 
  Serial.println(" ms elapsed");   


}


void loop() 
{

  gpsConnection.listen();
  delay(20);
  
  // Get to obtain a NMEA line
  for(;;)
  {
    char c = GPS.read();
    if (GPSECHO)
      if (c)   Serial.print(c);

    if (GPS.newNMEAreceived())
      if (GPS.parse(GPS.lastNMEA()))
        break;
  }

  // Sentence parsed! 
  Serial.println("OK");
  
  delay(1000);
  
  cameraConnection.listen();
  delay(20);
   
  // Take picture and save in SD card
  snap_photo();     
  

  delay(10000);


}













