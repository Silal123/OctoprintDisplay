#include "utils.h"
#include <math.h>  // Für trigonometrische Funktionen

extern Adafruit_GC9A01A tft;  // Wenn du ein TFT-Display verwendest

void drawArc(int x, int y, int radius, float startAngle, float endAngle, uint16_t color) {
  int segments = 100;
  for (int i = 0; i < segments; i++) {
    float angle1 = startAngle + (endAngle - startAngle) * i / segments;
    float angle2 = startAngle + (endAngle - startAngle) * (i + 1) / segments;
    
    int x1 = x + radius * cos(angle1);
    int y1 = y + radius * sin(angle1);
    int x2 = x + radius * cos(angle2);
    int y2 = y + radius * sin(angle2);

    tft.drawLine(x1, y1, x2, y2, color);
  }
}

void drawThickArc(int x, int y, int radius, int thickness, float startAngle, float endAngle, uint16_t color) {
  int segments = 100;

  for (int i = 0; i < thickness; i++) {
    int currentRadius = radius - i;
    for (int j = 0; j < segments; j++) {
      float angle1 = startAngle + (endAngle - startAngle) * j / segments;
      float angle2 = startAngle + (endAngle - startAngle) * (j + 1) / segments;
      
      int x1 = x + currentRadius * cos(angle1);
      int y1 = y + currentRadius * sin(angle1);
      int x2 = x + currentRadius * cos(angle2);
      int y2 = y + currentRadius * sin(angle2);
      
      tft.drawLine(x1, y1, x2, y2, color);
    }
  }
}

void drawPerArc(int i, uint16_t color) {
  float factor = ((i / 100.0) + 1) * 3.14159;

  if (factor > 2 * 3.14159) {
    factor = 2 * 3.14159;
  }

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = tft.width() / 2 - 10;
  int thickness = 20;

  drawArc(centerX, centerY, radius + 1, 3.14159, 2 * 3.14159, GC9A01A_WHITE);
  drawArc(centerX, centerY, radius - thickness, 3.14159, 2 * 3.14159, GC9A01A_WHITE);

  drawThickArc(centerX, centerY, radius, thickness, 3.14159, factor, color);
}

void displayCenteredText(String text, int x, int y, uint8_t text_size, uint16_t color) {
  int16_t textWidth = text.length() * 6 * text_size;
  int16_t tx = x - (textWidth / 2);
  int16_t ty = (y - ((12 * text_size) / 2)) ;

  tft.setCursor(tx, ty);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(text);
}

void displayText(String text, int x, int y, uint8_t text_size, uint16_t color) {
  int16_t ty = (y - ((12 * text_size) / 2)) ;

  tft.setCursor(x, ty);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(text);
}

String convertSeconds(int totalSeconds) {
    if (totalSeconds < 60) {
        return String(totalSeconds) + "s";
    }

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int days = minutes / 1440;
    minutes = minutes % 1440;

    String formattedTime = String(days) + ":";
    formattedTime += String(minutes) + ":";
    formattedTime += String(seconds);

    return formattedTime;
}

String cutText(String text, int chars) {
  if (text.length() > chars) {
    text = text.substring(0, chars) + "...";
  }
  
  return text;
}