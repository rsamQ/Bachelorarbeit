// Arduino library headers
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// ArduinoJson library headers
#include <ArduinoJson.h>
#include <Arduino.h>

// Spiffs library header
#include <FS.h>

// Adafruit library headers
#include <Adafruit_GFX.h>
#include <gfxfont.h>

// library for fonts
#include <Fonts/FreeMonoBold12pt7b.h>

// include templates
#include "new_template.h"

// 
#include <GxEPD2.h>
#include <GxEPD2_3C.h>

//
GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT / 4> display(GxEPD2_750c(/*CS=15*/ 15, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));


// WiFi Parameters
ESP8266WiFiMulti WiFiMulti;
const char* ssid = "Smart-Fridge";
const char* password = "BusterKeel";
const char* imageUrl = "http://192.168.0.100:150/serverData/image/";
const char* jsonUrl = "http://192.168.0.100:150/serverData/schedule/";
const size_t MAXIMUM_CAPACITY = 4096;

String saveData = "";

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
  getJsonData();
  writeJsonToSpiffs();  
}


void drawData(const void*){
  drawJsonData();
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
          saveData = http.getString();

        }else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          display.setCursor(50, 300);
          display.print(F("Error Message: HTTP GET failed"));
          display.refresh();
        }
        http.end();   //Close connection
      }else {
        Serial.printf("[HTTP} Unable to connect\n");
        display.setCursor(50, 300);
        display.print(F("Error Message: HTTP unable to connect"));
        display.refresh();
      }     
    }
  }
}


DynamicJsonDocument loadJsonFromSpiffs(){
  Serial.println(F("Loading file"));

  File configFile = SPIFFS.open("test.json", "r");

  if(configFile){
    
    DynamicJsonDocument doc(MAXIMUM_CAPACITY);
    DeserializationError error = deserializeJson(doc, configFile);

    //Error output on failure
    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    return doc;
  }else{
    Serial.println(F("Failed to open config file"));
    /*writeJsonToSpiffs();
    loadJsonFromSpiffs();*/
  }
  configFile.close();
}


void writeJsonToSpiffs(){
  Serial.println(F("Saving file"));

  File configFile = SPIFFS.open("test.json", "w");
  
  if(configFile){
    Serial.println(F("File opened"));
    Serial.println(F("Write file"));
    Serial.println(saveData);
    configFile.print(saveData);
  }else{
    Serial.println(F("Failed to open file!"));
  }
  Serial.println(F("Closing file"));
  configFile.close();
}


//Draw Json Data from Get Request
void drawJsonData(){

  int16_t days = 6;
  int16_t offsetLeft = 94;
  int16_t offsetTop = 46;
  int16_t duration = 15;
  int16_t heightPerDuration = 7;
  int16_t startingHour = 8;

  File configFile = SPIFFS.open("test.json", "r");

  DynamicJsonDocument doc1(MAXIMUM_CAPACITY);
  deserializeJson(doc1, configFile);

  DynamicJsonDocument doc2(MAXIMUM_CAPACITY);
  deserializeJson(doc2, saveData);

  JsonObject infoMode = doc2[6];

  if(infoMode["mode"] == 0){
    drawTemplate();
    
    for(int i = 1; i <= 6; i++){
      JsonObject repo0 = doc1[i-1].as<JsonObject>();
      JsonObject repo1 = doc2[i-1].as<JsonObject>();
      
      for(int j = 1; j < 7; j++){

        int16_t hour = repo1["data"][j-1]["hour"].as<int>();
        int16_t minutes = repo1["data"][j-1]["minutes"].as<int>();
        int16_t x = offsetLeft + (i-1) * ((display.width() - offsetLeft) / days);
        int16_t y = offsetTop + ((hour - startingHour) * (4 * heightPerDuration) + (minutes / duration) * heightPerDuration);
        int16_t w = ((display.width() - offsetLeft) / days) - 8;
        int16_t h = ((repo1["data"][j-1]["duration"].as<int>() / duration) * heightPerDuration);
        
        if(repo1["data"][j-1]["subject"] != repo0["data"][j-1]["subject"] || repo1["data"][j-1]["hour"] != repo0["data"][j-1]["hour"] ||
        repo1["data"][j-1]["professor"] != repo0["data"][j-1]["professor"] || repo1["data"][j-1]["minute"] != repo0["data"][j-1]["minute"]){
          
          display.drawRect(x, y, w, h, GxEPD_RED);
          display.setTextColor(GxEPD_RED);
          display.setCursor(x + 5, y + 5);
          display.println(repo1["data"][j-1]["subject"].as<char*>());
          display.setCursor(x + 5, y + 15);
          display.println(repo1["data"][j-1]["professor"].as<char*>());
          
        }else{
          
          display.drawRect(x, y, w, h, GxEPD_BLACK);
          display.setTextColor(GxEPD_BLACK);
          display.setCursor(x + 5, y + 5);
          display.println(repo1["data"][j-1]["subject"].as<char*>());
          display.setCursor(x + 5, y + 15);
          display.println(repo1["data"][j-1]["professor"].as<char*>());    
        }
      }
    }
  }
  configFile.close();
}


//Draw Template from hex array
void drawTemplate(){
  display.fillScreen(GxEPD_BLACK);
  display.drawBitmap(0, 0, imageBitmap, 640, 384, GxEPD_WHITE); 
}      


void loop() {
  getJsonData();
  display.refresh();
  display.drawPaged(drawData, 0);
  writeJsonToSpiffs();
  delay(1000*20);
 };
