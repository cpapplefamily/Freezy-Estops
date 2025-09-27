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

#ifndef STARTMATCH_H
#define STARTMATCH_H

// Includes
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "GlobalSettings.h"

// External Variables
extern const char *baseUrl;
extern bool eth_connected;
extern bool printSerialDebug;

// Constants
const char *START_MATCH_ENDPOINT = "/api/freezy/startMatch";

/**
 * Sends an HTTP POST request to start a match by sending {"match":"start"} to the API.
 */
void startMatchPost() {
    // Create JSON payload
    StaticJsonDocument<JSON_CAPACITY> payload;
    payload["match"] = "start";
    String jsonPayload;
    serializeJson(payload, jsonPayload);

    // Send the request
    sendHttpPost(START_MATCH_ENDPOINT,jsonPayload, "StartMatch.h", false);
}

#endif // STARTMATCH_H