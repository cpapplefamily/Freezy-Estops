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

// External Variables
extern const char *baseUrl;
extern bool eth_connected;
extern String deviceRole;
extern int heartbeatState;
extern bool printSerialDebug;

// Constants
const char *STOP_STATUS_ENDPOINT = "/api/freezy/eStopState";

// Helper Functions
/**
 * Sends an HTTP POST request with the given JSON payload to the stop status endpoint.
 * @param jsonPayload The JSON string to send.
 * @param functionName The name of the calling function for debug output.
 * @return True if the request was successful (HTTP code > 0), false otherwise.
 */
static bool sendHttpPost1(const String &jsonPayload, const String &functionName) {
    if (!eth_connected) {
        Serial.println("Network not connected! [" + functionName + "]");
        return false;
    }

    HTTPClient http;
    String url = String(baseUrl) + STOP_STATUS_ENDPOINT;
    
    if (printSerialDebug) {
        Serial.println("URL: " + url);
    }

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
        if (printSerialDebug) {
            Serial.println(functionName);
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        }
        // Toggle heartbeat state on success
        heartbeatState = (heartbeatState == 0) ? 1 : 0;
    } else {
        Serial.println(functionName);
        Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
        // Set heartbeat state to error or reset
        heartbeatState = (heartbeatState == 0) ? 2 : 0;
    }

    http.end();
    return httpResponseCode > 0;
}

/**
 * Sends an HTTP POST request to update the stop status for a single channel.
 * @param i The channel index (adjusted based on deviceRole).
 * @param stopButtonPressed The state of the stop button (false if pressed, true otherwise).
 */
void postSingleStopStatus(int i, bool stopButtonPressed) {
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonArray array = payload.to<JsonArray>();
    JsonObject channel = array.createNestedObject();

    // Map channel based on device role
    if (deviceRole == "FMS_TABLE") {
        channel["channel"] = 0; // Field stop always uses channel 0
    } else if (deviceRole == "RED_ALLIANCE") {
        channel["channel"] = i;
    } else if (deviceRole == "BLUE_ALLIANCE") {
        channel["channel"] = i + 6; // Offset by 6 for Blue Alliance
    }
    channel["state"] = stopButtonPressed;

    String jsonPayload;
    serializeJson(payload, jsonPayload);
    sendHttpPost1(jsonPayload, "PostSingleStopStatus");
}

/**
 * Sends an HTTP POST request to update the stop status for all 6 channels.
 * @param stopButtonStates Array of stop button states (false if pressed, true otherwise).
 * @param startingChannel The starting channel number (1 for RED_ALLIANCE, 7 for BLUE_ALLIANCE).
 */
void postAllStopStatus(bool stopButtonStates[6], int startingChannel) {
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonArray array = payload.to<JsonArray>();

    // Create payload for all channels
    for (int i = 0; i < 6; i++) {
        JsonObject channel = array.createNestedObject();
        channel["channel"] = i + startingChannel;
        channel["state"] = stopButtonStates[i];
    }

    String jsonPayload;
    serializeJson(payload, jsonPayload);
    sendHttpPost1(jsonPayload, "postAllStopStatus");
}

#endif // POSTSTOPSTATUS_H