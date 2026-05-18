![logo](images/Freezy%20Arena%20EStops%20logo.png)
# Freezy Estops

Freezy Estops is a robust application for managing emergency stop systems in an arena environment. It provides real-time monitoring and control of emergency stop mechanisms, LED indicators, and network communication to enhance safety and reliability in competitive settings.

## Key Features:
- Ethernet and WebSocket integration for reliable arena communication
- WS2812B LED strip control for visual status indicators
- Monitoring of multiple stop buttons
- Web-based configuration interface
- Persistent settings storage using non-volatile memory

# Table of Contents
- Overview
- Hardware Requirements
- Features
- Preferences (Persistent Settings)
- LED Configuration
- Build and Flash Instructions
- Web UI
- REST API and WebSocket
- Contributing
- License

## Overview

Freezy Estops runs on the ESP32-S3-DevKitM-1 board to manage emergency stop systems in an arena setting. It connects to an arena backend via Ethernet using REST API and WebSocket protocols, monitors stop buttons, controls a WS2812B LED strip with 750 LEDs for status indication, A web interface allows configuration of device roles, network settings, with settings stored persistently using the Preferences library.

The system supports four device roles:
- RED_ALLIANCE: Monitors stop buttons for the red alliance.
- BLUE_ALLIANCE: Monitors stop buttons for the blue alliance.
- FMS_TABLE: Controls field stack lights and start match button.
- BARGE_LIGHTS: Displays match state via LED sections.

##  Hardware Requirements
Supported Board
- ESP32-S3-DevKitM-1
 - ESP32-S3 ETH Development Board with POE
 - Features integrated W5500 Ethernet module for reliable connectivity.

## Pin Configuration
- Stop Buttons (7 buttons, active-low with external pull-ups):
 - Pin 33: Field Stop
 - Pin 1: Emergency Stop 1 (1E)
 - Pin 2: Alliance Stop 1 (1A)
 - Pin 3: Emergency Stop 2 (2E)
 - Pin 15: Alliance Stop 2 (2A)
 - Pin 18: Emergency Stop 3 (3E)
 - Pin 16: Alliance Stop 3 (3A)
 - Start Match Button: Pin 34 (active-low, internal pull-up)
- LED Strip: Pin 47 (WS2812B, 750 LEDs)
- Onboard RGB LED: Pin 21 (not used in current firmware)
- Ethernet Pins (W5500):
 - CS: 14
 - IRQ: 10
 - RST: 9
 - SPI SCK: 13
 - SPI MISO: 12
 - SPI MOSI: 11
## Additional Hardware
- WS2812B LED Strip: 750 LEDs (5 strips × 5 meters × 30 LEDs/meter).
- Ethernet Module: W5500 (integrated on ESP32-S3-DevKitM-1).
- Stop Buttons: 7 push buttons with external pull-up resistors.
- Power Supply: 
 - Barge 5V, ~45A for 750 LEDs at full brightness (reduced to ~3A with optimized brightness).
 - FMS Table 5V ~2A
 - Allinace SCC POE
## Features
- Web-Based Configuration:
 - Configure device role, network settings, via a web interface.
 - Save settings to non-volatile memory.
- Real-Time Status Updates:
  - LED Indicators: Visual feedback for:
    - Device role (red, blue, violet, or match-dependent).
    - Heartbeat status (white: OK, orange: HTTP error, red: network error, black: off).
    - Field stack lights (FMS_TABLE) and match state (BARGE_LIGHTS).
  - Stop Button Monitoring: Tracks 7 stop buttons, sending updates via REST API.
- Ethernet Connectivity:
  - Uses W5500 Ethernet module with DHCP or static IP (default: 192.168.10.220).
  - Connects to arena server at 10.0.100.5:8080.
- REST API Integration:
  - Start match, stop status, alert updates via HTTP POST/GET.
  - Retrieves field stack light status for FMS_TABLE.
- WebSocket Integration:
  - Real-time updates for match state and LED control in BARGE_LIGHTS role.
  - Automatic reconnection on disconnect.
- Persistent Settings:
 - Stores configuration in non-volatile memory, retained across reboots.
## Preferences (Persistent Settings)
Settings are stored using the Preferences library in non-volatile memory. Default values are applied if keys are unset.

Key           Type      Default Value   Description
deviceRole    String    RED_ALLIANCE    Device role (RED_ALLIANCE, BLUE_ALLIANCE, FMS_TABLE, BARGE_LIGHTS)
deviceIP      String    10.0.100.240    Static device IP (if useDHCP=false)
deviceGWIP    String    10.0.100.3      Gateway IP (if useDHCP=false)
useDHCP       Bool      true            Enable DHCP (true) or static IP (false)
arenaIP       String    10.0.100.5      Arena server IP for REST/WebSocket
arenaPort     String    8080            Arena server port

## LED Configuration
The WS2812B LED strip (750 LEDs, pin 47) is controlled using FastLED with optimized settings.
### LED Indices
- Global:
 - HEARTBEAT_LED (0): System status (white: OK, orange: HTTP error, red: network error, black: off).
 - DEVICE_ROLE_LED (1): Role indicator (red: RED_ALLIANCE, blue: BLUE_ALLIANCE, violet: FMS_TABLE).
 - SOCKET_ACTIVITY_LED (2): Reserved for WebSocket activity.
 - RESERVED1–4 (3–6): Reserved.
- FMS_TABLE:
  - RED_LED_INDEX (2, 60 LEDs): Red stack light.
  - BLUE_LED_INDEX (60, 60 LEDs): Blue stack light.
  - ORANGE_LED_INDEX (120, 60 LEDs): Orange stack light.
  - GREEN_LED_INDEX (180, 56 LEDs): Green stack light.
- BARGE_LIGHTS:
  - BLUE1_LED (7, 124 LEDs): Blue alliance, position 1.
  - BLUE2_LED (124, 124 LEDs): Blue alliance, position 2.
  - BLUE3_LED (248, 124 LEDs): Blue alliance, position 3.
  - RED3_LED (372, 124 LEDs): Red alliance, position 3.
  - RED2_LED (496, 124 LEDs): Red alliance, position 2.
  - RED1_LED (620, 124 LEDs): Red alliance, position 1.
  - Match States:
    - Pre-match (LT_MatchState=0): All 750 LEDs green if coilValues[7]=true.
    - Active match (LT_MatchState=1–5): Red/blue LEDs based on coilValues[8–13].
    - Post-match (LT_MatchState=6): Green if coilValues[7]=true, else red/blue per coilValues[8–13].
## Build and Flash Instructions
### Prerequisites
- IDE: Visual Studio Code with PlatformIO extension.
- Board: ESP32-S3-DevKitM-1.
- Dependencies (in platformio.ini):
    lib_deps =
        fastled/FastLED
        bblanchon/ArduinoJson
        links2004/WebSockets
        espressif/Preferences
        espressif/ETH
### Steps
- Clone the Repository:
    git clone <repository-url>
    cd freezy-estops
- Open in VS Code:
  - Open the project folder in VS Code with PlatformIO installed.
- Build:
    pio run -e esp32-s3-devkitm-1
- Flash:
    pio run -e esp32-s3-devkitm-1 -t upload
- Monitor Serial Output:
    pio device monitor --baud 115200
## Web UI
- Access: http://<device-ip>:80 (default: http://10.0.5.220). Root Page (/):
  - Displays stop button states, LED status, (alliance roles).
  - Polls /buttonStates (JSON) every 500 ms.
- Setup Page (/setup):
  - Configure device role, network settings (DHCP, IPs, port).
  - Saves settings to Preferences.
## REST API and WebSocket
### REST API
- Base URL: http://10.0.100.5:8080 (configurable via arenaIP, arenaPort).
- Endpoints:
  - POST /api/freezy/startMatch: Sends {"match":"start"} (FMS_TABLE).
  - POST /api/freezy/eStopState: Updates stop button states (e.g., {"button0":true}).
  - GET /api/freezy/field_stack_light: Retrieves stack light status (e.g., {"redStackLight":true,...}).
### WebSocket
- URL: ws://10.0.100.5:8080/setup/field_testing/websocket.
- Messages:
  - plcIoChange:
    - Registers[3]: Updates LT_MatchState (0: Pre-match, 1–5: Active, 6: Post-match, 7: Timeout, 8: Post-timeout).
    - Coils[0–13]: Updates coilValues for BARGE_LIGHTS LEDs.
- Reconnect: Retries every 5000 ms if disconnected.
## Contributing
We welcome contributions! To contribute:
- Fork the repository.
- Create a feature branch:
  - git checkout -b feature/<feature-name>
- Make changes and commit:
  - git commit -m "Add <feature-name>"
- Push to your branch:
  - git push origin feature/<feature-name>
- Open a pull request.
  - Include clear commit messages and test changes thoroughly.
## License
- This project is licensed under the MIT License. See the LICENSE file for details.
## Troubleshooting
- Enable printSerialDebug to log setLEDColor errors (e.g., out-of-bounds indices).
- Network Issues:
- Check Ethernet cable and W5500 module.
  - Verify arenaIP (10.0.100.5) and arenaPort (8080).
  - For static IP, ensure deviceIP and deviceGWIP are valid.
- Stop Buttons:
  - Verify pins 33, 1, 2, 3, 15, 18, 16 have external pull-ups.
- Test start button (pin 34) in FMS_TABLE mode for startMatchPost.
## Debugging:
- Enable printSerialDebug in Main.cpp:
  - printSerialDebug = true;
- Monitor serial output:
  - pio device monitor --baud 115200