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
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "StartMatch.h"               // Include the StartMatch header
#include "postStopStatus.h"           // Include the postStopStatus header
#include "Field_stack_lightStatus.h"  // Include the Field_stack_lightStatus header
#include "WebServerSetup.h"           // Include the WebServerSetup header
#include "GlobalSettings.h"           // Include the GlobalSettings header

#ifndef ETH_PHY_CS
#define ETH_PHY_TYPE     ETH_PHY_W5500
#define ETH_PHY_ADDR     1
#define ETH_PHY_CS       14
#define ETH_PHY_IRQ      10
#define ETH_PHY_RST      9
#define ETH_PHY_SPI_HOST SPI2_HOST
#define ETH_PHY_SPI_SCK  13
#define ETH_PHY_SPI_MISO 12
#define ETH_PHY_SPI_MOSI 11
#endif

// Define preferences objects
String g_allianceColor;

// Define the base URL for the API
const char* baseUrl = "http://192.168.10.124:8080";
//const char* baseUrl = "http://10.0.100.5:8080";

// Define the IP address and DHCP/Static configuration
extern String deviceIP;
extern bool useDHCP;

// Pins connected to the stop button
#define NUM_BUTTONS 7
const int stopButtonPins[NUM_BUTTONS] = {0, 1, 2, 3, 4, 5, 6};
volatile bool stopButtonPressed[NUM_BUTTONS] = {false, false, false, false, false, false, false};

#define START_MATCH_BTN 33
#define LEDSTRIP 21           // Pin connected to NeoPixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, LEDSTRIP, NEO_GRB + NEO_KHZ800);

bool eth_connected = false;

void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
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
    default: break;
  }
}

// Interrupt handlers for the stop buttons
void IRAM_ATTR handleStopButtonPress0() { stopButtonPressed[0] = true; }
void IRAM_ATTR handleStopButtonPress1() { stopButtonPressed[1] = true; }
void IRAM_ATTR handleStopButtonPress2() { stopButtonPressed[2] = true; }
void IRAM_ATTR handleStopButtonPress3() { stopButtonPressed[3] = true; }
void IRAM_ATTR handleStopButtonPress4() { stopButtonPressed[4] = true; }
void IRAM_ATTR handleStopButtonPress5() { stopButtonPressed[5] = true; }
void IRAM_ATTR handleStopButtonPress6() { stopButtonPressed[6] = true; }

// Setup function
void setup() {
  Serial.begin(115200);
  delay(5000);

  // Initialize the NeoPixel strip
  strip.begin();
  strip.setBrightness(20);
  strip.show(); // Initialize all pixels to 'off'

  // Initialize the start match button
  pinMode(START_MATCH_BTN, INPUT_PULLUP);

   // Initialize the stop buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
      pinMode(stopButtonPins[i], INPUT_PULLUP);
  }
    
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[0]), handleStopButtonPress0, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[1]), handleStopButtonPress1, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[2]), handleStopButtonPress2, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[3]), handleStopButtonPress3, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[4]), handleStopButtonPress4, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[5]), handleStopButtonPress5, FALLING);
  attachInterrupt(digitalPinToInterrupt(stopButtonPins[6]), handleStopButtonPress6, FALLING);

  Network.onEvent(onEvent);
  
   // Initialize preferences
    preferences.begin("settings", false);

    // Load IP address and DHCP/Static configuration from preferences
    deviceIP = preferences.getString("deviceIP", "");
    useDHCP = preferences.getBool("useDHCP", true);
    g_allianceColor = preferences.getString("allianceColor", "Red");

    // Initialize Ethernet with DHCP or Static IP
    if (useDHCP) {
        ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
    } else {
        IPAddress localIP;
        if (localIP.fromString(deviceIP)) {
          Serial.println("Setting static IP address.");
          // THis is not working Need to fix
            ETH.config(localIP);
            ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
        } else {
            Serial.println("Invalid static IP address. Falling back to DHCP.");
            ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
        }
    }

    // Wait for Ethernet to connect
    while (!eth_connected) {
        delay(100);
    }

    // Print the IP address
    Serial.print("init - IP Address: ");
    Serial.println(ETH.localIP());


  // Set up the web server
  setupWebServer();

}

// Main loop
void loop() {
  static unsigned long lastStatusCheck = 0;
  static unsigned long lastPrint = 0;
    unsigned long currentMillis = millis();

    // Check if the start match button is pressed
    if (digitalRead(START_MATCH_BTN) == LOW) {
        Serial.println("Start match button pressed!");
        startMatchPost();
    }

    // Check if the stop buttons are pressed
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (stopButtonPressed[i]) {
            stopButtonPressed[i] = false;
            Serial.printf("Stop button %d pressed!\n", i);
            postStopStatus(i, false);
        } 
    }
    
    // Check alliance status every 500ms
    if (currentMillis - lastStatusCheck >= 500) {
        getField_stack_lightStatus();
        lastStatusCheck = currentMillis;  
    }
    // print the IP address every 5 seconds
    if (currentMillis - lastPrint >= 5000) {
        lastPrint = currentMillis;
        deviceIP = preferences.getString("deviceIP", "");
        Serial.printf("Preferences IP Address: %s\n", deviceIP.c_str());
        useDHCP = preferences.getBool("useDHCP", true);
        Serial.printf("Use DHCP: %s\n", useDHCP ? "true" : "false");
        Serial.printf("Current IP Address: %s\n", ETH.localIP().toString().c_str());
        
        
    }
        strip.show();
}
