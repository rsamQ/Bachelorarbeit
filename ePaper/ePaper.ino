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
  display.drawPaged(getJsonData);
}


void getJsonData(){
  // Check WiFi Status
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print(F("[HTTP] begin...\n"));
    if (http.begin(client, jsonUrl)) {  // HTTP
    
      Serial.print(F("[HTTP] GET...\n"));
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

          writeJsonToSpiffs(payload);
          loadJsonFromSpiffs();

        }else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          display.setCursor(50, 300);
          display.print(F("Error Message: HTTP GET failed"));
          display.update();
        }
        http.end();   //Close connection
      }else {
        Serial.printf("[HTTP} Unable to connect\n");
        display.setCursor(50, 300);
        display.print(F("Error Message: HTTP unable to connect"));
        display.update();
      }     
    }
  }
}


void loadJsonFromSpiffs(){
  Serial.println(F("Loading file"));

  File configFile = SPIFFS.open("test.json", "r");

  if(configFile){
    
    DynamicJsonDocument doc(MAXIMUM_CAPACITY);
    DeserializationError error = deserializeJson(doc, configFile);

    //Error output on failure
    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    drawJsonData(doc);
  }else{
    Serial.println(F("Failed to open config file"));
  }
  configFile.close();
}



void writeJsonToSpiffs(String payload){
  Serial.println(F("Saving file"));

  File configFile = SPIFFS.open("test.json", "w");

  if(configFile){
    Serial.println(F("File opened"));

    DynamicJsonDocument doc(MAXIMUM_CAPACITY);
    DeserializationError error = deserializeJson(doc, payload);

    //Error output on failure
    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    Serial.println(F("Serializeing to file. Size = "));
    uint16_t size = serializeJson(doc, configFile);
    Serial.println(size);
  }else{
    Serial.println(F("Failed to open file!"));
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
      JsonObject repo0 = doc[i-1].as<JsonObject>();
      
      for(int j = 1; j < 7; j++){
        //JsonObject obj = repo0["data"][j-1];  
        display.setCursor((110*i), (j*50));
        display.println(repo0["data"][j-1]["subject"].as<char*>());    
        Serial.println(repo0["data"][j-1]["subject"].as<char*>());
        Serial.println(repo0["data"][j-1]["professor"].as<char*>());
      }
    }
  }
}


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
