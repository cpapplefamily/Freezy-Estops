#ifndef STARTMATCH_H
#define STARTMATCH_H


#include <HTTPClient.h>
#include <ArduinoJson.h>


extern const char* baseUrl;

void startMatchPost() {
    if (eth_connected) {
        HTTPClient http;

        // Define payload
        StaticJsonDocument<200> payload;
        payload["match"] = "start";
        String jsonPayload;
        serializeJson(payload, jsonPayload);

        // Configure HTTP POST request
        String url = String(baseUrl) + "/freezy/startMatch";
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

#endif // STARTMATCH_H