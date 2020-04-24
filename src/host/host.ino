// include library, include base class, make path known
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

//#define DEFALUT_FONT  FreeMono9pt7b
// #define DEFALUT_FONT  FreeMonoBoldOblique9pt7b
// #define DEFALUT_FONT FreeMonoBold9pt7b
// #define DEFALUT_FONT FreeMonoOblique9pt7b
#define DEFALUT_FONT FreeSans9pt7b
// #define DEFALUT_FONT FreeSansBold9pt7b
// #define DEFALUT_FONT FreeSansBoldOblique9pt7b
// #define DEFALUT_FONT FreeSansOblique9pt7b
// #define DEFALUT_FONT FreeSerif9pt7b
// #define DEFALUT_FONT FreeSerifBold9pt7b
// #define DEFALUT_FONT FreeSerifBoldItalic9pt7b
// #define DEFALUT_FONT FreeSerifItalic9pt7b

const GFXfont *fonts[] = {
    &FreeMono9pt7b,
    &FreeMonoBoldOblique9pt7b,
    &FreeMonoBold9pt7b,
    &FreeMonoOblique9pt7b,
    &FreeSans9pt7b,
    &FreeSansBold9pt7b,
    &FreeSansBoldOblique9pt7b,
    &FreeSansOblique9pt7b,
    &FreeSerif9pt7b,
    &FreeSerifBold9pt7b,
    &FreeSerifBoldItalic9pt7b,
    &FreeSerifItalic9pt7b
};

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include "SD.h"
#include "SPI.h"
#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "esp_wifi.h"
#include "Esp.h"
#include "ESPmDNS.h"
#include <Button2.h>
#include "board_def.h"
#include "clean_display.h"

#define FILESYSTEM SPIFFS

#define WIFI_SSID "BNET"
#define WIFI_PASSWORD "Victoria203!"
#define CHANNEL_0 0
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00

String WebServerStart(void);
void showMianPage(String ip);
void displayInit(void);
bool setPowerBoostKeepOn(int en);
void clean_display();
DynamicJsonDocument findTags(const char * service, const char * proto);

typedef enum {
    RIGHT_ALIGNMENT = 0,
    LEFT_ALIGNMENT,
    CENTER_ALIGNMENT,
} Text_alignment;

AsyncWebServer server(80);

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

void displayText(const String &str, int16_t y, uint8_t alignment)
{
    int16_t x = 0;
    int16_t x1, y1;
    uint16_t w, h;
    display.setCursor(x, y);
    display.getTextBounds(str, x, y, &x1, &y1, &w, &h);

    switch (alignment) {
    case RIGHT_ALIGNMENT:
        display.setCursor(display.width() - w - x1, y);
        break;
    case LEFT_ALIGNMENT:
        display.setCursor(0, y);
        break;
    case CENTER_ALIGNMENT:
        display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
        break;
    default:
        break;
    }
    display.println(str);
}

void serveFiles()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("signin.html");

    server.on("signup.html", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "signup.html", "text/html");
    });
    server.on("css/main.css", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "css/main.css", "text/css");
    });
    server.on("css/signin-up.css", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "css/main.css", "text/css");
    });
    server.on("js/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "js/jquery.min.js", "application/javascript");
    });
    server.on("js/tbdValidate.js", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "js/tbdValidate.js", "application/javascript");
    });
}

void serveAPI()
{
    server.on("/find-tags", HTTP_GET, [](AsyncWebServerRequest * request) {
        String response;
        serializeJson(findTags("find-tag", "tcp"), response);
        request->send(200, "application/json", response);
    });
}

void serveServices()
{
    serveFiles();
    serveAPI();
    
    server.onNotFound([](AsyncWebServerRequest * request) {
        request->send(404, "text/plain", "Not found");
    });
}

String WebServerStart(void)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        esp_restart();
    }
    Serial.println(F("WiFi connected"));
    Serial.println("");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("Tag Manager - Host")) {
        Serial.println("MDNS responder started");
    }
    
    serveServices();

    MDNS.addService("http", "tcp", 80);

    server.begin();

    return WiFi.localIP().toString();
}

void showMianPage(String ip)
{
    displayInit();
    display.fillScreen(GxEPD_WHITE);
    displayText("Host Local IP:", 50, CENTER_ALIGNMENT);
    displayText(ip, 80, CENTER_ALIGNMENT);
    display.update();
}

void displayInit(void)
{
    static bool isInit = false;
    if (isInit) {
        return;
    }
    isInit = true;
    display.init();
    display.setRotation(1);
    display.eraseDisplay(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DEFALUT_FONT);
    display.setTextSize(0);
    displayText("Tag Manager", 50, CENTER_ALIGNMENT);
    display.update();
    delay(3000);
}

bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}

void clean_display() {
    Serial.println("Adafruit EPD: Starting to clear screen");
    display_to_clean.begin();
    display_to_clean.clearBuffer();
    display_to_clean.display();
    Serial.println("Adafruit EPD: Screen cleared");
}

DynamicJsonDocument findTags(const char * service, const char * proto){
    DynamicJsonDocument  root(256);
    JsonArray devices = root.createNestedArray("devices");
  
    Serial.printf("Browsing for service _%s._%s.local. ... ", service, proto);
    int n = MDNS.queryService(service, proto);
    if (n == 0) {
        Serial.println("no services found");
    } else {
        Serial.print(n);
        Serial.println(" service(s) found and responded to client");
        for (int i = 0; i < n; ++i) {
            DynamicJsonDocument  device(256);
            device["hostName"] = MDNS.hostname(i);
            device["ip"] = MDNS.IP(i).toString();
            device["port"] = MDNS.port(i);
            devices.add(device);
        }
    }
    return root;
}

void setup()
{
    Serial.begin(115200);

    clean_display();

    #ifdef ENABLE_IP5306
        Wire.begin(I2C_SDA, I2C_SCL);
        bool ret = setPowerBoostKeepOn(1);
        Serial.printf("Power KeepUp %s\n", ret ? "PASS" : "FAIL");
    #endif

    // It is only necessary to turn on the power amplifier power supply on the T5_V24 board.
    #ifdef AMP_POWER_CTRL
        pinMode(AMP_POWER_CTRL, OUTPUT);
        digitalWrite(AMP_POWER_CTRL, HIGH);
    #endif

    //To see the files in the memory
    if (!FILESYSTEM.begin()) {
        Serial.println("FILESYSTEM is not database");
        Serial.println("Please use Arduino ESP32 Sketch data Upload files");
        while (1) {
            delay(1000);
        }
    }

    String ip = WebServerStart();
    showMianPage(ip);
}

void loop()
{
    //Nothing to do here
}
