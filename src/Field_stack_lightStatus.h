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
 * Retrieves the field stack light status via HTTP GET and updates LEDs for FMS_TABLE role.
 */
void getField_stack_lightStatus() {


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