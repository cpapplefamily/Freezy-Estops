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

// Helper Functions
/**
 * Sends an HTTP POST request with the given JSON payload to the specified endpoint.
 * @param jsonPayload The JSON string to send.
 * @param functionName The name of the calling function for debug output.
 * @return True if the request was successful (HTTP code > 0), false otherwise.
 */
static bool sendHttpPostStartMatch(const String &jsonPayload, const String &functionName) {
    if (!eth_connected) {
        Serial.println("Network not connected! [" + functionName + "]");
        return false;
    }

    HTTPClient http;
    String url = String(baseUrl) + START_MATCH_ENDPOINT;

    // Configure HTTP request
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Send the request
    int httpResponseCode = http.POST(jsonPayload);

    // Handle the response
    if (httpResponseCode > 0) {
        if (printSerialDebug) {
            Serial.println(functionName);
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        }
    } else {
        Serial.println(functionName);
        Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
    }

    http.end();
    return httpResponseCode > 0;
}

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
    sendHttpPostStartMatch
(jsonPayload, "StartMatch.h");
}

#endif // STARTMATCH_H