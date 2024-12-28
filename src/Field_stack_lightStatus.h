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

extern Adafruit_NeoPixel strip;

extern const char* baseUrl;
extern bool eth_connected;

/**
 * @brief Sets the color of two LEDs based on the status.
 * 
 * @param ledIndex1 The index of the first LED.
 * @param ledIndex2 The index of the second LED.
 * @param status The status indicating whether to turn on or off the LEDs.
 * @param color The color to set the LEDs to if the status is true.
 */
void setLEDColor(int ledIndex1, int ledIndex2, bool status, uint32_t color) {
    if (status) {
        strip.setPixelColor(ledIndex1, color); // Set first LED to color
        strip.setPixelColor(ledIndex2, color); // Set second LED to color
    } else {
        strip.setPixelColor(ledIndex1, strip.Color(0, 0, 0)); // Turn off first LED
        strip.setPixelColor(ledIndex2, strip.Color(0, 0, 0)); // Turn off second LED
    }
}

const uint32_t RED_COLOR = strip.Color(255, 0, 0);
const uint32_t BLUE_COLOR = strip.Color(0, 0, 255);
const uint32_t ORANGE_COLOR = strip.Color(100, 100, 0);
const uint32_t GREEN_COLOR = strip.Color(0, 255, 0);

void getField_stack_lightStatus() {
    if (eth_connected) {
        HTTPClient http;
        String url = String(baseUrl) + "/field_stack_light";
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
            setLEDColor(0, 1, redStackLightStatus, RED_COLOR); // Red
            setLEDColor(2, 3, blueStackLightStatus, BLUE_COLOR); // Blue
            setLEDColor(4, 5, orangeStackLightStatus, ORANGE_COLOR); // Orange
            setLEDColor(6, 7, greenStackLightStatus, GREEN_COLOR); // Green
            strip.show();

            // Print the JSON data
            //serializeJsonPretty(doc, Serial);
        } else {
            Serial.printf("GET request failed! Error code: %d\n", httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Network not connected!");
    }
}

#endif // FIELDSTACKLIGHTSTATUS_H