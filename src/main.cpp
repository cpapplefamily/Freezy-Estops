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
#define FASTLED_INTERNAL        // Suppress build banner
#include <FastLED.h>
#include <WebSocketsClient.h>


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

// Define the LED strip
#define LED_PIN        	47
#define NUM_LED_SRIPS  2
#define NUM_LEDS_SRIPS_L  5
#define NUM_LEDS_PER_M      30   
#define NUM_LEDS       	(NUM_LED_SRIPS * NUM_LEDS_SRIPS_L * NUM_LEDS_PER_M)
#define SECTION_LENGTH  49
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

int g_Brightness = 255;//15;         // 0-255 LED brightness scale
int g_PowerLimit = 50000;//900;        // 900mW Power Limit
CRGB g_LEDs[NUM_LEDS] = {0};    // Frame buffer for FastLED
const CRGB RED_COLOR = CRGB(255, 0, 0);
const CRGB BLUE_COLOR = CRGB(0, 0, 175);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR = CRGB(0, 255, 0);
const CRGB WHITE_COLOR = CRGB(20, 20, 20);

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

void setLEDColor(int ledIndex1, int length, bool status, CRGB color) {
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

bool coilValues[9]; // Adjust the size based on the number of coils you want to store
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
    LT_MatchState = registers[6];
    
		// Print the Coils array
    coils = data["Coils"];
    USE_SERIAL.print("Coils: ");
    int index = 0;
    for (int i : {4,5,13, 14, 16, 17, 19, 20, 21}) {
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
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
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
void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      USE_SERIAL.println("WiFi STA Started");
      //WiFi.setHostname("Freezy_Red");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //USE_SERIAL.printf("WiFi STA Got IP: '%s'\n", WiFi.localIP().toString().c_str());
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_START:
      USE_SERIAL.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("Freezy_ScoreTable");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: 
      USE_SERIAL.println("ETH Connected"); 
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:    
      USE_SERIAL.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif)); 
      USE_SERIAL.println(ETH);
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      USE_SERIAL.println("ETH Lost IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      USE_SERIAL.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      USE_SERIAL.println("ETH Stopped");
      eth_connected = false;
      break;
    default: break;
  }
}


// Setup function
void setup() {
  USE_SERIAL.begin(115200);
  delay(5000);

  // Initialize the LED strip
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);;               // Add our LED strip to the FastLED library
	FastLED.setBrightness(g_Brightness);
  //set_max_power_indicator_LED(LED_BUILTIN);                               // Light the builtin LED if we power throttle
  FastLED.setMaxPowerInMilliWatts(g_PowerLimit);                          // Set the power limit, above which brightness will be throttled


  Network.onEvent(onEvent);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, ETH_PHY_SPI_HOST, ETH_PHY_SPI_SCK, ETH_PHY_SPI_MISO, ETH_PHY_SPI_MOSI);
    
  // Wait for Ethernet to connect
  while (!eth_connected) {
    delay(100);
  }
  // Print the IP address
  Serial.print("init - IP Address: ");
  Serial.println(ETH.localIP());

  // Connect to the WebSocket server
  Serial.println("Connecting to WebSocket server...");
  webSocket.setExtraHeaders("Origin: http://192.168.10.124:8080");
  webSocket.begin("192.168.10.124", 8080, "ws://192.168.10.124:8080/setup/field_testing/websocket");

  // event handler
	webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
}


int heartbeatState = 1;
bool endGameFlasher = false;

// Main loop
void loop() {
  webSocket.loop();
  static unsigned long lastStatusCheck = 0;
  static unsigned long lastPrint = 0;
  unsigned long currentMillis = millis();
  FastLED.clear(); // Clear the LED strip

  switch(LT_MatchState){
		case 0: //Pre Match
			//MatchStatePre();
      if(coilValues[0] || coilValues[1]){
        setLEDColor(0, NUM_LEDS, true, GREEN_COLOR);
      }else{
        setLEDColor(0, NUM_LEDS, false, GREEN_COLOR);
      }
			break;
		case 1 ... 5: //start Matct
      //MatchStateAuto();
      // If Endgame warning
      if(coilValues[8]){
        EVERY_N_MILLISECONDS(500){
          endGameFlasher = !endGameFlasher;
        }
        setLEDColor(RED3_LED, RED1_LED_LENGTH + RED2_LED_LENGTH + RED3_LED_LENGTH, endGameFlasher, RED_COLOR);
        setLEDColor(BLUE1_LED, BLUE1_LED_LENGTH + BLUE2_LED_LENGTH + BLUE3_LED_LENGTH, endGameFlasher, BLUE_COLOR);
      }else{
        // If not Playoffs
        if(!coilValues[7]){
          //If coopertition is active
          if(coilValues[2] && coilValues[3] && coilValues[4] && coilValues[5]){
            setLEDColor(RED3_LED, RED1_LED_LENGTH + RED2_LED_LENGTH + RED3_LED_LENGTH, true, RED_COLOR);
            setLEDColor(BLUE1_LED, BLUE1_LED_LENGTH + BLUE2_LED_LENGTH + BLUE3_LED_LENGTH, true, BLUE_COLOR);
          }else{
            if(coilValues[2]){
              setLEDColor(RED1_LED, RED1_LED_LENGTH, true, RED_COLOR);
            }
            if(coilValues[3]){
              setLEDColor(RED2_LED, RED1_LED_LENGTH, true, RED_COLOR);
            }/*  */
            if(coilValues[4]){
              setLEDColor(BLUE1_LED, BLUE1_LED_LENGTH, true, BLUE_COLOR);
            }
            if(coilValues[5]){
              setLEDColor(BLUE2_LED, BLUE2_LED_LENGTH, true, BLUE_COLOR);
            }
          }
        }else{
          //is Playoffs
          setLEDColor(RED3_LED, RED1_LED_LENGTH + RED2_LED_LENGTH + RED3_LED_LENGTH, true, RED_COLOR);
          setLEDColor(BLUE1_LED, BLUE1_LED_LENGTH + BLUE2_LED_LENGTH + BLUE3_LED_LENGTH, true, BLUE_COLOR);
        }
      }
      break;
    case 6: //Post Match
      //MatchStatePost();
      if(coilValues[6]){
        setLEDColor(RED3_LED, RED1_LED_LENGTH + RED2_LED_LENGTH + RED3_LED_LENGTH, true, RED_COLOR);
        setLEDColor(BLUE1_LED, BLUE1_LED_LENGTH + BLUE2_LED_LENGTH + BLUE3_LED_LENGTH, true, BLUE_COLOR);
      }
    break;
    case 7: //TimeoutActive
      //MatchStateTimeout();
      //setLEDColor(7, 1, true, BLUE_COLOR);
      break;
    case 8: //PostTimeout
      //MatchStatePostTimeout();
      //setLEDColor(8, 1, true, BLUE_COLOR);
      break;
		default:
			//Do Nothing
			break;
	}

  // print the IP address every 5 seconds
  if (currentMillis - lastPrint >= 5000) {
    lastPrint = currentMillis;
    USE_SERIAL.printf("Current Wired IP Address: %s\n", ETH.localIP().toString().c_str());
  }
    
    // Use a case statement to set the g_LEDs color based on the heartbeat variable
    switch (heartbeatState) {
        case 0:
            g_LEDs[HEARTBEAT_LED] = CRGB::Black;
            break;
        case 1:
            g_LEDs[HEARTBEAT_LED] = CRGB(20, 15, 25); 
            break;
        case 2:
            g_LEDs[HEARTBEAT_LED] = CRGB::Orange;
            break;
        default:
            g_LEDs[HEARTBEAT_LED] = CRGB::Red;
            break;
    }

    if(socketDataActivity){
      g_LEDs[SOCKET_ACTIVITY_LED] = CRGB(20, 0, 0);
    }else{
      g_LEDs[SOCKET_ACTIVITY_LED] = CRGB::Black;
    }
    
    
    FastLED.show(g_Brightness); //  Show and delay

    EVERY_N_MILLISECONDS(1000){
      if(eth_connected){
        if (heartbeatState == 0) {
          heartbeatState = 1;
        } else {
          heartbeatState = 0;
        }
      }else      {
        if (heartbeatState == 99) {
          heartbeatState = 0;
        } else {
          heartbeatState = 99;
        }
      }
    }

}
