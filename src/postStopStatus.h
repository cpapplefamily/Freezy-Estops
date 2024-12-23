#ifndef POSTSTOPSTATUS_H
#define POSTSTOPSTATUS_H


#include <HTTPClient.h>
#include <ArduinoJson.h>


extern const char* baseUrl;

/**
 * Sends an HTTP POST request to update the stop status.
 * 
 * @param i The channel number.
 * @param stopButtonPressed The state of the stop button (false if pressed, true otherwise).
 */
void postStopStatus(int i, bool stopButtonPressed) {
    // Send the HTTP POST request
    if (eth_connected) {
        HTTPClient http;

        // Define payload
        StaticJsonDocument<200> payload;
        JsonArray array = payload.to<JsonArray>();
        
        JsonObject channel = array.createNestedObject();
        channel["channel"] = i;
        channel["state"] = stopButtonPressed;

        String jsonPayload;
        serializeJson(payload, jsonPayload);

        // Configure HTTP POST request
        String url = String(baseUrl) + "/freezy/eStopState";
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        // Send the request
        int httpResponseCode = http.POST(jsonPayload);

        // Handle the response
        if (httpResponseCode > 0) {
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        } else {
            Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
        }

        // Close the connection
        http.end();
    } else {
        Serial.println("Network not connected!");
    }
}

#endif // POSTSTOPSTATUS_H