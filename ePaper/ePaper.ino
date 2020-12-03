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
#include <Fonts/FreeMonoBold9pt7b.h>

// include templates
#include "new_template.h"

// 
#include <GxEPD2.h>
#include <GxEPD2_3C.h>

// define spi pins
GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT / 4> display(GxEPD2_750c(/*CS=15*/ 15, /*DC=4*/ 4, /*RST=5*/ 5, /*BUSY=16*/ 16));


// WiFi Parameters
ESP8266WiFiMulti WiFiMulti;
const char* ssid = "Smart-Fridge";
const char* password = "BusterKeel";
const char* imageUrl = "http://192.168.0.100:150/serverData/image/image.bmp";
const char* jsonUrl = "http://192.168.0.100:150/serverData/schedule/";

//
const char * headerKeys[] = {"date", "server"} ;
const size_t numberOfHeaders = 2;

//
String tempJson = "";
String updateTime = "";

//
bool receiveImage = false;

//
bool connectionError = false;
String errorMsg = "";

//
const size_t MAXIMUM_CAPACITY = 4096;




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

  //
  if (SPIFFS.format()) {
    Serial.println(F("done"));
  } else {
    Serial.println(F("ERROR"));
  }

  delay(1000); 
}




void drawData(const void*){
  compareAndDrawJsonData();
}




void drawTempData(const void*){
  drawJsonDataFromMemory();
}




void getJsonDataFrom_HTTP(const char* url){
  
  // Check WiFi Status
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print(F("[HTTP] begin...\n"));
    if (http.begin(client, url)) {  // HTTP

      //
      http.collectHeaders(headerKeys, numberOfHeaders);
    
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
          tempJson = http.getString();
          
          // Response Header time
          updateTime = http.header("date");

          //
          connectionError = false;

        }else {
          
          // 
          updateTime = http.header("date");
          connectionError = true;
          errorMsg = (F("Error: "));
          errorMsg += http.errorToString(httpCode).c_str();
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        
        http.end();   //Close connection
        
      }else {
        
        //
        connectionError = true;
        errorMsg = (F("Error: HTTP unable to connect"));
        Serial.printf("[HTTP] Unable to connect\n");
      }     
    }
  }else{
    connectionError = true;
    errorMsg = (F("Error: Unable to connect to WiFi"));
    Serial.printf("Unable to connect to WiFi\n");
  }
}

/*void checkForImage(const char* url){
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)

    Serial.print("[HTTP] begin...\n");

    // configure server and url
    http.begin(client, url);
    //http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {

        if(receiveImage == true){

          SPIFFS.remove("image.bmp");
  
          // get lenght of document (is -1 when Server sends no Content-Length header)
          int len = http.getSize();
  
          // create buffer for read
          uint8_t buff[128] = { 0 };
  
          // get tcp stream
          WiFiClient * stream = &client;
  
          fs::File file;
          file = SPIFFS.open("image.bmp", "w");
  
          // read all data from server
          while (http.connected() && (len > 0 || len == -1)) {
            // read up to 128 byte
            int c = stream->readBytes(buff, std::min((size_t)len, sizeof(buff)));
            Serial.printf("readBytes: %d\n", c);
            if (!c) {
              Serial.println("read timeout");
            }
  
            file.write(buff, c);
  
            // write it to Serial
            Serial.write(buff, c);
  
            if (len > 0) {
              len -= c;
            }
          }
          file.close();
  
          Serial.println();
          Serial.print("[HTTP] connection closed or file end.\n");
        }
        receiveImage = false;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}*/




void writeJsonTo_SPIFFS(){
  
  Serial.println(F("Saving file"));

  File storedData = SPIFFS.open("test.json", "w"); //
  
  if(storedData){
    
    Serial.println(F("File opened"));
    Serial.println(F("Write file"));
    Serial.println(tempJson);
    storedData.print(tempJson); //
  }else{
    Serial.println(F("Failed to open file!"));
  }
  
  Serial.println(F("Closing file"));
  
  storedData.close(); //
}




//Draw Json Data from Get Request
void drawJsonDataFromMemory(){

  //
  int16_t days = 6;
  int16_t offsetLeft = 94;
  int16_t offsetTop = 46;
  int16_t duration = 15;
  int16_t heightPerDuration = 7;
  int16_t startingHour = 8;

  File storedData = SPIFFS.open("test.json", "r");  //

  //
  DynamicJsonDocument storedDoc(MAXIMUM_CAPACITY);
  deserializeJson(storedDoc, storedData);

  //
  JsonObject infoMode = storedDoc[6];
  JsonObject room = storedDoc[7];
  JsonObject closedRoom = storedDoc[8];

  if(infoMode["mode"] == 0){
    drawTemplate();

    //
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(15, 25);
    display.println(room["room"].as<char*>());  //
    display.setCursor(15, 374);
    display.setFont();
    display.println("Last Update: " + updateTime);  // 

    for(int i = 1; i <= days; i++){
      JsonObject repo0 = storedDoc[i-1].as<JsonObject>();  //
    
      for(int j = 1; j <= repo0["data"].size(); j++){
        
        int16_t hour = repo0["data"][j-1]["hour"].as<int>();  //
        int16_t minutes = repo0["data"][j-1]["minutes"].as<int>(); //
        int16_t x = offsetLeft + (i-1) * ((display.width() - offsetLeft) / days); //
        int16_t y = offsetTop + ((hour - startingHour) * (4 * heightPerDuration) + (minutes / duration) * heightPerDuration); //
        int16_t w = ((display.width() - offsetLeft) / days) - 8;  //
        int16_t h = ((repo0["data"][j-1]["duration"].as<int>() / duration) * heightPerDuration);  //
          
        display.drawRect(x, y, w, h, GxEPD_BLACK); //
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(x + 5, y + 5);  //
        display.println(repo0["data"][j-1]["subject"].as<char*>());
        display.setCursor(x + 5, y + 15); //
        display.println(repo0["data"][j-1]["professor"].as<char*>());
        
        if(connectionError == true){
          display.setTextColor(GxEPD_RED);
          display.setCursor(335, 374);
          display.println(errorMsg);  // 
        }
      }
    }
  }else if(infoMode["mode"] == 1){

    /*receiveImage = true;
    drawBitmapFromSpiffs("image.bmp", 0, 0, false);
    receiveImage = false;*/
    
  }else if(infoMode["mode"] == 2){

    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(display.width()/4, display.height()/4);
    display.println(F("Dieser Raum ist momentan geschlossen!"));
    display.setCursor((display.width()/4), (display.height()/4)+ 20);
    display.println("Grund: ");
    display.println(closedRoom["info"].as<char*>());
    
  }
  storedData.close();
}




//Draw Json Data from Get Request
void compareAndDrawJsonData(){

  //
  int16_t days = 6;
  int16_t offsetLeft = 94;
  int16_t offsetTop = 46;
  int16_t duration = 15;
  int16_t heightPerDuration = 7;
  int16_t startingHour = 8;

  File storedData = SPIFFS.open("test.json", "r");  //

  if(!storedData){
    storedData.close();
    writeJsonTo_SPIFFS();
    storedData = SPIFFS.open("test.json", "r");  //
  }

  //
  DynamicJsonDocument storedDoc(MAXIMUM_CAPACITY);
  deserializeJson(storedDoc, storedData);

  //
  DynamicJsonDocument receivedDoc(MAXIMUM_CAPACITY);
  deserializeJson(receivedDoc, tempJson);

  //
  JsonObject infoMode = receivedDoc[6];
  JsonObject room = receivedDoc[7];
  JsonObject closedRoom = receivedDoc[8];

  Serial.println(infoMode["mode"].as<int>());

  if(infoMode["mode"] == 0){
    drawTemplate();

    //
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(15, 25);
    display.println(room["room"].as<char*>());  //
    display.setCursor(15, 374);
    display.setFont();
    display.println("Last Update: " + updateTime);  //
    
    
    for(int i = 1; i <= days; i++){
      JsonObject repo0 = storedDoc[i-1].as<JsonObject>();  //
      JsonObject repo1 = receivedDoc[i-1].as<JsonObject>();  //
    
      for(int j = 1; j <= repo1["data"].size(); j++){

        int16_t hour = repo1["data"][j-1]["hour"].as<int>();  //
        int16_t minutes = repo1["data"][j-1]["minutes"].as<int>(); //
        int16_t x = offsetLeft + (i-1) * ((display.width() - offsetLeft) / days); //
        int16_t y = offsetTop + ((hour - startingHour) * (4 * heightPerDuration) + (minutes / duration) * heightPerDuration); //
        int16_t w = ((display.width() - offsetLeft) / days) - 8;  //
        int16_t h = ((repo1["data"][j-1]["duration"].as<int>() / duration) * heightPerDuration);  //

        //
        if(repo1["data"][j-1]["subject"] != repo0["data"][j-1]["subject"] || repo1["data"][j-1]["hour"] != repo0["data"][j-1]["hour"] ||
        repo1["data"][j-1]["professor"] != repo0["data"][j-1]["professor"] || repo1["data"][j-1]["minute"] != repo0["data"][j-1]["minute"]){
          
          display.drawRect(x, y, w, h, GxEPD_RED); //
          display.setTextColor(GxEPD_RED);
          display.setCursor(x + 5, y + 5);  //
          display.println(repo1["data"][j-1]["subject"].as<char*>());
          display.setCursor(x + 5, y + 15); //
          display.println(repo1["data"][j-1]["professor"].as<char*>());
          
        }else{
          
          display.drawRect(x, y, w, h, GxEPD_BLACK);  //
          display.setTextColor(GxEPD_BLACK);
          display.setCursor(x + 5, y + 5);  //
          display.println(repo1["data"][j-1]["subject"].as<char*>());
          display.setCursor(x + 5, y + 15); //
          display.println(repo1["data"][j-1]["professor"].as<char*>());    
        }
      }
    }
  }else if(infoMode["mode"] == 1){
      
    /*receiveImage = true;
    checkForImage(imageUrl);
    drawBitmapFromSpiffs("image.bmp", 0, 0, false);
    receiveImage = false;
    return;*/
    
  }else if(infoMode["mode"] == 2){
    
    display.setTextColor(GxEPD_RED);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(display.width()/4, display.height()/4);
    display.println(F("Dieser Raum ist momentan geschlossen!"));
    display.setCursor((display.width()/4), (display.height()/4)+ 20);
    display.println("Grund: ");
    display.println(closedRoom["info"].as<char*>());
    
  }
  storedData.close();
}




//Draw Template from hex array
void drawTemplate(){
  display.fillScreen(GxEPD_BLACK);
  display.drawBitmap(0, 0, imageBitmap, 640, 384, GxEPD_WHITE); 
}      




void loop() {
  getJsonDataFrom_HTTP(jsonUrl);
  
  if(connectionError == true){
    display.refresh();
    display.drawPaged(drawTempData, 0);
  }else{
    display.refresh();
    display.drawPaged(drawData, 0);
    writeJsonTo_SPIFFS();
  }
  delay(1000*60);
 }



 static const uint16_t input_buffer_pixels = 800; // may affect performance

static const uint16_t max_row_width = 800; // for up to 7.5" display 800x480
static const uint16_t max_palette_pixels = 256; // for depth <= 8

uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w

void drawBitmapFromSpiffs(const char *filename, int16_t x, int16_t y, bool with_color)
{
  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true; // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display.epd2.WIDTH) || (y >= display.epd2.HEIGHT)) return;
  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');
  file = SPIFFS.open(filename, "r");
  if (!file)
  {
    Serial.print("File not found");
    return;
  }
  // Parse BMP header
  if (read16(file) == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width  = read32(file);
    uint32_t height = read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      Serial.print("File size: "); Serial.println(fileSize);
      Serial.print("Image Offset: "); Serial.println(imageOffset);
      Serial.print("Header size: "); Serial.println(headerSize);
      Serial.print("Bit Depth: "); Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8) rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0)
      {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.epd2.WIDTH)  w = display.epd2.WIDTH  - x;
      if ((y + h - 1) >= display.epd2.HEIGHT) h = display.epd2.HEIGHT - y;
      if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish, colored;
        if (depth == 1) with_color = false;
        if (depth <= 8)
        {
          if (depth < 8) bitmask >>= depth;
          //file.seek(54); //palette is always @ 54
          file.seek(imageOffset - (4 << depth)); // 54 for regular, diff for colorsimportant
          for (uint16_t pn = 0; pn < (1 << depth); pn++)
          {
            blue  = file.read();
            green = file.read();
            red   = file.read();
            file.read();
            whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        display.clearScreen();
        uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
        for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) // for each line
        {
          uint32_t in_remain = rowSize;
          uint32_t in_idx = 0;
          uint32_t in_bytes = 0;
          uint8_t in_byte = 0; // for depth <= 8
          uint8_t in_bits = 0; // for depth <= 8
          uint8_t out_byte = 0xFF; // white (for w%8!=0 border)
          uint8_t out_color_byte = 0xFF; // white (for w%8!=0 border)
          uint32_t out_idx = 0;
          file.seek(rowPosition);
          for (uint16_t col = 0; col < w; col++) // for each pixel
          {
            // Time to read more pixel data?
            if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
            {
              in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
              in_remain -= in_bytes;
              in_idx = 0;
            }
            switch (depth)
            {
              case 24:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 16:
                {
                  uint8_t lsb = input_buffer[in_idx++];
                  uint8_t msb = input_buffer[in_idx++];
                  if (format == 0) // 555
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                    red   = (msb & 0x7C) << 1;
                  }
                  else // 565
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                    red   = (msb & 0xF8);
                  }
                  whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                  colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                }
                break;
              case 1:
              case 4:
              case 8:
                {
                  if (0 == in_bits)
                  {
                    in_byte = input_buffer[in_idx++];
                    in_bits = 8;
                  }
                  uint16_t pn = (in_byte >> bitshift) & bitmask;
                  whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  in_byte <<= depth;
                  in_bits -= depth;
                }
                break;
            }
            if (whitish)
            {
              // keep white
            }
            else if (colored && with_color)
            {
              out_color_byte &= ~(0x80 >> col % 8); // colored
            }
            else
            {
              out_byte &= ~(0x80 >> col % 8); // black
            }
            if ((7 == col % 8) || (col == w - 1)) // write that last byte! (for w%8!=0 border)
            {
              output_row_color_buffer[out_idx] = out_color_byte;
              output_row_mono_buffer[out_idx++] = out_byte;
              out_byte = 0xFF; // white (for w%8!=0 border)
              out_color_byte = 0xFF; // white (for w%8!=0 border)
            }
          } // end pixel
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          display.writeImage(output_row_mono_buffer, output_row_color_buffer, x, yrow, w, 1);
        } // end line
        Serial.print("loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
        display.refresh();
      }
    }
  }
  file.close();
  if (!valid)
  {
    Serial.println("bitmap format not handled.");
  }
}


uint16_t read16(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
