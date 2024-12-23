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

#ifndef ALLIANCESTATUS_H
#define ALLIANCESTATUS_H

#include <HTTPClient.h>
#include <ArduinoJson.h>

extern const char* baseUrl;

void getAllianceStatus() {
    if (eth_connected) {
        HTTPClient http;
        String url = String(baseUrl) + "/api/allianceStatus";
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.printf("GET request successful! HTTP code: %d\n", httpResponseCode);
            Serial.println("Response:");
            Serial.println(response);

            // Parse and print JSON data
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                return;
            }
            serializeJsonPretty(doc, Serial);
        } else {
            Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Network not connected!");
    }
}

#endif // ALLIANCESTATUS_H