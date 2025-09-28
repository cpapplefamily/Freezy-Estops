#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>

extern int heartbeatState;    // Heartbeat state for system status
extern bool printSerialDebug; // Flag to enable/disable debug logging
extern const char *baseUrl;
extern bool eth_connected;
extern CRGB g_LEDs[]; // Declare the LED array

// Shared Constants
const size_t JSON_CAPACITY = 200; // Capacity for StaticJsonDocument in HTTP requests

// Declare the deviceRole variable
String deviceRole;

// Declare the useDHCP variable
bool useDHCP;

// Declare the deviceIP variable
String deviceIP;
String deviceGWIP;

// LED Colors
const CRGB RED_COLOR = CRGB(255, 0, 0);
const CRGB LTRED_COLOR = CRGB(50, 0, 0);
const CRGB BLUE_COLOR = CRGB(0, 0, 255);
const CRGB LTBLUE_COLOR = CRGB(0, 0, 50);
const CRGB VIOLET_COLOR = CRGB(255, 0, 255);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR = CRGB(0, 255, 0);
const CRGB SPRINGGREEN_COLOR = CRGB(0, 63, 40); // SpringGreen=0x00FF7F
const CRGB LTGREEN_COLOR = CRGB(0, 50, 0);
const CRGB WHITE_COLOR = CRGB(255, 255, 255);
const CRGB LTWHITE_COLOR = CRGB(50, 50, 50);

// LED Configuration
#ifdef ESP32_S3_DEVKITM_1
const uint8_t NUM_LED_STRIPS = 5;
const uint8_t NUM_LEDS_PER_STRIP = 5;
const uint8_t NUM_LEDS_PER_METER = 30;
#define NUM_LEDS (NUM_LED_STRIPS * NUM_LEDS_PER_STRIP * NUM_LEDS_PER_METER) // 750
#endif

// Helper Functions

/**
 * @brief Sets the color of two LEDs based on the status.
 *
 * @param ledIndex1 The index of the first LED.
 * @param ledIndex2 The index of the second LED.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
static void setLEDColor(int ledIndex1, int length, bool status, CRGB color) {
    if (ledIndex1 + length > NUM_LEDS) {
        length = NUM_LEDS - ledIndex1; // Adjust length to fit within bounds
    }
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
        heartbeatState = 3;
        return false;
    }

    HTTPClient http;
    String url = String(baseUrl) + endpoint;

    if (printSerialDebug) {
        Serial.println("URL: " + url);
    }

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String response = http.getString();
        if (printSerialDebug)
        {
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
        heartbeatState = 1;
    } else {
        Serial.println(functionName);
        Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
        heartbeatState = 2;
        http.end();
        return false;
    }

    http.end();
    return true;
}

#endif // GLOBALSETTINGS_H