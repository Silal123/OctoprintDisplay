#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

extern Adafruit_GC9A01A tft; // Wenn du ein TFT-Display verwendest

void drawArc(int x, int y, int radius, float startAngle, float endAngle, uint16_t color);
void drawThickArc(int x, int y, int radius, int thickness, float startAngle, float endAngle, uint16_t color);
void drawPerArc(int i, uint16_t color);
void displayCenteredText(String text, int x, int y, uint8_t text_size, uint16_t color);
void displayText(String text, int x, int y, uint8_t text_size, uint16_t color);
String convertSeconds(int totalSeconds);
String cutText(String text, int chars);

#endif
