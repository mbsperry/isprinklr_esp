#include "WebServer.h"

WebServer::WebServer() : server(80), hunter_controller(SMARTPORT_PIN) {
}

void WebServer::begin() {
    setupRoutes();
    server.begin();
    Serial.println("HTTP server started");
}

void WebServer::setupRoutes() {
    // Enhanced status endpoint with detailed system information
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Status check requested");
        
        // Create JSON response with detailed system information
        DynamicJsonDocument doc(1024);
        
        // Basic status
        doc["status"] = "ok";
        
        // System uptime
        doc["uptime_ms"] = millis();
        
        // ESP Chip Information
        JsonObject chip = doc.createNestedObject("chip");
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        
        // Set chip model
        switch(chip_info.model) {
            case CHIP_ESP32:
                chip["model"] = "ESP32";
                break;
            case CHIP_ESP32S2:
                chip["model"] = "ESP32-S2";
                break;
            case CHIP_ESP32S3:
                chip["model"] = "ESP32-S3";
                break;
            case CHIP_ESP32C3:
                chip["model"] = "ESP32-C3";
                break;
            default:
                chip["model"] = "Unknown";
        }
        
        chip["revision"] = chip_info.revision;
        chip["cores"] = chip_info.cores;
        
        // ESP-IDF Version
        #ifdef ESP_IDF_VERSION_MAJOR
            char idf_version[16];
            snprintf(idf_version, sizeof(idf_version), "%d.%d.%d", 
                     ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
            doc["idf_version"] = idf_version;
        #else
            doc["idf_version"] = esp_get_idf_version();
        #endif
        
        // Reset Reason
        esp_reset_reason_t reason = esp_reset_reason();
        const char* reset_reasons[] = {
            "Unknown", "Power on", "External pin", "Software reset", 
            "Software panic", "Interrupt watchdog", "Task watchdog", 
            "Other watchdog", "Exiting deep sleep", "Brownout", "SDIO"
        };
        doc["reset_reason"] = (reason < ESP_RST_SDIO + 1) ? reset_reasons[reason] : "Other";
        
        // Memory Information
        JsonObject memory = doc.createNestedObject("memory");
        memory["free_heap"] = esp_get_free_heap_size();
        memory["min_free_heap"] = esp_get_minimum_free_heap_size();
        
        // Network Status
        JsonObject network = doc.createNestedObject("network");
        iSprinklrNetwork* net = iSprinklrNetwork::getInstance();
        
        // Get connection status
        network["connected"] = net->isConnected();
        network["type"] = net->getNetworkType();
        
        // IP address information
        IPAddress ip = net->getIP();
        network["ip"] = ip.toString();
        
        // Additional information based on network type
        if (net->getNetworkType() == "WiFi") {
            network["ssid"] = WiFi.SSID();
            network["rssi"] = WiFi.RSSI();
            network["gateway"] = WiFi.gatewayIP().toString();
            network["subnet"] = WiFi.subnetMask().toString();
            network["dns"] = WiFi.dnsIP().toString();
        } else if (net->getNetworkType() == "Ethernet") {
            network["mac"] = ETH.macAddress();
            network["gateway"] = ETH.gatewayIP().toString();
            network["subnet"] = ETH.subnetMask().toString();
            network["speed"] = String(ETH.linkSpeed()) + " Mbps";
            network["duplex"] = ETH.fullDuplex() ? "Full" : "Half";
        }
        
        // Task Information
        JsonObject task = doc.createNestedObject("task");
        task["stack_hwm"] = uxTaskGetStackHighWaterMark(NULL);
        
        // Convert to string
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    // Setup JSON handler with simplified error handling
    server.on("/api/start", HTTP_POST, 
        // Regular request handler is empty as we'll handle everything in the body handler
        [](AsyncWebServerRequest *request) {},
        // No upload handler needed
        NULL,
        // Body handler for both valid and invalid JSON
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0 && total > 0) { // Ensure we process the body only once
                Serial.println("Start command received");
                
                // Parse the JSON directly from the received data
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, (const char*)data, len);
                
                if (error) {
                    Serial.print("JSON Error: ");
                    Serial.println(error.c_str());
                    request->send(400, "application/json", "{\"error\":\"Invalid JSON: " + String(error.c_str()) + "\"}");
                    return;
                }
                
                // Validate required parameters
                if (!doc.containsKey("zone") || !doc.containsKey("minutes")) {
                    request->send(400, "application/json", "{\"error\":\"Missing required parameters\"}");
                    return;
                }
                
                int zone = doc["zone"].as<int>();
                int minutes = doc["minutes"].as<int>();
                
                // Validate zone is within valid range (1-20)
                if (zone < 1 || zone > 20) {
                    request->send(400, "application/json", "{\"error\":\"Zone must be between 1 and 20\"}");
                    return;
                }
                
                // Validate minutes is within valid range (0-120)
                // Allow 0 minutes for testing stop functionality (HunterRoam::stopZone uses 0 minutes)
                if (minutes < 0 || minutes > 120) {
                    request->send(400, "application/json", "{\"error\":\"Minutes must be between 0 and 120\"}");
                    return;
                }
                
                Serial.print("Zone: ");
                Serial.println(zone);
                Serial.print("Minutes: ");
                Serial.println(minutes);
                
                // Use the hunter_controller to start the zone
                byte result = hunter_controller.startZone(zone, minutes);
                
                String status = "started";
                String errorMessage = "";
                
                // Check if there was an error
                if (result != 0) {
                    status = "error";
                    errorMessage = hunter_controller.errorHint(result);
                    Serial.print("Error starting zone: ");
                    Serial.println(errorMessage);
                    
                    DynamicJsonDocument responseDoc(256);
                    responseDoc["status"] = status;
                    responseDoc["zone"] = zone;
                    responseDoc["minutes"] = minutes;
                    responseDoc["error"] = errorMessage;
                    
                    String response;
                    serializeJson(responseDoc, response);
                    request->send(500, "application/json", response);
                } else {
                    DynamicJsonDocument responseDoc(256);
                    responseDoc["status"] = status;
                    responseDoc["zone"] = zone;
                    responseDoc["minutes"] = minutes;
                    
                    String response;
                    serializeJson(responseDoc, response);
                    request->send(200, "application/json", response);
                }
            }
        }
    );

    server.on("/api/stop", HTTP_POST, 
        // Regular request handler is empty as we'll handle everything in the body handler
        [](AsyncWebServerRequest *request) {},
        // No upload handler needed
        NULL,
        // Body handler for both valid and invalid JSON
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0 && total > 0) { // Ensure we process the body only once
                Serial.println("Stop command received");
                
                // Parse the JSON directly from the received data
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, (const char*)data, len);
                
                if (error) {
                    Serial.print("JSON Error: ");
                    Serial.println(error.c_str());
                    request->send(400, "application/json", "{\"error\":\"Invalid JSON: " + String(error.c_str()) + "\"}");
                    return;
                }
                
                // Validate required parameters
                if (!doc.containsKey("zone")) {
                    request->send(400, "application/json", "{\"error\":\"Missing required parameter: zone\"}");
                    return;
                }
                
                int zone = doc["zone"].as<int>();
                
                // Validate zone is within valid range (1-20)
                if (zone < 1 || zone > 20) {
                    request->send(400, "application/json", "{\"error\":\"Zone must be between 1 and 20\"}");
                    return;
                }
                
                Serial.print("Stopping zone: ");
                Serial.println(zone);
                
                // Use the hunter_controller to stop the zone
                byte result = hunter_controller.stopZone(zone);
                
                String status = "stopped";
                String errorMessage = "";
                
                // Check if there was an error
                if (result != 0) {
                    status = "error";
                    errorMessage = hunter_controller.errorHint(result);
                    Serial.print("Error stopping zone: ");
                    Serial.println(errorMessage);
                    
                    DynamicJsonDocument responseDoc(256);
                    responseDoc["status"] = status;
                    responseDoc["zone"] = zone;
                    responseDoc["error"] = errorMessage;
                    
                    String response;
                    serializeJson(responseDoc, response);
                    request->send(500, "application/json", response);
                } else {
                    DynamicJsonDocument responseDoc(256);
                    responseDoc["status"] = status;
                    responseDoc["zone"] = zone;
                    
                    String response;
                    serializeJson(responseDoc, response);
                    request->send(200, "application/json", response);
                }
            }
        }
    );
}
