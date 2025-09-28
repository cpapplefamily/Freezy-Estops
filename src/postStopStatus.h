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
#ifndef POSTSTOPSTATUS_H
#define POSTSTOPSTATUS_H

// Includes
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "GlobalSettings.h"

// Constants
const char *STOP_STATUS_ENDPOINT = "/api/freezy/eStopState";

/**
 * Sends an HTTP POST request to update the stop status.
 *
 * @param i The channel number.
 * @param stopButtonPressed The state of the stop button (false if pressed, true otherwise).
 */
void postSingleStopStatus(int i, bool stopButtonPressed)
{
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonArray array = payload.to<JsonArray>();
    JsonObject channel = array.createNestedObject();

    // Map channel based on device role
    if (deviceRole == "FMS_TABLE")
    {
        channel["channel"] = 0; // Field stop always uses channel 0
    }
    else if (deviceRole == "RED_ALLIANCE")
    {
        channel["channel"] = i;
    }
    else if (deviceRole == "BLUE_ALLIANCE")
    {
        channel["channel"] = i + 6; // Offset by 6 for Blue Alliance
    }
    channel["state"] = stopButtonPressed;

    String jsonPayload;
    serializeJson(payload, jsonPayload);
    sendHttpPost(STOP_STATUS_ENDPOINT, jsonPayload, "PostSingleStopStatus", true);
}

/**
 * Sends an HTTP POST request to update the stop status for all 6 channels.
 *
 * @param stopButtonStates An array of the states of the stop buttons (false if pressed, true otherwise).
 */
void postAllStopStatus(bool stopButtonStates[6], int startingChannel)
{
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonArray array = payload.to<JsonArray>();

    // Create payload for all channels
    for (int i = 0; i < 6; i++)
    {
        JsonObject channel = array.createNestedObject();
        channel["channel"] = i + startingChannel;
        channel["state"] = stopButtonStates[i];
    }

    String jsonPayload;
    serializeJson(payload, jsonPayload);
    sendHttpPost(STOP_STATUS_ENDPOINT, jsonPayload, "postAllStopStatus", true);
}

#endif // POSTSTOPSTATUS_H