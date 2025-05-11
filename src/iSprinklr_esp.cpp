#include <Arduino.h>
#include "HunterRoam.h"
#include "WebServer.h"
#include "iSprinklrNetwork.h"

#define LED 2

// Web server and network manager
WebServer webServer;
iSprinklrNetwork* network;

// Network mode from build flags 
// 1 = Ethernet Only, 2 = WiFi Only
#ifndef NETWORK_MODE
  #define NETWORK_MODE 1  // Default to Ethernet only mode if not specified
#endif

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    Serial.println("iSprinklr ESP32 starting...");
    
    // Get network manager instance
    network = iSprinklrNetwork::getInstance();
    
    // Configure WiFi if credentials are available
    #if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    network->configureWiFi(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("WiFi credentials configured.");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("Password: ");
    Serial.println(WIFI_PASSWORD);
    #else
    Serial.println("Note: WiFi credentials not configured. Only Ethernet will be available.");
    #endif
    
    // Determine network mode from build flags
    NetworkMode mode;
    switch (NETWORK_MODE) {
        case 2:
            mode = MODE_WIFI;
            Serial.println("Network Mode: WiFi Only");
            break;
        default:
            mode = MODE_ETHERNET;
            Serial.println("Network Mode: Ethernet Only");
            break;
    }
    
    // Initialize network
    if (!network->begin(mode)) {
        Serial.println("ERROR: Failed to connect to network!");
        // Continue anyway - maybe the connection will be established later
    }
    
    // Wait up to 30 seconds for network connection
    unsigned long startTime = millis();
    while (!network->isConnected() && (millis() - startTime < 30000)) {
        Serial.println("Waiting for network connection...");
        delay(1000);
    }
    
    if (network->isConnected()) {
        Serial.print("Connected to network via ");
        Serial.println(network->getNetworkType());
        Serial.print("IP Address: ");
        Serial.println(network->getIP());
    } else {
        Serial.println("WARNING: No network connection established!");
    }
    
    // Start the web server
    webServer.begin();
    Serial.println("System initialization complete");
}

void loop()
{
    // Check network status periodically
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) {  // Every 10 seconds
        lastCheck = millis();
        
        if (network->isConnected()) {
            // All good, nothing to do
        } else {
            Serial.println("Network connection lost! Trying to reconnect...");
            network->begin();  // Try to reconnect with same settings
        }
    }
    
    // Do nothing else. Everything is done in another task by the web server
    delay(1000);
}
