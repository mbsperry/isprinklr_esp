#include "iSprinklrNetwork.h"

// Initialize static instance
iSprinklrNetwork* iSprinklrNetwork::_instance = nullptr;

// Static event tracking for Ethernet
static bool ethConnected = false;
static bool wifiConnected = false;

iSprinklrNetwork::iSprinklrNetwork() {
    _connected = false;
    _mode = MODE_ETHERNET;
    
    // Default Ethernet settings for ESP32-S3-Ethernet
    _ethCsPin = 14;
    _ethIntPin = 10;
    _ethRstPin = 9;
    _ethMisoPin = 12;
    _ethMosiPin = 11;
    _ethSclkPin = 13;
    _ethAddr = 1;
    
    // Initialize fixed IP configuration as disabled
    _fixedIP.enabled = false;
    _fixedIP.ip = IPAddress(0, 0, 0, 0);
    _fixedIP.gateway = IPAddress(0, 0, 0, 0);
    _fixedIP.subnet = IPAddress(0, 0, 0, 0);
    _fixedIP.dns1 = IPAddress(0, 0, 0, 0);
    _fixedIP.dns2 = IPAddress(0, 0, 0, 0);
    
    // Check if fixed IP is configured via build flags
#ifdef FIXED_IP
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
    
    if (ip.fromString(FIXED_IP) && 
        gateway.fromString(FIXED_GATEWAY) && 
        subnet.fromString(FIXED_SUBNET)) {
        
        // Optional DNS servers
#ifdef FIXED_DNS1
        dns1.fromString(FIXED_DNS1);
#endif
#ifdef FIXED_DNS2
        dns2.fromString(FIXED_DNS2);
#endif
        
        configureFixedIP(ip, gateway, subnet, dns1, dns2);
    }
#endif
}

iSprinklrNetwork* iSprinklrNetwork::getInstance() {
    if (_instance == nullptr) {
        _instance = new iSprinklrNetwork();
    }
    return _instance;
}

void iSprinklrNetwork::configureWiFi(const char* ssid, const char* password) {
    _ssid = String(ssid);
    _password = String(password);
}

void iSprinklrNetwork::configureEthernet(int csPin, int intPin, int rstPin, int misoPin, int mosiPin, int sclkPin, int addr) {
    _ethCsPin = csPin;
    _ethIntPin = intPin;
    _ethRstPin = rstPin;
    _ethMisoPin = misoPin;
    _ethMosiPin = mosiPin;
    _ethSclkPin = sclkPin;
    _ethAddr = addr;
}

void iSprinklrNetwork::configureFixedIP(const IPAddress& ip, const IPAddress& gateway, const IPAddress& subnet, 
                                        const IPAddress& dns1, const IPAddress& dns2) {
    _fixedIP.enabled = true;
    _fixedIP.ip = ip;
    _fixedIP.gateway = gateway;
    _fixedIP.subnet = subnet;
    _fixedIP.dns1 = dns1;
    _fixedIP.dns2 = dns2;
    
    Serial.println("Fixed IP configuration enabled:");
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("Gateway: ");
    Serial.println(gateway);
    Serial.print("Subnet: ");
    Serial.println(subnet);
    if (dns1 != IPAddress(0, 0, 0, 0)) {
        Serial.print("DNS1: ");
        Serial.println(dns1);
    }
    if (dns2 != IPAddress(0, 0, 0, 0)) {
        Serial.print("DNS2: ");
        Serial.println(dns2);
    }
}

void iSprinklrNetwork::disableFixedIP() {
    _fixedIP.enabled = false;
    Serial.println("Fixed IP configuration disabled - using DHCP");
}

bool iSprinklrNetwork::begin(NetworkMode mode) {
    _mode = mode;
    _connected = false;
    
    // Register for WiFi/ETH events
    WiFi.onEvent(WiFiEventCallback);
    
    // If fixed IP is enabled, configure it before connecting
    if (_fixedIP.enabled) {
        Serial.println("Using fixed IP configuration");
    }
    
    // Start network based on mode
    if (_mode == MODE_ETHERNET) {
        // Try Ethernet
        Serial.println("Initializing Ethernet...");
        
        // Configure fixed IP if enabled (must be done before ETH.begin)
        if (_fixedIP.enabled) {
            ETH.config(_fixedIP.ip, _fixedIP.gateway, _fixedIP.subnet, _fixedIP.dns1, _fixedIP.dns2);
        }
        
        if (!ETH.begin(ETH_PHY_W5500, _ethAddr, _ethCsPin, _ethIntPin, _ethRstPin,
                     SPI3_HOST, _ethSclkPin, _ethMisoPin, _ethMosiPin)) {
            Serial.println("ETH start Failed!");
            return false;
        } else {
            // Wait for Ethernet to connect
            unsigned long startTime = millis();
            while (!ethConnected && (millis() - startTime < 10000)) {
                Serial.println("Waiting for Ethernet connection...");
                delay(500);
            }
            
            if (ethConnected) {
                _connected = true;
                Serial.println("Connected via Ethernet");
                if (_fixedIP.enabled) {
                    Serial.print("Using fixed IP: ");
                    Serial.println(ETH.localIP());
                }
                return true;
            } else {
                // Ethernet connection failed
                return false;
            }
        }
    }
    
    // If we're here, we're in WiFi mode
    if (_mode == MODE_WIFI) {
        // Try WiFi
        if (_ssid.length() == 0) {
            Serial.println("WiFi SSID not configured!");
            return false;
        }
        
        Serial.println("Connecting to WiFi...");
        Serial.print("SSID: ");
        Serial.println(_ssid);
        
        // Configure fixed IP if enabled (must be done before WiFi.begin)
        if (_fixedIP.enabled) {
            WiFi.config(_fixedIP.ip, _fixedIP.gateway, _fixedIP.subnet, _fixedIP.dns1, _fixedIP.dns2);
        }
        
        WiFi.begin(_ssid.c_str(), _password.c_str());
        
        // Wait for WiFi to connect
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startTime < 20000)) {
            Serial.print(".");
            delay(500);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            _connected = true;
            Serial.println("");
            Serial.println("Connected to WiFi");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            return true;
        } else {
            Serial.println("");
            Serial.println("WiFi connection failed!");
            return false;
        }
    }
    
    return false;
}

void iSprinklrNetwork::WiFiEventCallback(arduino_event_id_t event) {
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex()) {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.print("Mbps");
        Serial.print(", GatewayIP: ");
        Serial.println(ETH.gatewayIP());
        ethConnected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        ethConnected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        ethConnected = false;
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("WiFi Connected. IP address: ");
        Serial.println(WiFi.localIP());
        wifiConnected = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi Disconnected");
        wifiConnected = false;
        break;
    default:
        break;
    }
}

IPAddress iSprinklrNetwork::getIP() {
    if (ethConnected) {
        return ETH.localIP();
    } else if (wifiConnected) {
        return WiFi.localIP();
    } else {
        return IPAddress(0, 0, 0, 0);
    }
}

String iSprinklrNetwork::getNetworkType() {
    if (ethConnected) {
        return "Ethernet";
    } else if (wifiConnected) {
        return "WiFi";
    } else {
        return "Disconnected";
    }
}
