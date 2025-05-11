# iSprinklr_esp

Creates a simple REST API for controlling a Hunter Pro-c sprinkler system using an Esp32 device. The project supports both Ethernet and WiFi connectivity, with options to use either one exclusively or automatically select the best available connection.

This replaces the previous Arduino Uno device that was controlled via serial connect. I was having a number of issues with serial permissions and needing to be manually reset after power outages. Using an ESP32 with network connectivity eliminates those issues.

HunterRoam library used from https://github.com/ecodina/hunter-wifi (see HunterRoam.cpp for attributions).

## Overview

iSprinklr uses an arduino to control a Hunter Pro-c sprinkler system.

There are 3 components:
- iSprinklr_esp is the esp32 controller which has a very simple rest api for turning the sprinkler system on/off
- iSprinklr_api (https://github.com/mbsperry/isprinklr_api) is an API build with python and FastAPI. It provides a much more powerful API for controlling the system including monitoring which system is active and running user created schedules. 
- iSprinklr_react (https://github.com/mbsperry/iSprinklr_react) is the front end web app built in react. It provides a web interface for the API.

## Hardware layout
Connect LED_BUILTIN pin (pin 13) to REM pin on the Hunter Pro-C. Excellent overview on how to connect the arduino to the Pro-c here: https://github.com/ecodina/hunter-wifi/blob/master/docs/pages/hunterconnection.md
Connect arduino to a raspberry pi (or similar) via USB on the programming port.

## Installation and Build Options

### Network Configuration
The project can be built with different network connectivity options:

1. **Ethernet Only** (default): Use when you have a wired Ethernet connection available
   ```
   platformio run
   ```
   or
   ```
   platformio run -e ethernet
   ```

2. **WiFi Only**: Use when you only have WiFi connectivity
   ```
   platformio run -e wifi
   ```

When using WiFi mode, you must set environment variables for your WiFi credentials (see below).

### Fixed IP Configuration
You can configure the device to use a static IP address by uncommenting and modifying the fixed IP settings in the `platformio.ini` file:

```ini
[env]
; Uncomment below lines to enable fixed IP for both environments
; build_flags =
;   -D FIXED_IP=\"192.168.88.7\"
;   -D FIXED_GATEWAY=\"192.168.88.1\"
;   -D FIXED_SUBNET=\"255.255.255.0\"
;   -D FIXED_DNS1=\"1.1.1.1\"
;   -D FIXED_DNS2=\"8.8.8.8\"
```

Simply remove the semicolons to uncomment the lines and adjust the IP addresses as needed for your network. The fixed IP settings will be applied to both Ethernet and WiFi connections.

### WiFi Credentials
To keep your WiFi credentials secure and out of your Git repository, the project now uses environment variables to set your WiFi credentials. You can set these in several ways:

1. **Set environment variables before building**:
   ```bash
   # Linux/macOS
   export WIFI_SSID="YourSSID"
   export WIFI_PASSWORD="YourPassword"
   platformio run -e wifi

   # Windows (CMD)
   set WIFI_SSID=YourSSID
   set WIFI_PASSWORD=YourPassword
   platformio run -e wifi
   
   # Windows (PowerShell)
   $env:WIFI_SSID="YourSSID"
   $env:WIFI_PASSWORD="YourPassword"
   platformio run -e wifi
   ```

2. **Create a local environment file**:
   Create a file named `.env` in your project root (which should be added to `.gitignore`):
   ```
   WIFI_SSID=YourSSID
   WIFI_PASSWORD=YourPassword
   ```
   
   Then source this file before building:
   ```bash
   # Linux/macOS
   source .env
   platformio run
   ```

3. **For VSCode users**:
   You can set these in your VSCode settings by adding to `.vscode/settings.json`:
   ```json
   {
     "terminal.integrated.env.linux": {
       "WIFI_SSID": "YourSSID",
       "WIFI_PASSWORD": "YourPassword"
     },
     "terminal.integrated.env.osx": {
       "WIFI_SSID": "YourSSID",
       "WIFI_PASSWORD": "YourPassword"
     },
     "terminal.integrated.env.windows": {
       "WIFI_SSID": "YourSSID",
       "WIFI_PASSWORD": "YourPassword"
     }
   }
   ```

### Installation Steps
1. Build and install iSprinklr_esp using PlatformIO with your preferred network configuration
2. Git clone iSprinklr_api. Create a virtual environment and install requirements.txt using pip. Run the API using uvicorn.
3. Git clone iSprinklr_react. Build and serve via nginx or node serve.
4. If you want to use the scheduling feature you will need to setup a cron job to run the scheduler.py script daily.
5. Adjust data/sprinklers.csv to match your existing sprinkler zones and data/schedule.csv to your watering schedule.

Credit:
iSprinklr_esp relies on the HunterRoam library from ecodina (https://github.com/ecodina/hunter-wifi) to actually control the Hunter Pro-c.
