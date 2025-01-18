#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiClient.h>

#include <EEPROM.h>

#include <ArduinoJson.h>

#include <iostream>
#include <iomanip>
#include <string>

#include "utils.h"

//########################################
#define EEPROM_SIZE 512

const char* WIFI_SSID = "Silas-Netz";
const char* WIFI_PASS = "tWL6us6J";

const char* AP_SSID = "Fallback Printstatus";
const char* AP_PASS = "fallback";

const char* ESP_HOSTNAME = "printstatus";

String OCTOPRINT = "http://192.168.178.76/";
String OCTOPRINT_TOKEN = "5PPmKpAv6Nrmk0UAJVX910PohuEwvPp6uk6JRiUOf04";

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    D0
#define TFT_DC    D1
#define TFT_RST   D2
//########################################

JsonDocument OCTO_DATA;

HTTPClient sender;
WiFiClient wifiClient;

Adafruit_GC9A01A tft = Adafruit_GC9A01A(TFT_CS, TFT_DC, TFT_RST);

AsyncWebServer server(80);

//########################################

static const unsigned char PROGMEM image_music_pause_bits[] = {0xf9,0xf0,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0x89,0x10,0xf9,0xf0,0x00,0x00};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>Captive Portal</title>
  </head>
  <body>
    <h1>Test of default page!</h1>
    <p>Test!</p>
  </body>
</html>
)rawliteral";

void startServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/htmt", index_html);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<!DOCTYPE html><html><head><title>Captive Portal</title></head><body><h1>Page not found!</h1><p>Please connect to / default page!</p></body></html>");
  });

  server.begin();
}

void startCaptivePortal() {
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("Access Point gestartet");

  Serial.print("IP-Adresse des Access Points: ");
  Serial.println(WiFi.softAPIP());

  displayCenteredText("Ap: " + String(AP_SSID), tft.width() / 2, (tft.height() / 2) - 20, 1, GC9A01A_RED);
  displayCenteredText("Pass: " + String(AP_PASS), tft.width() / 2, (tft.height() / 2) - 5, 1, GC9A01A_RED);
  displayCenteredText(WiFi.softAPIP().toString(), tft.width() / 2, (tft.height() / 2) + 20, 2, GC9A01A_RED);

  startServer();
}

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
  //startCaptivePortal();

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
    float percent = completion;
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
  //if (state == "Printing" || state == "Pausing" || state == "Paused") {
  //  loadPrintingScreen();
  //} else {
  //  loadNoPrintScreen();
  //}
  delay(2000);
  loadPrintingScreen();
}