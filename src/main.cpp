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

// Use TRIG and ECHO pins configured here. Change to match your wiring.
const uint8_t TRIG_PIN = 21;
const uint8_t ECHO_PIN = 17;
const unsigned long PULSE_TIMEOUT = 30000UL; // 30 ms -> ~5 meters

SonarSensor sonar(TRIG_PIN, ECHO_PIN, PULSE_TIMEOUT);

// Example configuration
const float ALERT_THRESHOLD_CM = 30.0f;     // trigger when object closer than 30 cm
const unsigned long ALERT_HOLD_MS = 1000UL; // must stay below threshold for 1 second
const unsigned long MIN_OFF_MS = 1000UL;   // once triggered, require 10 seconds of cleared before allowing retrigger

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

#define USE_SERIAL Serial

// Define the base URL for the API
//const char *baseUrl = "http://192.168.10.124:8080";
const char* baseUrl = "http://192.168.10.124:8080";

// Define the IP address and DHCP/Static configuration
extern String deviceRole;
extern String deviceIP;
extern String deviceGWIP;
extern bool useDHCP;

// Pins connected to the stop button
#define NUM_BUTTONS 7


// C:\Users\Capplegate\.platformio\penv\Scripts\platformio.exe  run -e esp32-s3-devkitm-1 -t upload
#ifdef ESP32_S3_DEVKITM_1
const int stopButtonPins[NUM_BUTTONS] = {33,  // Field stop
                                         1,   // 1E stop
                                         2,   // 1A stop
                                         3,   // 2E stop
                                         15,  // 2A stop
                                         18,  // 3E stop
                                         16}; // 3A stop

#define START_MATCH_BTN 34
#define LEDSTRIP 47          // Pin connected to NeoPixel
#define NUM_LED_SRIPS  5
#define NUM_LEDS_SRIPS_L  5
#define NUM_LEDS_PER_M      30   
#define NUM_LEDS       	(NUM_LED_SRIPS * NUM_LEDS_SRIPS_L * NUM_LEDS_PER_M)
#define SECTION_LENGTH  124
#define BLUE1_LED       0
#define BLUE1_LED_LENGTH    SECTION_LENGTH
#define BLUE2_LED           SECTION_LENGTH
#define BLUE2_LED_LENGTH    SECTION_LENGTH
#define BLUE3_LED           SECTION_LENGTH * 2
#define BLUE3_LED_LENGTH    SECTION_LENGTH
#define RED1_LED            SECTION_LENGTH * 5
#define RED1_LED_LENGTH     SECTION_LENGTH
#define RED2_LED            SECTION_LENGTH * 4
#define RED2_LED_LENGTH     SECTION_LENGTH
#define RED3_LED            SECTION_LENGTH * 3
#define RED3_LED_LENGTH     SECTION_LENGTH
#define HEARTBEAT_LED       NUM_LEDS - 1
#define SOCKET_ACTIVITY_LED NUM_LEDS - 2
#define RESERVED1           NUM_LEDS - 3
#define RESERVED2           NUM_LEDS - 4
#define RESERVED3           NUM_LEDS - 5
#define RESERVED4           NUM_LEDS - 6

int g_Brightness = 255;        // 15;         // 0-255 LED brightness scale
int g_PowerLimit = 50000;    // 900;        // 900mW Power Limit
CRGB g_LEDs[NUM_LEDS] = {0}; // Frame buffer for FastLED

// #define ONBOARD_LED 26 //Board does not have
#define ONBOARD_RGB 21

// Adafruit_NeoPixel onBoardRGB = Adafruit_NeoPixel(10, ONBOARD_RGB, NEO_GRB + NEO_KHZ800);
#endif // ESP32_S3_DEVKITM_1

// C:\Users\Capplegate\.platformio\penv\Scripts\platformio.exe  run -e esp32dev -t upload
#ifdef ESP32DEV
const int stopButtonPins[NUM_BUTTONS] = {21,  // Field stop
                                         22,  // 1E stop
                                         23,  // 1A stop
                                         25,  // 2E stop
                                         26,  // 2A stop
                                         27,  // 3E stop
                                         32}; // 3a stop
#define START_MATCH_BTN 19
#define LEDSTRIP 4 // Pin connected to NeoPixel
#define ONBOARD_LED 2
#endif // ESP32DEV

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, LEDSTRIP, NEO_GRB + NEO_KHZ800);

WebSocketsClient webSocket;
int LT_MatchState = 0;
bool socketDataActivity = false;

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}

void setLED_Color(int ledIndex1, int length, bool status, CRGB color) {
  if (status) {
      for (int i = ledIndex1; i < ledIndex1 + length; i++) {
        g_LEDs[i] = color; // Set LED to color
      }
  } else {
      for (int i = ledIndex1; i < ledIndex1 + length; i++) {
        g_LEDs[i] = CRGB::Black; // Turn off the LED
      }
  }
}

bool coilValues[14]; // Adjust the size based on the number of coils you want to store
JsonArray coils;

void parser(String s){
  socketDataActivity = !socketDataActivity;
  DynamicJsonDocument doc(6145);
	deserializeJson(doc, s);
  JsonObject json= doc.as<JsonObject>();

	const char* type = doc["type"]; // "arenaStatus, matchTime, ..."

	JsonObject data = doc["data"];	// Most Jason Files have a "data" section

	if(strcmp(type, "plcIoChange") == 0 ){

    // set the match state
    JsonArray registers = data["Registers"];
    LT_MatchState = registers[3];
    
		// Print the Coils array
    coils = data["Coils"];
    USE_SERIAL.print("Coils: ");
    int index = 0;
    for (int i : {0,1,2,3,4,5,6,7,8,9,10,11,12,13}) {
      if (i < coils.size()) {
        bool coilValue = coils[i].as<bool>();
        coilValues[index++] = coilValue; // Copy the value to the global arra
        USE_SERIAL.print(coils[i].as<bool>());
        USE_SERIAL.print(" ");
      } else {
        USE_SERIAL.print("N/A ");
      }
    }
    USE_SERIAL.println();
  
  }

}

//A string to concat the socketData together
String socketData;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	//USE_SERIAL.printf("type: %x\n", type);
			
	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
			socketData = (char * )payload;
			parser(socketData);
			break;
		case WStype_BIN:
			//USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);
			break;
		case WStype_ERROR:	
			USE_SERIAL.println("[************WSc ERROR***********]");		
		case WStype_FRAGMENT_TEXT_START:
			socketData = (char * )payload;
			//USE_SERIAL.printf("[WSc] Fragment Text Start: %s\n", payload);
			//USE_SERIAL.println(socketData);
			//USE_SERIAL.println("WStype_FRAGMENT_TEXT_START");
			break;
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
			socketData += (char * )payload;
			break;
		case WStype_FRAGMENT_FIN:
			socketData += (char * )payload; 
			parser(socketData);
			//USE_SERIAL.println(socketData);
			break;
		case WStype_PONG:
			//USE_SERIAL.printf("[WSc] WStype_PONG: Ping reply\n");
			break;
	}

}


bool eth_connected = false;
int heartbeatState = 0;

void onEvent(arduino_event_id_t event, arduino_event_info_t info)
{
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

IPAddress local_ip(192, 168, 10, 220);
IPAddress local_gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
void intiWifi()
{
  WiFi.onEvent(onEvent);
  // eth_connected = true;
  WiFi.mode(WIFI_STA);
  WiFi.config(local_ip, local_gateway, subnet, primaryDNS, secondaryDNS);
  WiFi.begin(ssid, password);
  USE_SERIAL.print("Connecting to WiFi .. ");
  while (WiFi.status() != WL_CONNECTED)
  {
    USE_SERIAL.print('.');
    delay(1000);
  }
  // WiFi.reconnect();
  Serial.println("Connected to the WiFi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(3000);
}

// Setup function
void setup()
{
  Serial.begin(115200);
  delay(5000);

  deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
  if(deviceRole != "BARGE_LIGHTS"){
    g_Brightness = 15;
    g_PowerLimit = 900;
  }
  // Initialize the LED strip
  FastLED.addLeds<WS2812B, LEDSTRIP, GRB>(g_LEDs, NUM_LEDS); // Add our LED strip to the FastLED library
  FastLED.setBrightness(g_Brightness);
  // set_max_power_indicator_LED(LED_BUILTIN);                               // Light the builtin LED if we power throttle
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit); // Set the power limit, above which brightness will be throttled

  // Initialize the start match button
  pinMode(START_MATCH_BTN, INPUT_PULLUP);

  // Initialize the stop buttons
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(stopButtonPins[i], INPUT);
  }

  // Initialize preferences
  preferences.begin("settings", false);

  // Load IP address and DHCP/Static configuration from preferences
  deviceIP = preferences.getString("deviceIP", "10.0.100.240");
  deviceGWIP = preferences.getString("deviceGWIP", "10.0.100.3");
  useDHCP = preferences.getBool("useDHCP", true);

  sonar.begin();
  sonar.setMinOffTime(MIN_OFF_MS);
  // start background sampling (non-blocking for the main loop)
  sonar.startBackground(200);

#ifdef ESP32DEV
  // Connect to the WiFi network
  intiWifi();
  pinMode(ONBOARD_LED, OUTPUT);
#endif // ESP32

#ifdef ESP32_S3_DEVKITM_1
  Network.onEvent(onEvent);
  // Initialize Ethernet with DHCP or Static IP
  if (true)
  {
    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
  }
  else
  {
    IPAddress localIP;
    IPAddress localGW;
    if (localIP.fromString(deviceIP))
    {
      Serial.println("Setting static IP address.");
      // THis is not working Need to fix
      localGW.fromString(deviceGWIP);
      ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
      ETH.config(localIP, localGW, subnet, primaryDNS, secondaryDNS);
    }
    else
    {
      Serial.println("Invalid static IP address. Falling back to DHCP.");
      ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
    }
  }

  // Wait for Ethernet to connect
  while (!eth_connected)
  {
    Serial.print(".");
    delay(500);
  }
  // Print the IP address
  Serial.print("init - IP Address: ");
  Serial.println(ETH.localIP());

#endif // ESP32

  // Set up the web server
  setupWebServer();

  // Connect to the WebSocket server
  Serial.println("Connecting to WebSocket server...");
  webSocket.setExtraHeaders("Origin: http://192.168.10.124:8080");
  webSocket.begin("192.168.10.124", 8080, "ws://192.168.10.124:8080/setup/field_testing/websocket");

  // event handler
	webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

bool stopButtonStates[NUM_BUTTONS-1];
bool startButtonState = false;
int looptime = 100;
bool sonarAlertSent = false; // one-shot flag for sonar alert
float sonarDistance;

// Main loop
void loop()
{
  static unsigned long lastStatusCheck = 0;
  static unsigned long lastPrint = 0;
  unsigned long currentMillis = millis();
  deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
  //FastLED.clear(); // Clear the LED strip
  looptime = 100;

  webSocket.loop(); // Handle WebSocket events
  
  
  if (deviceRole == "RED_ALLIANCE")
  {
    setLEDColor(1, 1, true, RED_COLOR);  // RED
    // Create an array to store the states of the stop buttons
    for (int i = 0; i < NUM_BUTTONS-1; i++)
    {
      stopButtonStates[i] = !digitalRead(stopButtonPins[i+1]);
    }
    
    // Call the postAllStopStatus method with the array
    //postAllStopStatus(stopButtonStates,1);
    
    sonarDistance = sonar.getLastDistance();
    
/*     Serial.print("Sonar Distance: ");
    Serial.print(d);
    Serial.println(" cm"); */
    // Example: also use belowThresholdFor (non-blocking since background samples run separately)
    if (sonar.belowThresholdFor(ALERT_THRESHOLD_CM))
    {
      // only fire once per event
      if (!sonarAlertSent)
      {
        Serial.println("ALERT: object within threshold for >=1s (one-shot)");
        sonarAlertSent = true; // prevent repeat until cleared
        
        switch (LT_MatchState)
        {
          case 0: //Pre Match
          //do nothing
          break;
          case 1 ... 6:
          postElement("red","ProcessorAlgae");
          break;
          case 7: //TimeoutActive
          //do nothing
          break;
          case 8: //PostTimeout
          //do nothing
          break;
          default:
          break;
        }
      }
    }
    else
    {
      // reset one-shot when sensor clears (simple reset). If you want a hold before reset,
      // replace this with a timed/hysteresis check.
      sonarAlertSent = false;
    }
    
  }
  else if (deviceRole == "BLUE_ALLIANCE")
  {
    setLEDColor(1, 1, true, BLUE_COLOR);  // RED
    // Create an array to store the states of the stop buttons
    for (int i = 0; i < NUM_BUTTONS-1; i++)
    {
      stopButtonStates[i] = !digitalRead(stopButtonPins[i+1]);
    }
    
    // Call the postAllStopStatus method with the array
    postAllStopStatus(stopButtonStates,7);
    
    sonarDistance = sonar.getLastDistance();

    // Example: also use belowThresholdFor (non-blocking since background samples run separately)
    if (sonar.belowThresholdFor(ALERT_THRESHOLD_CM))
    {
      // only fire once per event
      if (!sonarAlertSent)
      {
        Serial.println("ALERT: object within threshold for >=1s (one-shot)");
        sonarAlertSent = true; // prevent repeat until cleared

        switch (LT_MatchState)
        {
        case 0: //Pre Match
          //do nothing
          break;
        case 1 ... 6:
          postElement("red","ProcessorAlgae");
          break;
        case 7: //TimeoutActive
          //do nothing
          break;
        case 8: //PostTimeout
          //do nothing
          break;
        default:
          break;
        }
      }
    }
    else
    {
      // reset one-shot when sensor clears (simple reset). If you want a hold before reset,
      // replace this with a timed/hysteresis check.
      sonarAlertSent = false;
    }
  }
  else if (deviceRole == "FMS_TABLE")
  {
    setLEDColor(1, 1, true, VIOLET_COLOR);  // RED
    // Check if the start match button is pressed
    startButtonState = !digitalRead(START_MATCH_BTN);
    if (startButtonState)
    {
      Serial.println("Start match button pressed!");
      startMatchPost();
    }
    
    stopButtonStates[0] = !digitalRead(stopButtonPins[0]);
    postSingleStopStatus(0, stopButtonStates[0]);

    // Check alliance status every 500ms
    if (currentMillis - lastStatusCheck >= 500)
    {
      getField_stack_lightStatus();
      lastStatusCheck = currentMillis;
    }
  }else if (deviceRole == "BARGE_LIGHTS"){
    looptime = 0;
    switch(LT_MatchState){
      case 0: //Pre Match
        //MatchStatePre();
        if(coilValues[7]){
          setLED_Color(0, NUM_LEDS, true, GREEN_COLOR);
        }else{
          setLED_Color(0, NUM_LEDS, false, GREEN_COLOR);
        }
        break;
      case 1 ... 5: //start Matct
        //MatchStateAuto();
        setLED_Color(RED1_LED, RED1_LED_LENGTH, coilValues[8], RED_COLOR);
        setLED_Color(RED2_LED, RED1_LED_LENGTH, coilValues[9], RED_COLOR);
        setLED_Color(RED3_LED, RED3_LED_LENGTH, coilValues[10], RED_COLOR);
        setLED_Color(BLUE1_LED, BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
        setLED_Color(BLUE2_LED, BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
        setLED_Color(BLUE3_LED, BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
        break;
      case 6: //Post Match
        //MatchStatePost();
        if(coilValues[7]){
          setLED_Color(0, NUM_LEDS, true, GREEN_COLOR);
        }else{
          setLED_Color(RED1_LED, RED1_LED_LENGTH, coilValues[8], RED_COLOR);
          setLED_Color(RED2_LED, RED1_LED_LENGTH, coilValues[9], RED_COLOR);
          setLED_Color(RED3_LED, RED3_LED_LENGTH, coilValues[10], RED_COLOR);
          setLED_Color(BLUE1_LED, BLUE1_LED_LENGTH, coilValues[11], BLUE_COLOR);
          setLED_Color(BLUE2_LED, BLUE2_LED_LENGTH, coilValues[12], BLUE_COLOR);
          setLED_Color(BLUE3_LED, BLUE3_LED_LENGTH, coilValues[13], BLUE_COLOR);
        }
      break;
      case 7: //TimeoutActive
        //MatchStateTimeout();
        //setLED_Color(7, 1, true, BLUE_COLOR);
        break;
      case 8: //PostTimeout
        //MatchStatePostTimeout();
        //setLED_Color(8, 1, true, BLUE_COLOR);
        break;
      default:
        //Do Nothing
        break;
    }
  }


  // print the IP address every 5 seconds
  if (currentMillis - lastPrint >= 5000)
  {
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

  int heartbeat_LED = 0;
  // Use a case statement to set the g_LEDs color based on the heartbeat variable
  switch (heartbeatState)
  {
  case 0:
    g_LEDs[heartbeat_LED] = CRGB::Black;
    break;
  case 1:
    g_LEDs[heartbeat_LED] = CRGB::White;
    break;
  case 2:
    g_LEDs[heartbeat_LED] = CRGB::Orange;
    break;
  default:
    g_LEDs[heartbeat_LED] = CRGB::Red;
    break;
  }

  FastLED.show(g_Brightness); //  Show and delay
  delay(looptime);
}
