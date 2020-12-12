#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
// 24 x 24 gridicons_layout
const unsigned char gridicons_layout[] PROGMEM = { /* 0X01,0X01,0XB4,0X00,0X40,0X00, */
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xE0, 0x7F, 0xFF, 0xC0, 0x3E, 0x0F, 0xC0, 
0x3C, 0x07, 0xC0, 0x3C, 0x07, 0xC0, 0x3C, 0x07, 
0xC0, 0x3C, 0x07, 0xE0, 0x7C, 0x07, 0xFF, 0xFC, 
0x07, 0xFF, 0xFC, 0x07, 0xF0, 0x1C, 0x07, 0xE0, 
0x0C, 0x07, 0xE0, 0x0C, 0x07, 0xE0, 0x0C, 0x07, 
0xE0, 0x0C, 0x07, 0xE0, 0x0C, 0x07, 0xE0, 0x0C, 
0x07, 0xE0, 0x0E, 0x0F, 0xE0, 0x0F, 0xFF, 0xF0, 
0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
