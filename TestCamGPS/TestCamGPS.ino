#include <SD.h>

#include <SdFat.h>
#include <SdFatUtil.h>

Sd2Card card;
SdVolume volume;
SdFile root;

#include <SoftwareSerial.h>

#include <Adafruit_VC0706.h>
#include <Adafruit_GPS.h>


// Camera
SoftwareSerial cameraConnection(7,8);
Adafruit_VC0706 cam =  Adafruit_VC0706(&cameraConnection); 


//GPS
SoftwareSerial gpsConnection(3,6);
Adafruit_GPS GPS = Adafruit_GPS(&gpsConnection);


#define GPSECHO true
boolean usingInterrupt = false;

#define chipSelect 4

char fileName[13] = "P2345678.JPG";

void init_gps()
{
  Serial.println("Initializing GPS ... ");

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

}


void init_camera()
{

  Serial.print("Initializing Camera... ");
  // Camera
  if (cam.begin()) {
    Serial.println("OK!");
  } 
  else {
    Serial.println("No camera found?");
    return;
  }

  cam.setImageSize(VC0706_320x240);

  // Print out the camera version information (optional)
  
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } 
  else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  
}

void setup()
{

  Serial.begin(9600);
 
  
  pinMode(3, INPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT);
  pinMode(8, OUTPUT);

  Serial.print("Initializing SD card... ");
  
  pinMode(chipSelect, OUTPUT);
  
  /*
  if (!SD.begin(chipSelect)) {
    Serial.println("Error");
    return;
  }
  Serial.println("Ok!");  
  */  
  
   if (!card.init(SPI_HALF_SPEED, 4)) 
     Serial.println("card.init failed!");
 
  // initialize a FAT volume
  if (!volume.init(&card)) 
    Serial.println("vol.init failed!");
  
  if (!root.openRoot(&volume)) 
    Serial.println("openRoot failed");
     
  delay(2000);  

}



void snapPhoto ()
{   

  
  if (! cam.takePicture()) 
  {
    Serial.println("Failed to snap!");
    return;
  }
  else 
    Serial.println("Picture taken!");             

  
  File imgFile = SD.open(fileName);
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
  Serial.print(time); 
  Serial.println(" ms elapsed");   


}

uint32_t timer = millis();

void loop()               
{
  
   // gps
  delay(2000);
  init_gps();
  Serial.println("");
  
  for (;;)
  {  
    char c = GPS.read();
    if (GPSECHO)
    {
      if (c) 
        Serial.print(c);
      
    }
            
    if (GPS.newNMEAreceived()) {
      char *nmea = GPS.lastNMEA();
            
      if (GPS.parse(nmea))
        break;
    }
  }       
  
  // camera    
  init_camera();      
  delay(2000);
  //snapPhoto();

  
  File imgFile = SD.open("file.txt");
  if (imgFile == false)
  {
    Serial.println("Can't open the file");
 
  } 
  else
  {
     Serial.println("File opened correctly");
     imgFile.close();
  }
    

  
}




