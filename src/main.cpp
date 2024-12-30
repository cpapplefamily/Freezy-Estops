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
#include "WiFiCredentials.h"  // Include the WiFi credentials
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

#define USE_SERIAL Serial

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

//C:\Users\Capplegate\.platformio\penv\Scripts\platformio.exe  run -e esp32-s3-devkitm-1 -t upload
#ifdef ESP32_S3_DEVKITM_1
  const int stopButtonPins[NUM_BUTTONS] = {33,  //Field stop
                                          34,   //1E stop
                                          35,   //1A stop
                                          36,   //2E stop
                                          37,   //2A stop
                                          38,   //3E stop
                                          39};   //3a stop
                                                      
  #define START_MATCH_BTN 40
  #define LEDSTRIP 21           // Pin connected to NeoPixel
#endif // ESP32_S3_DEVKITM_1

//C:\Users\Capplegate\.platformio\penv\Scripts\platformio.exe  run -e esp32dev -t upload
#ifdef ESP32DEV
  const int stopButtonPins[NUM_BUTTONS] = {21,  //Field stop
                                          22,   //1E stop
                                          23,   //1A stop
                                          25,   //2E stop
                                          26,   //2A stop
                                          27,   //3E stop
                                          32};   //3a stop
  #define START_MATCH_BTN 19
  #define LEDSTRIP 4           // Pin connected to NeoPixel
#endif // ESP32DEV

#
volatile bool stopButtonPressed[NUM_BUTTONS] = {false, false, false, false, false, false, false};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, LEDSTRIP, NEO_GRB + NEO_KHZ800);

bool eth_connected = false;

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

IPAddress local_ip(192,168,10,220);
IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secondaryDNS(8,8,4,4);
void intiWifi(){
  WiFi.onEvent(onEvent);
  //eth_connected = true;
	WiFi.mode(WIFI_STA);
	WiFi.config(local_ip,gateway,subnet,primaryDNS,secondaryDNS);
	WiFi.begin(ssid, password);
	USE_SERIAL.print("Connecting to WiFi .. ");
	while(WiFi.status() != WL_CONNECTED){
		USE_SERIAL.print('.');
		delay(1000);
	}
	//WiFi.reconnect();
	Serial.println("Connected to the WiFi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
	delay(3000);
}


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
  
   // Initialize preferences
    preferences.begin("settings", false);

    // Load IP address and DHCP/Static configuration from preferences
    deviceIP = preferences.getString("deviceIP", "");
    useDHCP = preferences.getBool("useDHCP", true);
    g_allianceColor = preferences.getString("allianceColor", "Red");


  #ifdef ESP32DEV
    
    // Connect to the WiFi network
    intiWifi();
  #endif // ESP32 

  #ifdef ESP32_S3_DEVKITM_1
    Network.onEvent(onEvent);
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
  #endif // ESP32


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
        #ifdef ESP32DEV
          Serial.printf("Current WiFi IP Address: %s\n", WiFi.localIP().toString().c_str());
        #endif
        #ifdef ESP32_S3_DEVKITM_1
          Serial.printf("Current Wired IP Address: %s\n", ETH.localIP().toString().c_str());
        #endif
        
    }
        strip.show();
        delay(1000);
}
