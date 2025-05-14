# iSprinklr_esp

Creates a simple REST API for controlling a Hunter Pro-c sprinkler system using an ESP32 device. The project supports both Ethernet and WiFi connectivity.

This replaces the previous Arduino Zero device that was controlled via serial connection. I was having a number of issues with serial permissions and needing to manually reset after power outages. Using an ESP32 with network connectivity eliminates those issues.

HunterRoam library used from https://github.com/ecodina/hunter-wifi (see HunterRoam.cpp for attributions).

## Overview

iSprinklr uses an ESP32 to control a Hunter Pro-c sprinkler system.

There are 3 components:
- iSprinklr_esp is the esp32 controller which has a very simple rest api for turning the sprinkler system on/off
- iSprinklr_api (https://github.com/mbsperry/isprinklr_api) is an API build with python and FastAPI. It provides a much more powerful API for controlling the system including monitoring which system is active and running user created schedules. 
- iSprinklr_react (https://github.com/mbsperry/iSprinklr_react) is the front end web app built in react. It provides a web interface for the API.

## Hardware layout
Connect GPIO pin 18 to REM port on the Hunter Pro-C.
Connect VSYS to AC#2 on the Hunter Pro-c
Excellent writeup on the hardware connections can be found here: https://www.loullingen.lu/projekte/Hunter/index.php?language=EN
Power for my setup comes from ethernet POE. However you can also power the ESP32 using a usb power supply. 

## Installation and Build Options

### Network Configuration
The project can be built with different network connectivity options:

1. **Ethernet Only** (default)
   ```
   platformio run
   ```
   or
   ```
   platformio run -e ethernet
   ```

2. **WiFi Only**
   ```
   platformio run -e wifi
   ```

When using WiFi mode, you must set your WiFi credentials directly in the platformio.ini file (see below).

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

### WiFi Credentials
To use WiFi connectivity, you need to directly edit the WiFi credentials in the `platformio.ini` file:

1. Find the WiFi environment section in your platformio.ini file:
   ```ini
   [env:wifi]
   platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
   board = waveshare_esp32s3_eth
   build_flags = 
     ${env.build_flags}
     -D NETWORK_MODE=2
     '-D WIFI_SSID="<YOUR_SSID>"'
     '-D WIFI_PASSWORD="<YOUR_PASSWORD>"'
   ```

### Installation Steps
1. Build and install iSprinklr_esp using PlatformIO with your preferred network configuration. The ESP32 will print out it's IP address to the serial monitor when it connects to the network. Make note of this IP. 
2. Git clone iSprinklr_api. Create a virtual environment and install requirements.txt using pip. Create conf/api.conf following the example.conf file. Put the IP of the ESP32 in the conf/api.conf file. Run the API using fastapi run main.py from inside the isprinklr directory.
3. Git clone iSprinklr_react. Update src/config.js. Build and serve via nginx or node serve.
4. If you want to use the scheduling feature you will need to setup a cron job to run the scheduler.py script daily.

Credit:
iSprinklr_esp relies on the HunterRoam library from ecodina (https://github.com/ecodina/hunter-wifi) to actually control the Hunter Pro-c.
