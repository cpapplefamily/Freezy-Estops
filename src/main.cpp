/*  ______                              ___
   / ____/_______  ___  ____  __  __   /   |  ________  ____  ____ _
  / /_  / ___/ _ \/ _ \/_  / / / / /  / /| | / ___/ _ \/ __ \/ __ `/
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
#include "WiFiCredentials.h" // Include the WiFi credentials
#include <ArduinoJson.h>
#define FASTLED_INTERNAL // Suppress build banner
#include <FastLED.h>
#include "StartMatch.h"              // Include the StartMatch header
#include "postStopStatus.h"          // Include the postStopStatus header
#include "postElementIncrement.h"    // Include the postElementIncrement header
#include "Field_stack_lightStatus.h" // Include the Field_stack_lightStatus header
#include "WebServerSetup.h"          // Include the WebServerSetup header
#include "GlobalSettings.h"          // Include the GlobalSettings header
#include <WebSocketsClient.h>
#include "SonarSensor.h"

// Constants
// Sonar Sensor Configuration
const uint8_t TRIG_PIN = 21;
const uint8_t ECHO_PIN = 17;
const unsigned long PULSE_TIMEOUT = 30000UL; // 30 ms -> ~5 meters
unsigned long alertTrigCm = 30UL;   // trigger when object closer than 30 cm
unsigned long alertHoldMs = 1000UL; // must stay below threshold for 1 second
unsigned long minOffMs = 1000UL;    // once triggered, require 10 seconds of cleared before allowing retrigger
unsigned long loopTimeThresholdMs = 200; // 200ms threshold for loop timing warning
int loopWarning = 0; // Show warning for 2 blinks

// Stop Button Configuration
const uint8_t NUM_BUTTONS = 7;

// Ethernet Configuration
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

// Network Configuration
#define USE_SERIAL Serial
const char *baseUrl = "http://10.0.100.5:8080";
const IPAddress LOCAL_IP(192, 168, 10, 220);
const IPAddress LOCAL_GATEWAY(192, 168, 10, 1);
const IPAddress SUBNET(255, 255, 255, 0);
const IPAddress PRIMARY_DNS(8, 8, 8, 8);
const IPAddress SECONDARY_DNS(8, 8, 4, 4);
const char *FIELD_TESTING_ENDPOINT = "/setup/field_testing/websocket";

// Platform-Specific Configurations
// C:\Users\Capplegate\.platformio\penv\Scripts\platformio.exe  run -e esp32-s3-devkitm-1 -t upload
#ifdef ESP32_S3_DEVKITM_1
#define ONBOARD_RGB 21
// Stop Button Pins
const int stopButtonPins[NUM_BUTTONS] = {33,  // Field stop
                                         1,   // 1E stop
                                         2,   // 1A stop
                                         3,   // 2E stop
                                         15,  // 2A stop
                                         18,  // 3E stop
                                         16}; // 3A stop

#define START_MATCH_BTN                 34
#define LEDSTRIP                        47 // Pin connected to NeoPixel
#define NUM_LED_SRIPS                   5
#define NUM_LEDS_SRIPS_L                5
#define NUM_LEDS_PER_M                  30
#define NUM_LEDS                        (NUM_LED_SRIPS * NUM_LEDS_SRIPS_L * NUM_LEDS_PER_M)
#define SECTION_LENGTH                  124
#define BARGE_RED1_LED_START            3
#define BARGE_RED1_LED_LENGTH           SECTION_LENGTH
#define BARGE_RED2_LED_START            SECTION_LENGTH
#define BARGE_RED2_LED_LENGTH           SECTION_LENGTH
#define BARGE_RED3_LED_START            SECTION_LENGTH * 2
#define BARGE_RED3_LED_LENGTH           SECTION_LENGTH
#define BARGE_BLUE3_LED_START           SECTION_LENGTH * 3
#define BARGE_BLUE3_LED_LENGTH          SECTION_LENGTH
#define BARGE_BLUE2_LED_START           SECTION_LENGTH * 4
#define BARGE_BLUE2_LED_LENGTH          SECTION_LENGTH
#define BARGE_BLUE1_LED_START           SECTION_LENGTH * 5
#define BARGE_BLUE1_LED_LENGTH          SECTION_LENGTH
#define HEARTBEAT_LED                   0
#define DEVICE_ROLE_LED                 1
#define SOCKET_ACTIVITY_LED             2
#define RESERVED1                       3
#define RESERVED2                       4
#define RESERVED3                       5
#define RESERVED4                       6
#endif // ESP32_S3_DEVKITM_1

// Global Variables
extern String       deviceRole;
extern String       deviceIP;
extern String       deviceGWIP;
extern bool         useDHCP;
bool                eth_connected = false;                     // Network connection status
int                 heartbeatState = 0;                        // Heartbeat state for system status
bool                printSerialDebug = false;                  // Debug logging flag
bool                stopButtonStates[NUM_BUTTONS - 1];         // States for stop buttons (excluding field stop)
bool                startButtonState = false;                  // State for start match button
int                 LT_MatchState = 0;                         // Match state from WebSocket
int                 looptime = 100;

String              arenaIP = "10.0.100.5";                     // Arena server IP
String              arenaPort = "8080";                         // Arena server port
String              socketData;

CRGB                g_LEDs[NUM_LEDS] = {0};                     // LED frame buffer
int                 g_Brightness = 255;                         // LED brightness (0-255)
int                 g_PowerLimit = 50000;                       // Power limit in mW

SonarSensor         sonar(TRIG_PIN, ECHO_PIN, PULSE_TIMEOUT);   // Sonar sensor
bool                sonarAlertSent = false;                     // One-shot flag for sonar alerts
float               sonarDistance;                              // Last sonar distance

WebSocketsClient    webSocket;
bool                socketDataActivity = false;                 // WebSocket activity indicator
JsonArray           coils;                                      // JSON array for coils
bool                coilValues[14];                             // Coil states from WebSocket


// Helper Functions
/**
 * Prints a memory hexdump for debugging binary data.
 * @param mem Pointer to the memory to dump.
 * @param len Length of the memory to dump.
 * @param cols Number of columns per line (default 16).
 */
static void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
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

/**
 * Parses WebSocket JSON data and updates match state and coil values.
 * @param s The JSON string to parse.
 */
static void parser(String s) {
  DynamicJsonDocument doc(6145);
  deserializeJson(doc, s);
  JsonObject json = doc.as<JsonObject>();

  const char *type = doc["type"]; // "arenaStatus, matchTime, ..."

  JsonObject data = doc["data"]; // Most Jason Files have a "data" section

  if (strcmp(type, "plcIoChange") == 0)
  {

    // set the match state
    JsonArray registers = data["Registers"];
    LT_MatchState = registers[3];

    // Print the Coils array
    coils = data["Coils"];
    USE_SERIAL.print("Coils: ");
    int index = 0;
    for (int i : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13})
    {
      if (i < coils.size())
      {
        bool coilValue = coils[i].as<bool>();
        coilValues[index++] = coilValue; // Copy the value to the global arra
        USE_SERIAL.print(coils[i].as<bool>());
        USE_SERIAL.print(" ");
      }
      else
      {
        USE_SERIAL.print("N/A ");
      }
    }
    USE_SERIAL.println();
  }
}

/**
 * Handles WebSocket events (connect, disconnect, text, binary, etc.).
 * @param type The WebSocket event type.
 * @param payload The event payload.
 * @param length The payload length.
 */
static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  // USE_SERIAL.printf("type: %x\n", type);

  switch (type)
  {
  case WStype_DISCONNECTED:
    USE_SERIAL.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
    USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
    break;
  case WStype_TEXT:
    USE_SERIAL.printf("[WSc] get text: %s\n", payload);
    socketDataActivity = !socketDataActivity;
    socketData = (char *)payload;
    parser(socketData);
    break;
  case WStype_BIN:
    // USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
    hexdump(payload, length);
    break;
  case WStype_ERROR:
    USE_SERIAL.println("[************WSc ERROR***********]");
  case WStype_FRAGMENT_TEXT_START:
    socketData = (char *)payload;
    // USE_SERIAL.printf("[WSc] Fragment Text Start: %s\n", payload);
    // USE_SERIAL.println(socketData);
    // USE_SERIAL.println("WStype_FRAGMENT_TEXT_START");
    break;
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
    socketData += (char *)payload;
    break;
  case WStype_FRAGMENT_FIN:
    socketData += (char *)payload;
    parser(socketData);
    // USE_SERIAL.println(socketData);
    break;
  case WStype_PONG:
    // USE_SERIAL.printf("[WSc] WStype_PONG: Ping reply\n");
    break;
  }
}

/**
 * Handles network events (WiFi/Ethernet connect, disconnect, IP assignment).
 * @param event The Arduino event ID.
 * @param info Event information.
 */
static void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event)
  {
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
    // set eth hostname here
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


void setLED_Color(int ledIndex1, int length, bool status, CRGB color) {
  if (ledIndex1 + length > NUM_LEDS) {
    length = NUM_LEDS - ledIndex1; // Adjust length to fit within bounds
  }
  if (status) {
    for (int i = ledIndex1; i < ledIndex1 + length; i++) {
      g_LEDs[i] = color; // Set LED to color
    }
  }else{
    for (int i = ledIndex1; i < ledIndex1 + length; i++) {
      g_LEDs[i] = CRGB::Black; // Turn off the LED
    }
  }
}

/**
 * Initializes Ethernet with DHCP or static IP.
 */
static void initEthernet() {
  Network.onEvent(onEvent);
  if (useDHCP) {
    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
  } else {
    IPAddress localIP;
    IPAddress localGW;
    if (localIP.fromString(deviceIP) && localGW.fromString(deviceGWIP)) {
      if (printSerialDebug) {
        Serial.println("Setting static IP address.");
      }
      ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
      ETH.config(localIP, localGW, SUBNET, PRIMARY_DNS, SECONDARY_DNS);
    } else {
      Serial.println("Invalid static IP address. Falling back to DHCP.");
      ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
    }
  }
  while (!eth_connected) {
    if (printSerialDebug) {
      Serial.print(".");
    }
    delay(500);
  }
  if (printSerialDebug) {
    Serial.print("init - IP Address: ");
    Serial.println(ETH.localIP());
  }
}

/**
 * Reads stop button states for RED_ALLIANCE or BLUE_ALLIANCE.
 * @param startingIndex The starting pin index (1 for RED_ALLIANCE, BLUE_ALLIANCE).
 */
static void readStopButtonStates(int startingIndex) {
  for (int i = 0; i < NUM_BUTTONS - 1; i++) {
    stopButtonStates[i] = !digitalRead(stopButtonPins[i + startingIndex]);
  }
}

/**
 * Prints network and preferences information every 5 seconds.
 * @param currentMillis Current time in milliseconds.
 * @param lastPrint Last time information was printed.
 * @return Updated lastPrint time.
 */
static unsigned long printNetworkInfo(unsigned long currentMillis, unsigned long lastPrint, unsigned long interval) {
  if (currentMillis - lastPrint >= interval) {
    if (printSerialDebug) {
      deviceIP = preferences.getString("deviceIP", "");
      deviceGWIP = preferences.getString("deviceGWIP", "");
      useDHCP = preferences.getBool("useDHCP");
      Serial.printf("Preferences IP Address: %s\n", deviceIP.c_str());
      Serial.printf("Preferences Gateway IP Address: %s\n", deviceGWIP.c_str());
      Serial.printf("Preferences useDHCP: %s\n", useDHCP ? "true" : "false");
#ifdef ESP32_S3_DEVKITM_1
      Serial.printf("Current Wired IP Address: %s\n", ETH.localIP().toString().c_str());
#endif
    }
    return currentMillis;
  }
  return lastPrint;
}

/**
 * Updates the heartbeat LED based on heartbeatState.
 */
static unsigned long updateHeartbeatLED(unsigned long currentMillis, unsigned long lastHeartBeat, unsigned long interval) {
    static bool heartbeatled_show = false;
    if (currentMillis - lastHeartBeat >= interval) {
      heartbeatled_show = !heartbeatled_show;
        if (loopWarning > 0){
          USE_SERIAL.println(loopWarning);
          loopWarning = loopWarning - 1;
        }
      if (heartbeatled_show) {
            switch (heartbeatState) {
                case 0:
                    g_LEDs[HEARTBEAT_LED] = CRGB::Black;
                    break;
                case 1:
                    g_LEDs[HEARTBEAT_LED] = (loopWarning == 0) ? LTWHITE_COLOR : RED_COLOR;
                    break;
                case 2:
                    g_LEDs[HEARTBEAT_LED] = CRGB::Orange;
                    break;
                default:
                    g_LEDs[HEARTBEAT_LED] = CRGB::Red;
                    break;
            }
            heartbeatState = 0;
        } else {
            g_LEDs[HEARTBEAT_LED] = CRGB::Black;
        }
        
        return currentMillis;
  }
  return lastHeartBeat;
}

/**
 * Handles sonar alerts for RED_ALLIANCE or BLUE_ALLIANCE.
 * @param alliance The alliance identifier ("red" or "blue").
 */
static void handleSonarAlerts(const String &alliance) {
  sonarDistance = sonar.getLastDistance();
  if (sonar.belowThresholdFor(alertTrigCm)) {
    if (!sonarAlertSent) {
      if (printSerialDebug) {
        Serial.println("ALERT: object within threshold for >=1s (one-shot)");
      }
      sonarAlertSent = true;
      switch (LT_MatchState) {
        case 0: // Pre Match
        case 7: // TimeoutActive
        case 8: // PostTimeout
          break;
        case 1 ... 6:
          postElement(alliance, "ProcessorAlgae");
          break;
        default:
          break;
      }
    }
  } else {
    sonarAlertSent = false;
  }
}

// Setup function
void setup()
{
  USE_SERIAL.begin(115200);
  delay(5000);

  // Initialize preferences
  preferences.begin("settings", false);
  deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
  deviceIP = preferences.getString("deviceIP", "10.0.100.240");
  deviceGWIP = preferences.getString("deviceGWIP", "10.0.100.3");
  useDHCP = preferences.getBool("useDHCP", true);
  alertTrigCm = preferences.getULong("alertTrigCm", 30);   // Default: 30 cm
  alertHoldMs = preferences.getULong("alertHoldMs", 1000); // Default: 1000 ms
  minOffMs = preferences.getULong("minOffMs", 1000);       // Default: 500 ms
  arenaIP = preferences.getString("arenaIP", "10.0.100.5");
  arenaPort = preferences.getString("arenaPort", "8080");

  // Initialize LED strip
  if (deviceRole != "BARGE_LIGHTS") {
    g_Brightness = 15;
    g_PowerLimit = 900;
  }
  FastLED.addLeds<WS2812B, LEDSTRIP, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setBrightness(g_Brightness);
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit); // Set the power limit, above which brightness will be throttled

  // Initialize the start match button
  pinMode(START_MATCH_BTN, INPUT_PULLUP);

  // Initialize the stop buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(stopButtonPins[i], INPUT);
  }

  // Initialize sonar
  /*
  Need to test why sonar is causing issues on FMS_TABLE
  1. If the sonar is initialized, the FMS_TABLE Estops and Start Match button do not work
  */
  if (deviceRole == "RED_ALLIANCE" || deviceRole == "BLUE_ALLIANCE") {
    Serial.println("Initializing Sonar Sensor...");
    sonar.begin();
    sonar.setMinOffTime(minOffMs);
    // start background sampling (non-blocking for the main loop)
    sonar.startBackground(200); 
  }


  // Initialize network
#ifdef ESP32_S3_DEVKITM_1
  initEthernet();
#endif // ESP32

  // Set up the web server
  setupWebServer();

  // Connect to the WebSocket server
  USE_SERIAL.println("Connecting to WebSocket server...");
  String originHeader = "Origin: http://" + arenaIP + ":" + arenaPort;
  webSocket.setExtraHeaders(originHeader.c_str());
  uint16_t port = (uint16_t) strtoul(arenaPort.c_str(), NULL, 10);
  String wsUrl = "ws://" + arenaIP + ":" + arenaPort + FIELD_TESTING_ENDPOINT;
  webSocket.begin(arenaIP, port, wsUrl.c_str());

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}



// Main loop
void loop() {
  static unsigned long lastStatusCheck = 0;
  unsigned long startTime = millis();
  static unsigned long lastPrint = 0;
  static unsigned long lastHeartbeat = 0;
  unsigned long currentMillis = millis();
  deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
  // FastLED.clear(); // Clear the LED strip
  looptime = 100;

  webSocket.loop(); // Handle WebSocket events

  if (deviceRole == "RED_ALLIANCE") {
    setLEDColor(DEVICE_ROLE_LED, 1, true, RED_COLOR); // RED
    readStopButtonStates(1);
    postAllStopStatus(stopButtonStates,1);
    handleSonarAlerts("red");
  }
  else if (deviceRole == "BLUE_ALLIANCE") {
    setLEDColor(DEVICE_ROLE_LED, 1, true, BLUE_COLOR);
    readStopButtonStates(1);
    postAllStopStatus(stopButtonStates,7);
    handleSonarAlerts("blue");
  } else if (deviceRole == "FMS_TABLE") {
    setLEDColor(DEVICE_ROLE_LED, 1, true, VIOLET_COLOR);
    // Check if the start match button is pressed
    startButtonState = !digitalRead(START_MATCH_BTN);
    if (startButtonState) {
      Serial.println("Start match button pressed!");
      startMatchPost();
    }
    stopButtonStates[0] = !digitalRead(stopButtonPins[0]);
    if (!stopButtonStates[0]) {
      Serial.println("Field stop button pressed!");
    }
    postSingleStopStatus(0, stopButtonStates[0]);
    // Check alliance status every 500ms
    if (currentMillis - lastStatusCheck >= 500)
    {
      getField_stack_lightStatus();
      lastStatusCheck = currentMillis;
    }
  } else if (deviceRole == "BARGE_LIGHTS") {
    setLEDColor(DEVICE_ROLE_LED, 1, true, SPRINGGREEN_COLOR);
    looptime = 0;
    heartbeatState = 1; // Heartbeat solid on
    switch (LT_MatchState) {
        case 0: // Pre Match
            // MatchStatePre();
            if (coilValues[7]) {
                setLED_Color(BARGE_RED1_LED_START, NUM_LEDS , true, GREEN_COLOR);
            } else{
                setLED_Color(BARGE_RED1_LED_START, NUM_LEDS , false, GREEN_COLOR);
            }
            break;
        case 1 ... 5: // start Matct
            // MatchStateAuto();
            setLED_Color(BARGE_RED1_LED_START, BARGE_RED1_LED_LENGTH, coilValues[8], RED_COLOR);
            setLED_Color(BARGE_RED2_LED_START, BARGE_RED2_LED_LENGTH, coilValues[9], RED_COLOR);
            setLED_Color(BARGE_RED3_LED_START, BARGE_RED3_LED_LENGTH, coilValues[10], RED_COLOR);
            setLED_Color(BARGE_BLUE1_LED_START, BARGE_BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
            setLED_Color(BARGE_BLUE2_LED_START, BARGE_BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
            setLED_Color(BARGE_BLUE3_LED_START, BARGE_BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
            break;
        case 6: // Post Match
            // MatchStatePost();
            if (coilValues[7]) {
                setLED_Color(BARGE_RED1_LED_START, NUM_LEDS, true, GREEN_COLOR);
            } else {
                setLED_Color(BARGE_RED1_LED_START, BARGE_RED1_LED_LENGTH, coilValues[8], RED_COLOR);
                setLED_Color(BARGE_RED2_LED_START, BARGE_RED2_LED_LENGTH, coilValues[9], RED_COLOR);
                setLED_Color(BARGE_RED3_LED_START, BARGE_RED3_LED_LENGTH, coilValues[10], RED_COLOR);
                setLED_Color(BARGE_BLUE1_LED_START, BARGE_BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
                setLED_Color(BARGE_BLUE2_LED_START, BARGE_BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
                setLED_Color(BARGE_BLUE3_LED_START, BARGE_BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
            }
            break;
        case 7: // TimeoutActive
            // MatchStateTimeout();
            // setLED_Color(7, 1, true, BLUE_COLOR);
            break;
        case 8: // PostTimeout
            // MatchStatePostTimeout();
            // setLED_Color(8, 1, true, BLUE_COLOR);
            break;
        default:
            // Do Nothing
            break;
    }
  }

  // print the IP address every 5 seconds
  lastPrint = printNetworkInfo(currentMillis, lastPrint, 5000);
  
  lastHeartbeat = updateHeartbeatLED(currentMillis, lastHeartbeat, 500);

  if(socketDataActivity) {
    setLED_Color(SOCKET_ACTIVITY_LED, 1, true, LTGREEN_COLOR); // Green for activity
  } else {
    setLED_Color(SOCKET_ACTIVITY_LED, 1, false, GREEN_COLOR); // Off for no activity
  }

  // Loop time warning
  unsigned long loopTime = millis() - startTime;
  if (loopTime > loopTimeThresholdMs) {
    Serial.printf("WARNING: Loop time exceeded threshold! Loop time: %lu ms, Threshold: %lu ms\n", loopTime, loopTimeThresholdMs);
    setLED_Color(HEARTBEAT_LED, 1, true, RED_COLOR); // Orange for warning
    loopWarning = 20; // Show warning for 2 blinks
  }

  FastLED.show(g_Brightness); //  Show and delay
  delay(looptime);

}
