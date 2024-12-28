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

#include <HTTPClient.h>
#include <ArduinoJson.h>


extern const char* baseUrl;
extern bool eth_connected;

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