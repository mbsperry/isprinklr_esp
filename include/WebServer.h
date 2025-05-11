#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <WiFi.h>
#include "iSprinklrNetwork.h"
#include "HunterRoam.h"

// Define SmartPort pin
#define SMARTPORT_PIN 18

// ESP System headers
#include "esp_system.h"
#include "esp_chip_info.h"
#ifdef ESP_IDF_VERSION_MAJOR
#include "esp_idf_version.h"
#endif

class WebServer {
private:
    AsyncWebServer server;
    HunterRoam hunter_controller;
    
public:
    WebServer();
    void begin();
    void setupRoutes();
};

#endif // WEB_SERVER_H
