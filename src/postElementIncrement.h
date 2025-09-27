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

#ifndef POSTELEMENTINCREMENT_H
#define POSTELEMENTINCREMENT_H

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
const char *ELEMENT_INCREMENT_ENDPOINT = "/freezy/alternateio/increment";

// Helper Functions
/**
 * Sends an HTTP POST request with the given JSON payload to the specified endpoint.
 * @param jsonPayload The JSON string to send.
 * @param functionName The name of the calling function for debug output.
 * @return True if the request was successful (HTTP code > 0), false otherwise.
 */
static bool sendHttpPost3(const String &jsonPayload, const String &functionName) {
    if (!eth_connected) {
        Serial.println("Network not connected! [" + functionName + "]");
        return false;
    }

    HTTPClient http;
    String url = String(baseUrl) + ELEMENT_INCREMENT_ENDPOINT;

    // Configure HTTP request
    if (printSerialDebug) {
        Serial.println("URL: " + url);
    }
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    // Send the request
    int httpResponseCode = http.POST(jsonPayload);

    // Handle the response
    if (httpResponseCode > 0) {
        if (printSerialDebug) {Serial.println(functionName);
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
 * Sends an HTTP POST request to increment an element count for the specified alliance.
 * @param alliance The alliance identifier (e.g., "red", "blue").
 * @param element The element to increment (e.g., "ProcessorAlgae").
 */
void postElement(String alliance, String element) {
    // Create JSON payload
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonObject channel = payload.to<JsonObject>();
    channel["alliance"] = alliance;
    channel["element"] = element;

    String jsonPayload;
    serializeJson(payload, jsonPayload);

    // Send the request
    sendHttpPost3(jsonPayload, "postElement");
}

#endif // POSTELEMENTINCREMENT_H