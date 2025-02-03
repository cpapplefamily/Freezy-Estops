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
#include <Adafruit_NeoPixel.h>
#include "GlobalSettings.h"

extern Adafruit_NeoPixel strip;

extern const char* baseUrl;
extern bool eth_connected;
extern String allianceColor;
extern String arenaIP;
extern String arenaPort;

/**
 * @brief Sets the color of two LEDs based on the status.
 * 
 * @param ledIndex1 The index of the first LED.
 * @param ledIndex2 The index of the second LED.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
void setLEDColor(int ledIndex1, int length, bool status, uint32_t color) {
    if (status) {
        for (int i = ledIndex1; i < ledIndex1 + length; i++) {
            strip.setPixelColor(i, color); // Set LED to color
        }
    } else {
        for (int i = ledIndex1; i < ledIndex1 + length; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // Set LED to color
        }
    }
}

const uint32_t RED_COLOR = strip.Color(255, 0, 0);
const uint32_t BLUE_COLOR = strip.Color(0, 0, 255);
const uint32_t ORANGE_COLOR = strip.Color(150, 100, 0);
const uint32_t GREEN_COLOR = strip.Color(0, 255, 0);
const uint32_t WHITE_COLOR = strip.Color(20, 20, 20);
bool heartbeat = false;

void getField_stack_lightStatus() {
    if (eth_connected) {
        HTTPClient http;
        String url = "http://" + arenaIP + ":" + arenaPort + "/field_stack_light";
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String response = http.getString();
            //Serial.printf("GET request successful! HTTP code: %d\n", httpResponseCode);
            //Serial.println("Response:");
            //Serial.println(response);

            // Parse and print JSON data
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                return;
            }

            // Set the LED color based on the field_stack_light status
            bool redStackLightStatus = doc.containsKey("redStackLight") ? doc["redStackLight"].as<bool>() : false;
            bool blueStackLightStatus = doc.containsKey("blueStackLight") ? doc["blueStackLight"].as<bool>() : false;
            bool orangeStackLightStatus = doc.containsKey("orangeStackLight") ? doc["orangeStackLight"].as<bool>() : false;
            bool greenStackLightStatus = doc.containsKey("greenStackLight") ? doc["greenStackLight"].as<bool>() : false;
            if (allianceColor == "Field") {
                setLEDColor(1, 2, redStackLightStatus, RED_COLOR); // Red
                setLEDColor(3, 2, blueStackLightStatus, BLUE_COLOR); // Blue
                setLEDColor(5, 2, orangeStackLightStatus, ORANGE_COLOR); // Orange
                setLEDColor(7, 2, greenStackLightStatus, GREEN_COLOR); // Green
            } else if (allianceColor == "Red") {
                setLEDColor(1, 1, true, RED_COLOR); // RED
                setLEDColor(2, 8, false, RED_COLOR); // RED
            } else if (allianceColor == "Blue") {
                setLEDColor(1, 1, true, BLUE_COLOR); // BLUE
                setLEDColor(2, 8, false, BLUE_COLOR); // RED
            } else{
                setLEDColor(1, 1, true, ORANGE_COLOR); // ORANGE
            }
            

            // Print the JSON data
            //serializeJsonPretty(doc, Serial);
            heartbeat = !heartbeat; // Toggle heartbeat
            setLEDColor(0, 1, heartbeat, WHITE_COLOR); // Orange
            strip.show();
        } else {
            Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
            heartbeat = !heartbeat; // Toggle heartbeat
            setLEDColor(0, 1, heartbeat, ORANGE_COLOR); // Orange
            strip.show();
        }
        http.end();
    } else {
        Serial.println("Network not connected! [FSL]");
        heartbeat = !heartbeat; // Toggle heartbeat
        setLEDColor(0, 1, heartbeat, RED_COLOR); // Orange
        strip.show();
    }
}

#endif // FIELDSTACKLIGHTSTATUS_H