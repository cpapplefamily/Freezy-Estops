#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Declare the deviceRole variable
String deviceRole;

// Declare the arenaIP variable
String arenaIP;

// Declare the arenaPort variable
String arenaPort;

// Declare the useDHCP variable
bool useDHCP;

// Declare the deviceIP variable
String deviceIP;
String deviceGWIP;

// Shared Variables
extern Preferences preferences;        // Persistent storage for settings
//extern String deviceRole;              // Device role (e.g., FMS_TABLE, RED_ALLIANCE, BLUE_ALLIANCE, BARGE_LIGHTS)
//extern String deviceIP;                // Device IP address
extern String deviceGWIP;              // Gateway IP address
//extern bool useDHCP;                   // Flag to use DHCP or static IP
//extern String arenaIP;                 // Arena server IP address
//extern String arenaPort;               // Arena server port
extern const char *baseUrl;            // Base URL for HTTP requests
extern bool eth_connected;             // Network connection status
extern int heartbeatState;             // Heartbeat state for system status
extern bool printSerialDebug;          // Flag to enable/disable debug logging

// Shared Constants
const size_t JSON_CAPACITY = 200;      // Capacity for StaticJsonDocument in HTTP requests

// Helper Functions
/**
 * Sends an HTTP POST request with the given JSON payload to the specified endpoint.
 * @param jsonPayload The JSON string to send.
 * @param endpoint The API endpoint (e.g., "/api/freezy/eStopState").
 * @param functionName The name of the calling function for debug output.
 * @param updateHeartbeat Whether to update heartbeatState (true for eStop, false for others).
 * @return True if the request was successful (HTTP code > 0), false otherwise.
 */
static bool sendHttpPost(const String &endpoint, const String &jsonPayload, const String &functionName, bool updateHeartbeat) {
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

    if (httpResponseCode > 0) {
        if (printSerialDebug) {
            Serial.println(functionName);
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        }
        if (updateHeartbeat) {
            heartbeatState = (heartbeatState == 0) ? 1 : 0;
        }
    } else {
        Serial.println(functionName);
        Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
        if (updateHeartbeat) {
            heartbeatState = (heartbeatState == 0) ? 2 : 0;
        }
    }

    http.end();
    return httpResponseCode > 0;
}

/**
 * Sends an HTTP GET request to the specified endpoint and returns the JSON response.
 * @param endpoint The API endpoint (e.g., "/api/freezy/field_stack_light").
 * @param functionName The name of the calling function for debug output.
 * @param doc The JSON document to store the parsed response.
 * @return True if the request was successful and JSON parsed correctly, false otherwise.
 */
static bool sendHttpGet(const String &endpoint, const String &functionName, StaticJsonDocument<JSON_CAPACITY> &doc) {
    if (!eth_connected) {
        Serial.println("Network not connected! [FSL]");
        heartbeatState = (heartbeatState == 0) ? 3 : 0;
        return false;
    }

    HTTPClient http;
    String url = "http://" + arenaIP + ":" + arenaPort + endpoint;

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

#endif // GLOBALSETTINGS_H