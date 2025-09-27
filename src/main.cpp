/*  ______                              ___
   / ____/_______  ___  ____  __  __   /   |  ________  ____  ____ _
  / /_  / ___/ _ \/ _ \/___ / / / /  / /| | / ___/ _ \/ __ \/ __ `/
 / __/ / /  /  __/  __/ / /_/ /_/ /  / ___ |/ /  /  __/ / / / /_/ /
/_/ __/_/___\___/\___/ /___/\__, /  /_/  |_/_/   \___/_/ /_/\__,_/
   / ____/ ___// /_____  __/____/___
  / __/  \__ \/ __/ __ \/ __ \/ ___/
 / /___ ___/ / /_/ /_/ / /_/ (__  )
/_____//____/\__/\____/ .___/____/
                     /_/
*/

#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include "WiFiCredentials.h"
#include <ArduinoJson.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include "StartMatch.h"
#include "postStopStatus.h"
#include "postElementIncrement.h"
#include "Field_stack_lightStatus.h"
#include "WebServerSetup.h"
#include "GlobalSettings.h"
#include <WebSocketsClient.h>
#include "SonarSensor.h"

// Constants and Pin Definitions
#define USE_SERIAL Serial
const char *baseUrl = "http://192.168.10.124:8080";
const uint8_t NUM_BUTTONS = 7;

// Sonar configuration
const uint8_t TRIG_PIN = 21;
const uint8_t ECHO_PIN = 17;
const unsigned long PULSE_TIMEOUT = 30000UL; // 30 ms -> ~5 meters
SonarSensor sonar(TRIG_PIN, ECHO_PIN, PULSE_TIMEOUT);

// Alert configuration
unsigned long alertTrigCm = 30UL;   // Trigger when object closer than 30 cm
unsigned long alertHoldMs = 1000UL; // Must stay below threshold for 1 second
unsigned long minOffMs = 1000UL;    // Once triggered, require 10 seconds of cleared before allowing retrigger
unsigned long loopTimeThresholdMs = 200; // 200ms threshold for loop timing warning
int looptime = 100; // Loop delay in ms

// Ethernet/WiFi configuration
#ifdef ESP32_S3_DEVKITM_1
const int stopButtonPins[NUM_BUTTONS] = {33, 1, 2, 3, 15, 18, 16}; // Field stop, 1E, 1A, 2E, 2A, 3E, 3A
#define START_MATCH_BTN 34
#define LEDSTRIP 47
#define NUM_LED_STRIPS 5
#define NUM_LEDS_PER_STRIP 5
#define NUM_LEDS_PER_M 30
#define NUM_LEDS (NUM_LED_STRIPS * NUM_LEDS_PER_STRIP * NUM_LEDS_PER_M)
#define SECTION_LENGTH 124
#define BLUE1_LED 0
#define BLUE1_LED_LENGTH SECTION_LENGTH
#define BLUE2_LED SECTION_LENGTH
#define BLUE2_LED_LENGTH SECTION_LENGTH
#define BLUE3_LED SECTION_LENGTH * 2
#define BLUE3_LED_LENGTH SECTION_LENGTH
#define RED1_LED SECTION_LENGTH * 5
#define RED1_LED_LENGTH SECTION_LENGTH
#define RED2_LED SECTION_LENGTH * 4
#define RED2_LED_LENGTH SECTION_LENGTH
#define RED3_LED SECTION_LENGTH * 3
#define RED3_LED_LENGTH SECTION_LENGTH
#define HEARTBEAT_LED NUM_LEDS - 1
#define SOCKET_ACTIVITY_LED NUM_LEDS - 2
#define RESERVED1 NUM_LEDS - 3
#define RESERVED2 NUM_LEDS - 4
#define RESERVED3 NUM_LEDS - 5
#define RESERVED4 NUM_LEDS - 6
int g_Brightness = 255;
int g_PowerLimit = 50000;
#else // ESP32DEV
const int stopButtonPins[NUM_BUTTONS] = {21, 22, 23, 25, 26, 27, 32};
#define START_MATCH_BTN 19
#define LEDSTRIP 4
#define ONBOARD_LED 2
#endif

// Ethernet PHY configuration
#ifndef ETH_PHY_CS
#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 1
#define ETH_PHY_CS 14
#define ETH_PHY_IRQ 10
#define ETH_PHY_RST 9
#define ETH_PHY_SPI_HOST SPI2_HOST
#define ETH_PHY_SPI_SCK 13
#define ETH_PHY_SPI_MISO 12
#define ETH_PHY_SPI_MOSI 11
#endif

// Global Variables
extern String deviceRole;
extern String deviceIP;
extern String deviceGWIP;
extern bool useDHCP;
bool stopButtonStates[NUM_BUTTONS - 1];
bool startButtonState = false;
bool sonarAlertSent = false;
float sonarDistance;
CRGB g_LEDs[NUM_LEDS] = {0};
WebSocketsClient webSocket;
int LT_MatchState = 0;
bool socketDataActivity = false;
bool eth_connected = false;
int heartbeatState = 0;
bool coilValues[14];
JsonArray coils;
String socketData;
bool printSerialDebug = false;

// Utility Functions
void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t *src = (const uint8_t *)mem;
    USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        USE_SERIAL.printf("%02X ", *src);
        src++;
    }
    USE_SERIAL.printf("\n");
}

void setLED_Color(int ledIndex, int length, bool status, CRGB color) {
    for (int i = ledIndex; i < ledIndex + length; i++) {
        g_LEDs[i] = status ? color : CRGB::Black;
    }
}

// WebSocket Parser
void parser(String s) {
    socketDataActivity = !socketDataActivity;
    DynamicJsonDocument doc(6145);
    deserializeJson(doc, s);
    JsonObject json = doc.as<JsonObject>();
    const char *type = doc["type"];
    JsonObject data = doc["data"];
    if (strcmp(type, "plcIoChange") == 0) {
        JsonArray registers = data["Registers"];
        LT_MatchState = registers[3];
        coils = data["Coils"];
        USE_SERIAL.print("Coils: ");
        int index = 0;
        for (int i : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}) {
            if (i < coils.size()) {
                bool coilValue = coils[i].as<bool>();
                coilValues[index++] = coilValue;
                USE_SERIAL.print(coilValue);
                USE_SERIAL.print(" ");
            } else {
                USE_SERIAL.print("N/A ");
            }
        }
        USE_SERIAL.println();
    }
}

// WebSocket Event Handler
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            socketData = (char *)payload;
            parser(socketData);
            break;
        case WStype_BIN:
            hexdump(payload, length);
            break;
        case WStype_ERROR:
            USE_SERIAL.println("[************WSc ERROR***********]");
            break;
        case WStype_FRAGMENT_TEXT_START:
            socketData = (char *)payload;
            break;
        case WStype_FRAGMENT:
            socketData += (char *)payload;
            break;
        case WStype_FRAGMENT_FIN:
            socketData += (char *)payload;
            parser(socketData);
            break;
        case WStype_PONG:
            break;
    }
}

// Network Event Handler
void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi STA Started");
            WiFi.setHostname("Freezy_Red");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("WiFi STA Got IP: '%s'\n", WiFi.localIP().toString().c_str());
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("ETH Started");
            ETH.setHostname("Freezy_ScoreTable");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("ETH Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
            Serial.println(ETH);
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_LOST_IP:
            Serial.println("ETH Lost IP");
            eth_connected = false;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("ETH Disconnected");
            eth_connected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("ETH Stopped");
            eth_connected = false;
            break;
        default:
            break;
    }
}

// WiFi Initialization
void initWiFi() {
    WiFi.onEvent(onEvent);
    WiFi.mode(WIFI_STA);
    IPAddress local_ip(192, 168, 10, 220);
    IPAddress local_gateway(192, 168, 10, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8);
    IPAddress secondaryDNS(8, 8, 4, 4);
    WiFi.config(local_ip, local_gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.begin(ssid, password);
    USE_SERIAL.print("Connecting to WiFi .. ");
    while (WiFi.status() != WL_CONNECTED) {
        USE_SERIAL.print('.');
        delay(1000);
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    delay(3000);
}

// Handle Stop Buttons
void handleStopButtons() {
    for (int i = 0; i < NUM_BUTTONS - 1; i++) {
        stopButtonStates[i] = !digitalRead(stopButtonPins[i + 1]);
    }
}

// Handle Sonar Alerts
void handleSonarAlerts() {
    sonarDistance = sonar.getLastDistance();
    if (sonar.belowThresholdFor(alertTrigCm)) {
        if (!sonarAlertSent) {
            Serial.println("ALERT: object within threshold for >=1s (one-shot)");
            sonarAlertSent = true;
            if (LT_MatchState >= 1 && LT_MatchState <= 6) {
                postElement("red", "ProcessorAlgae");
            }
        }
    } else {
        sonarAlertSent = false;
    }
}

// Setup Function
void setup() {
    Serial.begin(115200);
    delay(5000);

    // Initialize preferences
    preferences.begin("settings", false);
    deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
    deviceIP = preferences.getString("deviceIP", "10.0.100.240");
    deviceGWIP = preferences.getString("deviceGWIP", "10.0.100.3");
    useDHCP = preferences.getBool("useDHCP", true);
    alertTrigCm = preferences.getULong("alertTrigCm", 30);
    alertHoldMs = preferences.getULong("alertHoldMs", 1000);
    minOffMs = preferences.getULong("minOffMs", 1000);
    preferences.end();

    if (deviceRole != "BARGE_LIGHTS") {
        g_Brightness = 15;
        g_PowerLimit = 900;
    }

    // Initialize LED strip
    FastLED.addLeds<WS2812B, LEDSTRIP, GRB>(g_LEDs, NUM_LEDS);
    FastLED.setBrightness(g_Brightness);
    FastLED.setMaxPowerInMilliWatts(g_PowerLimit);

    // Initialize buttons
    pinMode(START_MATCH_BTN, INPUT_PULLUP);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(stopButtonPins[i], INPUT);
    }

    // Initialize sonar
    sonar.begin();
    sonar.setMinOffTime(minOffMs);
    sonar.startBackground(200);

#ifdef ESP32DEV
    initWiFi();
    pinMode(ONBOARD_LED, OUTPUT);
#endif
#ifdef ESP32_S3_DEVKITM_1
    Network.onEvent(onEvent);
    if (useDHCP) {
        ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
    } else {
        IPAddress localIP, localGW;
        if (localIP.fromString(deviceIP) && localGW.fromString(deviceGWIP)) {
            Serial.println("Setting static IP address.");
            ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
            ETH.config(localIP, localGW, IPAddress(255, 255, 255, 0), IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
        } else {
            Serial.println("Invalid static IP address. Falling back to DHCP.");
            ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
        }
    }
    while (!eth_connected) {
        Serial.print(".");
        delay(500);
    }
    Serial.print("init - IP Address: ");
    Serial.println(ETH.localIP());
#endif

    setupWebServer();
    webSocket.setExtraHeaders("Origin: http://192.168.10.124:8080");
    webSocket.begin("192.168.10.124", 8080, "ws://192.168.10.124:8080/setup/field_testing/websocket");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);

    Serial.println("Setup complete.");
    Serial.println("Free heap: " + String(ESP.getFreeHeap()));
}

// Main Loop
void loop() {
    static unsigned long lastLoopTime = 0;
    unsigned long startTime = millis();
    static unsigned long lastStatusCheck = 0;
    static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();
    deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");

    webSocket.loop();

    if (deviceRole == "RED_ALLIANCE") {
        setLED_Color(1, 1, true, RED_COLOR);
        handleStopButtons();
        postAllStopStatus(stopButtonStates, 1);
        handleSonarAlerts();
    } else if (deviceRole == "BLUE_ALLIANCE") {
        setLED_Color(1, 1, true, BLUE_COLOR);
        handleStopButtons();
        postAllStopStatus(stopButtonStates, 7);
        handleSonarAlerts();
    } else if (deviceRole == "FMS_TABLE") {
        setLED_Color(1, 1, true, VIOLET_COLOR);
        startButtonState = !digitalRead(START_MATCH_BTN);
        if (startButtonState) {
            Serial.println("Start match button pressed!");
            startMatchPost();
        }
        stopButtonStates[0] = !digitalRead(stopButtonPins[0]);
        postSingleStopStatus(0, stopButtonStates[0]);
        if (currentMillis - lastStatusCheck >= 500) {
            getField_stack_lightStatus();
            lastStatusCheck = currentMillis;
        }
    } else if (deviceRole == "BARGE_LIGHTS") {
        looptime = 0;
        switch (LT_MatchState) {
            case 0: // Pre Match
                setLED_Color(0, NUM_LEDS, coilValues[7], GREEN_COLOR);
                break;
            case 1 ... 5: // Match
                setLED_Color(RED1_LED, RED1_LED_LENGTH, coilValues[8], RED_COLOR);
                setLED_Color(RED2_LED, RED2_LED_LENGTH, coilValues[9], RED_COLOR);
                setLED_Color(RED3_LED, RED3_LED_LENGTH, coilValues[10], RED_COLOR);
                setLED_Color(BLUE1_LED, BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
                setLED_Color(BLUE2_LED, BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
                setLED_Color(BLUE3_LED, BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
                break;
            case 6: // Post Match
                if (coilValues[7]) {
                    setLED_Color(0, NUM_LEDS, true, GREEN_COLOR);
                } else {
                    setLED_Color(RED1_LED, RED1_LED_LENGTH, coilValues[8], RED_COLOR);
                    setLED_Color(RED2_LED, RED2_LED_LENGTH, coilValues[9], RED_COLOR);
                    setLED_Color(RED3_LED, RED3_LED_LENGTH, coilValues[10], RED_COLOR);
                    setLED_Color(BLUE1_LED, BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
                    setLED_Color(BLUE2_LED, BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
                    setLED_Color(BLUE3_LED, BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
                }
                break;
            case 7: // TimeoutActive
            case 8: // PostTimeout
                break;
            default:
                break;
        }
    }

    if (currentMillis - lastPrint >= 5000) {
        lastPrint = currentMillis;
        deviceIP = preferences.getString("deviceIP", "");
        Serial.printf("Preferences IP Address: %s\n", deviceIP.c_str());
        deviceGWIP = preferences.getString("deviceGWIP", "");
        Serial.printf("Preferences Gateway IP Address: %s\n", deviceGWIP.c_str());
        useDHCP = preferences.getBool("useDHCP", true);
        Serial.printf("Preferences useDHCP: %s\n", useDHCP ? "true" : "false");
#ifdef ESP32DEV
        Serial.printf("Current WiFi IP Address: %s\n", WiFi.localIP().toString().c_str());
        digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
#endif
#ifdef ESP32_S3_DEVKITM_1
        Serial.printf("Current Wired IP Address: %s\n", ETH.localIP().toString().c_str());
#endif
    }

    switch (heartbeatState) {
        case 0:
            g_LEDs[HEARTBEAT_LED] = CRGB::Black;
            break;
        case 1:
            g_LEDs[HEARTBEAT_LED] = CRGB::White;
            break;
        case 2:
            g_LEDs[HEARTBEAT_LED] = CRGB::Orange;
            break;
        default:
            g_LEDs[HEARTBEAT_LED] = CRGB::Red;
            break;
    }

    FastLED.show(g_Brightness);

    yield();
    delay(looptime);

    // Loop time warning
    unsigned long loopTime = millis() - startTime;
    if (loopTime > loopTimeThresholdMs) {
        Serial.printf("WARNING: Loop time exceeded threshold! Loop time: %lu ms, Threshold: %lu ms\n", 
                      loopTime, loopTimeThresholdMs);
    }
}