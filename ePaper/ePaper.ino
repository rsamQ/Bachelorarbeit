#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <FS.h>

ESP8266WiFiMulti WiFiMulti;

#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>  

//specific library for the e-Paper 7.5" colors black/white/red
#include <GxGDEW075Z09/GxGDEW075Z09.h>

// library for fonts
#include <Fonts/FreeMonoBold12pt7b.h>

// include templates
#include "template.h"
#include "new_template.h"

//definition of SPI pins
#define CS_PIN           15 // D8
#define RST_PIN          5  // D1
#define DC_PIN           4  // D2
#define BUSY_PIN         16 // D0

GxIO_Class io(SPI, CS_PIN, DC_PIN, RST_PIN);
GxEPD_Class display(io, RST_PIN, BUSY_PIN);

// WiFi Parameters
const char* ssid = "Smart-Fridge";
const char* password = "BusterKeel";
const char* imageUrl = "http://192.168.0.100:150/serverData/image/";
const char* jsonUrl = "http://192.168.0.100:150/serverData/schedule/";
const size_t MAXIMUM_CAPACITY = 3072;

void setup() {
  
  //
  Serial.begin(115200);
  display.init(115200);

  //
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  //
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Smart-Fridge", "BusterKeel");

  //
  if (SPIFFS.begin()) {
    Serial.println(F("\nSPIFFS mounted"));
  } else {
    Serial.println(F("\nFailed to mount SPIFFS"));
  }

  Serial.println(F("Formating SPIFFS "));
  if (SPIFFS.format()) {
    Serial.println(F("done"));
  } else {
    Serial.println(F("ERROR"));
  }

  delay(1000);
  
  display.eraseDisplay();
  display.drawPaged(loadJsonFromSpiffs);
}



String getJsonData(){
  // Check WiFi Status
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, jsonUrl)) {  // HTTP
    
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

          // Json Payload
          String payload = http.getString();
          return payload;

        }else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          display.setCursor(50, 300);
          display.print("Error Message: HTTP GET failed");
          display.update();
        }
        http.end();   //Close connection
      }else {
        Serial.printf("[HTTP} Unable to connect\n");
        display.setCursor(50, 300);
        display.print("Error Message: HTTP unable to connect");
        display.update();
      }     
    }
  }
}



void loadJsonFromSpiffs(){
  Serial.println(F("Loading file"));

  if(!SPIFFS.exists("test.json")){
    writeJsonToSpiffs();
  }
  
  File configFile = SPIFFS.open("test.json", "r");

  DynamicJsonDocument doc(MAXIMUM_CAPACITY);
  DeserializationError error = deserializeJson(doc, configFile);

  //Error output on failure
  if(error){
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  drawJsonData(doc);

  Serial.print(F("serializeJson = "));
  serializeJson(doc, Serial);
  configFile.close();
}



void writeJsonToSpiffs(){
  Serial.println(F("Saving file"));

  File configFile = SPIFFS.open("test.json", "w");

  if(configFile){
    Serial.println(F("File opened"));

    DynamicJsonDocument doc(MAXIMUM_CAPACITY);
    DeserializationError error = deserializeJson(doc, getJsonData());

    //Error output on failure
    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    Serial.println(F("Serializeing to file. Size = "));
    uint16_t size = serializeJson(doc, configFile);
    Serial.println(size);
  }
  configFile.close();
}



//Draw Json Data from Get Request
void drawJsonData(DynamicJsonDocument doc){

  JsonObject infoMode = doc[5];

  if(infoMode["mode"] == 0){
    drawTemplate();
    display.setTextColor(GxEPD_BLACK);
    
    for(int i = 1; i < 6; i++){
      JsonObject repo0 = doc[i-1];
      
      for(int j = 1; j < 7; j++){
        JsonObject obj = repo0["data"][j-1];        
        Serial.println(obj["subject"].as<char*>());
        Serial.println(obj["professor"].as<char*>());
      }
    }
  }
}
  /*if(infoMode[5] == 2){
    //display.setCursor();
    //display.println();
    
  }else if(infoMode[5] == 1){
    
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(200, 130);
    display.println("Im Moment geschlossen!");
    display.setCursor(200, 160);
    display.println("Grund: ");
    display.setCursor(300, 160);
    display.print(infoMode["info"].as<char*>());
    Serial.println(infoMode["info"].as<char*>());
    
  }else if(infoMode[5] == 0){
    
    drawTemplate();
    display.setTextColor(GxEPD_BLACK);
    
    for(int i = 1; i < 6; i++){
      JsonObject repo0 = doc[i-1];
      
      for(int j = 1; j < 7; j++){
        JsonObject obj = repo0["data"][j-1];        
        display.setCursor(110 * i, (j * 50));
        
        if(j > 3){
          display.setCursor(110 * i, (j * 50) + 50);
        }
        display.println(obj["subject"].as<char*>());

        
        display.setCursor(110 * i, 10 + (j * 50));
        if(j > 3){
          display.setCursor(110 * i, (j * 50) + 60);
        }
        display.println(obj["professor"].as<char*>());
        
        //Serial.println(obj["subject"].as<char*>());
        //Serial.println(obj["professor"].as<char*>());
      }           
    }
  }*/


//Draw Template from hex array
void drawTemplate(){
  display.fillScreen(GxEPD_BLACK);
  display.drawBitmap(0, 0, imageBitmap, 640, 384, GxEPD_WHITE); 
}
          
          


void loop() {
  /*display.eraseDisplay();
  display.drawPaged(getJsonData);
  delay(1000*60);*/
 };
