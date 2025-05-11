#ifndef ISPRINKLR_NETWORK_H
#define ISPRINKLR_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <SPI.h>

// Network connection options
enum NetworkMode {
    MODE_ETHERNET,    // Use Ethernet only
    MODE_WIFI         // Use WiFi only
};

// Fixed IP configuration
struct FixedIPConfig {
    bool enabled;
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
};

class iSprinklrNetwork {
private:
    NetworkMode _mode;
    bool _connected;
    String _ssid;
    String _password;
    
    // Ethernet settings
    int _ethCsPin;
    int _ethIntPin;
    int _ethRstPin;
    int _ethMisoPin;
    int _ethMosiPin;
    int _ethSclkPin;
    int _ethAddr;
    
    // Fixed IP configuration
    FixedIPConfig _fixedIP;
    
    static iSprinklrNetwork* _instance;
    
    // Private constructor for singleton
    iSprinklrNetwork();
    
public:
    static iSprinklrNetwork* getInstance();
    
    // Initialize network with the specified mode
    bool begin(NetworkMode mode = MODE_ETHERNET);
    
    // Configure WiFi settings
    void configureWiFi(const char* ssid, const char* password);
    
    // Configure Ethernet settings
    void configureEthernet(int csPin, int intPin, int rstPin, int misoPin, int mosiPin, int sclkPin, int addr);
    
    // Configure Fixed IP settings
    void configureFixedIP(const IPAddress& ip, const IPAddress& gateway, const IPAddress& subnet, 
                          const IPAddress& dns1 = IPAddress(0,0,0,0), const IPAddress& dns2 = IPAddress(0,0,0,0));
    
    // Disable Fixed IP (use DHCP instead)
    void disableFixedIP();
    
    // Check if Fixed IP is enabled
    bool isFixedIPEnabled() { return _fixedIP.enabled; }
    
    // WiFi event handler
    static void WiFiEventCallback(arduino_event_id_t event);
    
    // Get connection status
    bool isConnected() { return _connected; }
    
    // Get current IP address
    IPAddress getIP();
    
    // Get current mode
    NetworkMode getMode() { return _mode; }
    
    // Get current network type (returns "Ethernet" or "WiFi")
    String getNetworkType();
};

#endif // ISPRINKLR_NETWORK_H
