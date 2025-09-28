#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>

extern int heartbeatState;             // Heartbeat state for system status
extern bool printSerialDebug;          // Flag to enable/disable debug logging

// Shared Constants
const size_t JSON_CAPACITY = 200;      // Capacity for StaticJsonDocument in HTTP requests

// Declare the deviceRole variable
String deviceRole;

// Declare the useDHCP variable
bool useDHCP;

// Declare the deviceIP variable
String deviceIP;
String deviceGWIP;

// LED Colors
const CRGB RED_COLOR    = CRGB(255, 0, 0);
const CRGB BLUE_COLOR   = CRGB(0, 0, 255);
const CRGB VIOLET_COLOR = CRGB(255, 0, 255);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR  = CRGB(0, 255, 0);
const CRGB WHITE_COLOR  = CRGB(20, 20, 20);

// Helper Functions
/**
 * Sends an HTTP POST request with the given JSON payload to the specified endpoint.
 * @param jsonPayload The JSON string to send.
 * @param endpoint The API endpoint (e.g., "/api/freezy/eStopState").
 * @param functionName The name of the calling function for debug output.
 * @param updateHeartbeat Whether to update heartbeatState (true for eStop, false for others).
 * @return True if the request was successful (HTTP code > 0), false otherwise.
 */
static bool sendHttpPost_Function(const String &endpoint, const String &jsonPayload, const String &functionName, bool updateHeartbeat) {
    if (!eth_connected) {
        Serial.println("Network not connected! [" + functionName + "]");
        if (updateHeartbeat) {
            heartbeatState = (heartbeatState == 0) ? 3 : 0;
        }
        return false;
    }

    HTTPClient http;
    String url = String(baseUrl) + endpoint;

    if (printSerialDebug) {
        Serial.println("URL: " + url);
    }

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonPayload);

    // Handle the response
        if (httpResponseCode > 0) {
            if (printSerialDebug) {
                Serial.println("postAllStopStatus");
                Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
                String response = http.getString();
                Serial.println("Response:");
                Serial.println(response);
            }
            heartbeatState = 1;
        } else {
            Serial.println("postAllStopStatus");
            Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
            heartbeatState = 2;
        }

    http.end();
    return httpResponseCode > 0;
}

#endif // GLOBALSETTINGS_H