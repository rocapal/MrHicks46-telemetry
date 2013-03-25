
#include <Adafruit_GPS.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <SPI.h>


#define GPSECHO false
#define chipSelect 4

#include <SdFat.h>
SdFat sd;
SdFile myFile;

SoftwareSerial gpsConnection(3,6);
//Adafruit_GPS GPS(&gpsConnection);

SoftwareSerial cameraConnection(7,8);
//Adafruit_VC0706 cam(&cameraConnection); 


void init_mygps (Adafruit_GPS &GPS)
{
  Serial.print("Initializing GPS... ");    
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_NOANTENNA);
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

  cam->setImageSize(VC0706_160x120);
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

  /*
  if (!SD.begin(chipSelect)) 
   Serial.println("Error");
   else
   Serial.println("Ok!"); 
   */

  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  else
    Serial.println("OK");


  delay(2000);  

}

void snap_photo(Adafruit_VC0706* cam)
{   


  if (! cam->takePicture()) 
  {
    Serial.println("Failed to snap!");
    return;
  }


  if (!myFile.open("test.jpg", O_RDWR | O_CREAT ))
    sd.errorHalt("opening test.jpg");


  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam->frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image. ");


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


}


void loop() 
{

  Adafruit_VC0706 *cam = new Adafruit_VC0706(&cameraConnection); 
  cameraConnection.listen();
  init_camera(cam);   

  // Take picture and save in SD card
  snap_photo(cam);     

  free(cam);

  delay(1000);



  Adafruit_GPS *GPS = new Adafruit_GPS(&gpsConnection);
  gpsConnection.listen();
  init_mygps(*GPS);


  delay(20);

  // TODO: Put a timer (60sec) to get a GPS Position
  uint32_t timer = millis();
  
  for(;;)
  {
    
    if (millis() - timer > 10000) 
    {
      Serial.println("Timeout GPS");
      break;
    }
    
    char c = GPS->read();
    if (GPSECHO)
      if (c)   Serial.print(c);

    if (GPS->newNMEAreceived())    
      if (GPS->parse(GPS->lastNMEA()))      
      {

        if ( GPS->fix ) {
          Serial.print("Location: ");
          Serial.print(GPS->latitude, 4); 
          Serial.print(GPS->lat);
          Serial.print(", "); 
          Serial.print(GPS->longitude, 4); 
          Serial.println(GPS->lon);                    
          Serial.println(GPS->speed);                    
          Serial.println(GPS->altitude);
          
          break;
        }

        
      }
  }

  free(GPS);

  delay(2000);


}


















