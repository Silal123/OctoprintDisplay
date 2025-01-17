#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include <iostream>
#include <iomanip>
#include <string>

#define EEPROM_SIZE 512

const char* WIFI_SSID = "Silas-Netz";
const char* WIFI_PASS = "tWL6us6J";

JsonDocument OCTO_DATA;

String OCTOPRINT = "http://192.168.178.76/";
String OCTOPRINT_TOKEN = "5PPmKpAv6Nrmk0UAJVX910PohuEwvPp6uk6JRiUOf04";

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    D0
#define TFT_DC    D1
#define TFT_RST   D2

HTTPClient sender;
WiFiClient wifiClient;

Adafruit_GC9A01A tft = Adafruit_GC9A01A(TFT_CS, TFT_DC, TFT_RST);

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

static const unsigned char PROGMEM image_music_pause_bits[] = {0xf9,0xf0,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0xf9,0xf0,0x00,0x00};

void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void fetchData() {
  Serial.println("Fetching Data!");
  Serial.println(OCTOPRINT);
  if (sender.begin(wifiClient, OCTOPRINT + "api/job")) {
    sender.addHeader("Authorization", "Bearer " + OCTOPRINT_TOKEN);
    int code = sender.GET();
    Serial.println("Http Code: " + String(code));
    if (code > 0) {
      if (code == HTTP_CODE_OK) {
        String data = sender.getString();
        Serial.println(data);
        deserializeJson(OCTO_DATA, data);
      }
    }

    Serial.println("Ready!");
    sender.end();
  }
  
}

void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(GC9A01A_BLACK);

  Serial.println("Trying Connecting to Wifi!");
  setupWifi();

  displayCenteredText("Connecting...", tft.width() / 2, tft.height() / 2, 2, GC9A01A_RED);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println("Connecting...");
  }
}

void loadPrintingScreen() {
  tft.fillScreen(GC9A01A_BLACK);

  if (OCTO_DATA["state"] == "Paused") {
    tft.drawBitmap(30, (tft.height() / 2) + 30, image_music_pause_bits, 12, 16, GC9A01A_ORANGE);
  }

  if (!OCTO_DATA["progress"]["printTimeLeft"].isNull()) {
    displayCenteredText(convertSeconds(OCTO_DATA["progress"]["printTimeLeft"]), tft.width() / 2, (tft.height() / 2) + 10, 2, GC9A01A_WHITE);
  } else {
    displayCenteredText("00:00", tft.width() / 2, (tft.height() / 2) + 10, 2, GC9A01A_RED);
  }

  if (!OCTO_DATA["job"]["file"]["name"].isNull()) {
    displayText("File: " + cutText(String(OCTO_DATA["job"]["file"]["name"]), 15), (tft.width() / 2) - 40, (tft.height() / 2) + 30, 1, GC9A01A_WHITE);
  }

  if (!OCTO_DATA["state"].isNull()) {
    displayText("State: " + cutText(String(OCTO_DATA["state"]), 13), (tft.width() / 2) - 40, (tft.height() / 2) + 40, 1, GC9A01A_WHITE);
  }

  if (!OCTO_DATA["progress"]["completion"].isNull()) {
    float completion = OCTO_DATA["progress"]["completion"];
    float percent = completion * 10;
    displayCenteredText(String(percent) + "%", tft.width() / 2, (tft.height() / 2) - 20, 4, GC9A01A_WHITE);
    drawPerArc(percent, GC9A01A_GREEN);
  } else {
    drawPerArc(100, GC9A01A_RED);
    displayCenteredText("None", tft.width() / 2, (tft.height() / 2) - 20, 4, GC9A01A_RED);
  }
}

void loadNoPrintScreen() {
  tft.fillScreen(GC9A01A_BLACK);

  displayCenteredText("No Print!", tft.width() / 2, tft.height() / 2, 3, GC9A01A_RED);
  displayCenteredText("There is no print in progress!", tft.width() / 2, (tft.height() / 2) + 20, 2, GC9A01A_RED);
}

void loop() {
  fetchData();
  String state = OCTO_DATA["state"];
  if (state == "Printing" || state == "Pausing" || state == "Paused") {
    loadPrintingScreen();
  } else {
    loadNoPrintScreen();
  }
  
  delay(10000);
}
