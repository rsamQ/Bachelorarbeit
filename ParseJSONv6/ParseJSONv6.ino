#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

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

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Smart-Fridge", "BusterKeel");

  display.init(115200);
  display.eraseDisplay();
  display.drawPaged(getJsonData);
}

void getJsonData(){
  // Check WiFi Status
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.0.100:150/sendJson/student/")) {  // HTTP
    
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          
          // Parsing
          const size_t capacity = 6*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 35*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 490;
          DynamicJsonDocument doc(capacity);

          // Json Payload
          String payload = http.getString();

          //Deserialize Json payload into DynamicJsonDocument
          DeserializationError error = deserializeJson(doc, payload);

          //
          drawJsonData(doc);

          //Error output on failure
          if(error){
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }
        }else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();   //Close connection
      }else {
        Serial.printf("[HTTP} Unable to connect\n");
      }     
    }
  }
}
  
//Draw Template from hex array
void drawTemplate(){
  display.fillScreen(GxEPD_BLACK);
  display.drawBitmap(0, 0, imageBitmap, 640, 384, GxEPD_WHITE); 
}

//Draw Json Data from Get Request
void drawJsonData(DynamicJsonDocument doc){
  
  JsonObject stat = doc[5];
  
  if(stat["status"]["event"] == true){
    //display.setCursor();
    //display.println();
    
  }else if(stat["status"]["closed"] == true){
    
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(200, 130);
    display.println("Im Moment geschlossen!");
    display.setCursor(200, 160);
    display.println("Grund: ");
    display.setCursor(300, 160);
    display.print(stat["status"]["reason"].as<char*>());
    Serial.println(stat["status"]["reason"].as<char*>());
    
  }else if(stat["status"]["closed"] == false && stat["status"]["event"] == false){
    
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
  }
}
          
          


void loop() {
  /*display.eraseDisplay();
  display.drawPaged(getJsonData);
  delay(1000*60);*/
 };
