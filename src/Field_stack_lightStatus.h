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
#ifndef FIELDSTACKLIGHTSTATUS_H
#define FIELDSTACKLIGHTSTATUS_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#define FASTLED_INTERNAL // Suppress build banner
#include <FastLED.h>
#include "GlobalSettings.h"

// extern Adafruit_NeoPixel strip;

extern const char *baseUrl;
extern bool eth_connected;
extern String deviceRole;
extern String arenaIP;
extern String arenaPort;

extern CRGB g_LEDs[]; // Declare the LED array

/**
 * @brief Sets the color of two LEDs based on the status.
 *
 * @param ledIndex1 The index of the first LED.
 * @param ledIndex2 The index of the second LED.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
void setLEDColor(int ledIndex1, int length, bool status, CRGB color)
{
    if (status)
    {
        for (int i = ledIndex1; i < ledIndex1 + length; i++)
        {
            g_LEDs[i] = color; // Set LED to color
        }
    }
    else
    {
        for (int i = ledIndex1; i < ledIndex1 + length; i++)
        {
            g_LEDs[i] = CRGB::Black; // Turn off the LED
        }
    }
}

const CRGB RED_COLOR = CRGB(255, 0, 0);
const CRGB BLUE_COLOR = CRGB(0, 0, 255);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR = CRGB(0, 255, 0);
const CRGB WHITE_COLOR = CRGB(20, 20, 20);
int heartbeatState = 0;
long int hartBeatTck = 0;
long int currentTime = 0;

void getField_stack_lightStatus()
{
    long int currentTime = millis();
    if (eth_connected)
    {
        // if(currentTime > hartBeatTck){  //Get the status every 500ms
        // hartBeatTck = currentTime + 500;

        HTTPClient http;
        String url = "http://" + arenaIP + ":" + arenaPort + "/api/freezy/field_stack_light";
        Serial.println("URL: " + url); // Print the URL
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.println("GetField_stack_lightStatus");
            Serial.printf("Request successful! HTTP code: %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println("Response:");
            Serial.println(response);

            // Parse and print JSON data
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, response);
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                return;
            }

            // Set the LED color based on the field_stack_light status
            bool redStackLightStatus = doc.containsKey("redStackLight") ? doc["redStackLight"].as<bool>() : false;
            bool blueStackLightStatus = doc.containsKey("blueStackLight") ? doc["blueStackLight"].as<bool>() : false;
            bool orangeStackLightStatus = doc.containsKey("orangeStackLight") ? doc["orangeStackLight"].as<bool>() : false;
            bool greenStackLightStatus = doc.containsKey("greenStackLight") ? doc["greenStackLight"].as<bool>() : false;
            if (deviceRole == "FMS_TABLE")
            {
                setLEDColor(2, 60, redStackLightStatus, RED_COLOR);         // Red
                setLEDColor(60, 60, blueStackLightStatus, BLUE_COLOR);      // Blue
                setLEDColor(120, 60, orangeStackLightStatus, ORANGE_COLOR); // Orange
                setLEDColor(180, 56, greenStackLightStatus, GREEN_COLOR);   // Green
            }
            else if (deviceRole == "RED_ALLIANCE")
            {
                setLEDColor(1, 1, true, RED_COLOR);  // RED
                setLEDColor(2, 8, false, RED_COLOR); // RED
            }
            else if (deviceRole == "BLUE_ALLIANCE")
            {
                setLEDColor(1, 1, true, BLUE_COLOR);  // BLUE
                setLEDColor(2, 8, false, BLUE_COLOR); // RED
            }
            else
            {
                setLEDColor(1, 1, true, ORANGE_COLOR); // ORANGE
            }

            // Print the JSON data
            // serializeJsonPretty(doc, Serial);
            // heartbeat = !heartbeat; // Toggle heartbeat
            // setLEDColor(1, 1, heartbeat, WHITE_COLOR); // White
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
            Serial.println("getField_stack_lightStatus");
            Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
            // heartbeat = !heartbeat; // Toggle heartbeat
            // setLEDColor(1, 1, heartbeat, ORANGE_COLOR); // Orange
            if (heartbeatState == 0)
            {
                heartbeatState = 2;
            }
            else
            {
                heartbeatState = 0;
            }
        }
        http.end();
        //} //Get the status every 500ms
    }
    else
    {
        Serial.println("Network not connected! [FSL]");
        // heartbeat = !heartbeat; // Toggle heartbeat
        // setLEDColor(1, 1, heartbeat, ORANGE_COLOR); // Red
        if (heartbeatState == 0)
        {
            heartbeatState = 3;
        }
        else
        {
            heartbeatState = 0;
        }
    }
}

#endif // FIELDSTACKLIGHTSTATUS_H