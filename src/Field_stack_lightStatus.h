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

#ifndef FIELDSTACKLIGHTSTATUS_H
#define FIELDSTACKLIGHTSTATUS_H

// Includes
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define FASTLED_INTERNAL // Suppress build banner
#include <FastLED.h>
#include "GlobalSettings.h"

// External Variables
extern const char *baseUrl;
extern bool eth_connected;
extern String deviceRole;
extern String arenaIP;
extern String arenaPort;
extern CRGB g_LEDs[];
extern int heartbeatState;
extern bool printSerialDebug;

// Constants
const char *STACK_LIGHT_ENDPOINT = "/api/freezy/field_stack_light";

// LED Colors
const CRGB RED_COLOR = CRGB(255, 0, 0);
const CRGB BLUE_COLOR = CRGB(0, 0, 255);
const CRGB VIOLET_COLOR = CRGB(255, 0, 255);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR = CRGB(0, 255, 0);
const CRGB WHITE_COLOR = CRGB(20, 20, 20);

// LED Indices and Lengths for FMS_TABLE
const int RED_LED_INDEX = 2;
const int RED_LED_LENGTH = 60;
const int BLUE_LED_INDEX = 60;
const int BLUE_LED_LENGTH = 60;
const int ORANGE_LED_INDEX = 120;
const int ORANGE_LED_LENGTH = 60;
const int GREEN_LED_INDEX = 180;
const int GREEN_LED_LENGTH = 56;

// Helper Functions
/**
 * Sends an HTTP GET request to the specified endpoint and returns the JSON response.
 * @param functionName The name of the calling function for debug output.
 * @param doc The JSON document to store the parsed response.
 * @return True if the request was successful and JSON parsed correctly, false otherwise.
 */
static bool sendHttpGet(const String &functionName, StaticJsonDocument<JSON_CAPACITY> &doc) {
    if (!eth_connected) {
        Serial.println("Network not connected! [FSL]");
        heartbeatState = (heartbeatState == 0) ? 3 : 0;
        return false;
    }

    HTTPClient http;
    String url = "http://" + arenaIP + ":" + arenaPort + STACK_LIGHT_ENDPOINT;
    if (printSerialDebug) {
        Serial.println("URL: " + url);
    }
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String response = http.getString();
        if (printSerialDebug) {
            Serial.println(functionName);
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            Serial.println("Response:");
            Serial.println(response);
        }

        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
            http.end();
            return false;
        }
        heartbeatState = (heartbeatState == 0) ? 1 : 0;
    } else {
        Serial.println(functionName);
        Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
        heartbeatState = (heartbeatState == 0) ? 2 : 0;
        http.end();
        return false;
    }

    http.end();
    return true;
}

/**
 * Sets the color of a range of LEDs based on the status.
 * @param ledIndex The starting index of the LED range.
 * @param length The number of LEDs to set.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
void setLEDColor(int ledIndex, int length, bool status, CRGB color) {
    for (int i = ledIndex; i < ledIndex + length; i++) {
        g_LEDs[i] = status ? color : CRGB::Black;
    }
}

/**
 * Retrieves the field stack light status via HTTP GET and updates LEDs for FMS_TABLE role.
 */
void getField_stack_lightStatus() {
    // Static variables for timing
    static long int hartBeatTck = 0;
    long int currentTime = millis();

    StaticJsonDocument<JSON_CAPACITY> doc;
    if (!sendHttpGet("GetField_stack_lightStatus", doc)) {
        return;
    }

    // Parse stack light statuses with default false if key is missing
    bool redStackLightStatus = doc.containsKey("redStackLight") ? doc["redStackLight"].as<bool>() : false;
    bool blueStackLightStatus = doc.containsKey("blueStackLight") ? doc["blueStackLight"].as<bool>() : false;
    bool orangeStackLightStatus = doc.containsKey("orangeStackLight") ? doc["orangeStackLight"].as<bool>() : false;
    bool greenStackLightStatus = doc.containsKey("greenStackLight") ? doc["greenStackLight"].as<bool>() : false;

    // Update LEDs for FMS_TABLE role
    if (deviceRole == "FMS_TABLE") {
        setLEDColor(RED_LED_INDEX, RED_LED_LENGTH, redStackLightStatus, RED_COLOR);
        setLEDColor(BLUE_LED_INDEX, BLUE_LED_LENGTH, blueStackLightStatus, BLUE_COLOR);
        setLEDColor(ORANGE_LED_INDEX, ORANGE_LED_LENGTH, orangeStackLightStatus, ORANGE_COLOR);
        setLEDColor(GREEN_LED_INDEX, GREEN_LED_LENGTH, greenStackLightStatus, GREEN_COLOR);
    }
}

#endif // FIELDSTACKLIGHTSTATUS_H