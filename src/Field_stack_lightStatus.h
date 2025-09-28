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

#include <HTTPClient.h>
#include <ArduinoJson.h>
#define FASTLED_INTERNAL // Suppress build banner
#include <FastLED.h>
#include "GlobalSettings.h"

// extern Adafruit_NeoPixel strip;

extern const char *baseUrl;
extern bool eth_connected;
extern String deviceRole;
extern String arenaIP;
extern String arenaPort;

extern CRGB g_LEDs[]; // Declare the LED array

// LED Indices and Lengths for FMS_TABLE
const int RED_LED_INDEX = 3;
const int RED_LED_LENGTH = 60;
const int BLUE_LED_INDEX = 60;
const int BLUE_LED_LENGTH = 60;
const int ORANGE_LED_INDEX = 120;
const int ORANGE_LED_LENGTH = 60;
const int GREEN_LED_INDEX = 180;
const int GREEN_LED_LENGTH = 56;

// Constants
const char *STACK_LIGHT_ENDPOINT = "/api/freezy/field_stack_light";

/**
 * @brief Sets the color of two LEDs based on the status.
 *
 * @param ledIndex1 The index of the first LED.
 * @param ledIndex2 The index of the second LED.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
void setLEDColor(int ledIndex1, int length, bool status, CRGB color)
{
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


extern int heartbeatState;
long int hartBeatTck = 0;
long int currentTime = 0;

void getField_stack_lightStatus() {
    // Static variables for timing
    static long int hartBeatTck = 0;
    long int currentTime = millis();

    StaticJsonDocument<JSON_CAPACITY> doc;
    if (!sendHttpGet(STACK_LIGHT_ENDPOINT, "GetField_stack_lightStatus", doc)) {
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