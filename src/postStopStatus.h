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
 * @param i The channel number.
 * @param stopButtonPressed The state of the stop button (false if pressed, true otherwise).
 */
void postSingleStopStatus(int i, bool stopButtonPressed)
{
    // Send the HTTP POST request
    if (eth_connected)
    {
        HTTPClient http;

        // Define payload
        StaticJsonDocument<200> payload;
        JsonArray array = payload.to<JsonArray>();

        JsonObject channel = array.createNestedObject();
        if (deviceRole == "FMS_TABLE")
        {
            channel["channel"] = 0; // Only Channel 0 is used for Field
            channel["state"] = stopButtonPressed;
        }
        else if (deviceRole == "RED_ALLIANCE")
        {
            channel["channel"] = i;
            channel["state"] = stopButtonPressed;
        }
        else if (deviceRole == "BLUE_ALLIANCE")
        {
            channel["channel"] = i + 6;
            channel["state"] = stopButtonPressed;
        }

        String jsonPayload;
        serializeJson(payload, jsonPayload);

        // Configure HTTP POST request
        String url = String(baseUrl) + "/api/freezy/eStopState";
        Serial.println("URL: " + url); // Print the URL
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        // Send the request
        int httpResponseCode = http.POST(jsonPayload);

        // Handle the response
        if (httpResponseCode > 0)
        {
            Serial.println("PostSingleStopStatus");
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);
        }
        else
        {
            Serial.println("SingleStopStatus");
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

/**
 * Sends an HTTP POST request to update the stop status for all 6 channels.
 *
 * @param stopButtonStates An array of the states of the stop buttons (false if pressed, true otherwise).
 */
void postAllStopStatus(bool stopButtonStates[6], int startingChannel)
{
    // Send the HTTP POST request
    if (eth_connected)
    {
        HTTPClient http;

        // Define payload
        StaticJsonDocument<200> payload;
        JsonArray array = payload.to<JsonArray>();
       
        for (int i = 0; i < 6; i++){
            JsonObject channel = array.createNestedObject();
            channel["channel"] = i + startingChannel;
            channel["state"] = stopButtonStates[i];
            Serial.println("Channel: " + String(i + startingChannel) + " State: " + String(stopButtonStates[i]));
        }

        // Convert payload to JSON string
        String jsonString;
        serializeJson(payload, jsonString);

        // Configure HTTP POST request
        String url = String(baseUrl) + "/api/freezy/eStopState";
        Serial.println("URL: " + url); // Print the URL
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        // Send the request
        int httpResponseCode = http.POST(jsonString);

        // Handle the response
        if (httpResponseCode > 0)
        {
            Serial.println("postAllStopStatus");
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);

            if (heartbeatState == 0)
            {
                heartbeatState = 1;
            }
            else
            {
                heartbeatState = 0;
            }
        }
        else
        {
            Serial.println("postAllStopStatus");
            Serial.printf("Request failed! Error code: %d\n", httpResponseCode);
            if (heartbeatState == 0)
            {
                heartbeatState = 2;
            }
            else
            {
                heartbeatState = 0;
            }
        }

        // Close the connection
        http.end();
    }
    else
    {
        Serial.println("Network not connected![PSS]");
    }
}

#endif // POSTSTOPSTATUS_H