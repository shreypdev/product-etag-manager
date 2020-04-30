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
#include <FirebaseESP32.h>
#include "esp_wifi.h"
#include "Esp.h"
#include "ESPmDNS.h"
#include <Button2.h>
#include "board_def.h"
#include "clean_display.h"

#define FILESYSTEM SPIFFS

#define HOST_SECRET_KEY "abc-123"
#define WIFI_SSID "BNET"
#define WIFI_PASSWORD "Victoria203!"
#define FIREBASE_HOST "https://tag-manager-b3096.firebaseio.com/"
#define FIREBASE_AUTH "3vbv5Q4wu7jzImPkSD6N3RiRV0tDXmSkiyrIm3uY"
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

void serveAPI()
{
    server.on("/find-tags", HTTP_GET, [](AsyncWebServerRequest * request) {
        String response;
        serializeJson(findTags("find-tag", "tcp"), response);
        request->send(200, "application/json", response);
    });
    server.on("/confirm-host-secret-key", HTTP_POST, [](AsyncWebServerRequest * request) {
        String responseMessage;

        String paramName_hostSecretKey = request->getParam(0)->name();
        String paramVal_hostSecretKey = request->getParam(0)->value();

        if(paramName_hostSecretKey == "hostSecretKey"){
          if(paramVal_hostSecretKey == HOST_SECRET_KEY){
            responseMessage = "true";
          } else {
            responseMessage = "false";
          }
        } else {
          responseMessage = "false";
        }
        
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", responseMessage);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    server.on("/tag-ip-changed", HTTP_POST, [](AsyncWebServerRequest * request) {
        String responseMessage;

        String paramName_tagID = request->getParam(0)->name();
        String paramVal_tagID = request->getParam(0)->value();
        String paramName_oldTagIP = request->getParam(1)->name();
        String paramVal_oldTagIP = request->getParam(1)->value();
        String paramName_newTagIP = request->getParam(2)->name();
        String paramVal_newTagIP = request->getParam(2)->value();

        if(paramName_tagID == "tagID" && paramName_oldTagIP == "oldTagIP" && paramName_newTagIP == "newTagIP"){
          if(paramVal_tagID != "" && paramVal_oldTagIP != "" && paramVal_newTagIP != ""){

            // FirebaseData firebaseData;
            FirebaseData firebaseData;
            String hostSecretKey = HOST_SECRET_KEY;
            if (Firebase.get(firebaseData, "Data/hosts/" + hostSecretKey + "/tags/tagID-detailKey-mapping/" + paramVal_tagID + "")){
              String tagDetailsKey = firebaseData.stringData();
              if (Firebase.get(firebaseData, "Data/hosts/" + hostSecretKey + "/tags/detail/" + tagDetailsKey + "/tag_ip")){
                String tag_ip = firebaseData.stringData();

                if(tag_ip != paramVal_newTagIP){
                  FirebaseJson json;
                  json.set("tagDetailsKey", tagDetailsKey);
                  json.set("oldTagIP", paramVal_oldTagIP);
                  json.set("newTagIP", paramVal_newTagIP);
                  json.set("hostID", hostSecretKey);
                  //TODO: Add tag Name

                  if (Firebase.set(firebaseData, "Data/hosts/" + hostSecretKey + "/notifications/0", json)){
                     responseMessage = "true";
                  } else {
                     Serial.println("REASON: " + firebaseData.errorReason());
                     responseMessage = "false";
                  }
                }
                
                responseMessage = "true";
              } else {
                Serial.println("REASON: " + firebaseData.errorReason());
                responseMessage = "false";
              }
            } else {
              Serial.println("REASON: " + firebaseData.errorReason());
              responseMessage = "false";
            }
          } else {
            responseMessage = "false";
          }
        } else {
          responseMessage = "false";
        }
        
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", responseMessage);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
}

void serveServices()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("signin.html");
    
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
            device["tagName"] = MDNS.hostname(i);
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

    //Connect Firebase
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

void printResult(FirebaseData &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}

void loop()
{
    //Nothing to do here
}
