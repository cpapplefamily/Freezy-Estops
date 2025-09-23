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

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "GlobalSettings.h"

extern const char *baseUrl;
extern bool eth_connected;
extern String deviceRole;
extern int heartbeatState;

/**
 * Sends an HTTP POST request to update the stop status.
 *
 * @param alliance
 * @param element
 */
void postElement(String alliance, String element)
{
    // Send the HTTP POST request
    if (eth_connected)
    {
        HTTPClient http;

        // Define payload
        StaticJsonDocument<200> payload;
        JsonObject channel = payload.to<JsonObject>();
        channel["alliance"] = alliance;
        channel["element"] = element;

        String jsonPayload;
        serializeJson(payload, jsonPayload);

        // Configure HTTP POST request
        String url = String(baseUrl) + "/freezy/alternateio/increment";
        Serial.println("URL: " + url); // Print the URL
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        // Send the request
        int httpResponseCode = http.POST(jsonPayload);

        // Handle the response
        if (httpResponseCode > 0)
        {
            Serial.println("postElement");
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        }
        else
        {
            Serial.println("postElement");
            Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
        }

        // Close the connection
        http.end();
    }
    else
    {
        Serial.println("Network not connected![PSS]");
    }
}

#endif // POSTELEMENTINCREMENT_H