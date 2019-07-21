#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "DHT.h"
#include <SparkFunTSL2561.h>
#include <Wire.h>

#define DHTPIN 15
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
SFE_TSL2561 light;

String dataMessage;
float h, c, f;
boolean gain, good;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms, data0, data1;  
unsigned char ID;
double lux;

File sensorData;

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void logSDCard() {
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/DataLogCS.csv", dataMessage.c_str());
}

void printError(byte error) { //Error print from gy2561
  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error)
  {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  light.begin();
  
  if(!SD.begin()) { //SD indicator
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  if (light.getID(ID)) { //GY2561 indicator
    Serial.print("Got factory ID: 0X");
    Serial.print(ID,HEX);
    Serial.println(", should be 0X5X");
  }
  else {
    byte error = light.getError();
    printError(error);
  }

  gain = 0;
  unsigned char time = 2;

  light.setTiming(gain,time,ms);
  light.setPowerUp();
  writeFile(SD, "/DataLogCS.csv", "Humidity,Temperature(C),Light0, Light1, Lumunosity\n");
}

void loop() {
  light.getData(data0,data1);
  light.getLux(gain,ms,data0,data1,lux);

  h = dht.readHumidity(); //read humidity
  c = dht.readTemperature(); //read Celcius Temp
  f = dht.readTemperature(true); //read Fahrenheit temp
  
  dataMessage = String(h) + "," + String(c) + "," + String(data0) + "," + String(data1) + "," + String(lux) + "\r\n";

  logSDCard();
  delay(500);
}
