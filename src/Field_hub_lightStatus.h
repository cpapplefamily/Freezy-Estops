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

#ifndef FIELDHUBLIGHTSTATUS_H
#define FIELDHUBLIGHTSTATUS_H
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "GlobalSettings.h"
#include "ColorUtils.h"
#include "BlinkState.h"

extern const char *baseUrl;
//extern bool net_connected;
extern String allianceColor;
extern String arenaIP;
extern String arenaPort;

//CRGB g_LEDs[NUM_LEDS] = {0};


/* void setupLEDs() {
  // The 12v stack light strip that has 3-LEDs per position.
  // LEDSTRIP macro must be defined before including this header
  FastLED.addLeds<WS2811, LEDSTRIP, BRG>(g_LEDs, NUM_LEDS);
  FastLED.setTemperature(Tungsten100W);
} */

void setHubLight(CRGB rgbColor, boolean blink, int blinkRateMS)
{
    updateBlinkState(blinkRateMS);

    if(blink && !ledBlinkState) {
        rgbColor=CRGB::Black;
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
        g_LEDs[i] = rgbColor;
    }
    FastLED.show();
}

void setHubLight(CRGB rgbColor, boolean blink) {
    setHubLight(rgbColor, blink, 250);
}

void setHubLight(CRGB rgbColor) {
    setHubLight(rgbColor, false, 0);
}

// Example payload from FMS:
// {
//      "red": {
// 	       "color": "black",
// 	        "blink": false,
//          "rgb": {
//              "r": 0,
//	            "g": 0,
//			    "b": 0
//		}
// 	},
// 	{
//      "blue": {
// 	        "color": "red",
// 	        "blink": true
// 	    }
// 	}

static long lastPollTimeMS;
static const int pollIntervalMS = 200;
void updateHub_lightStatus()
{
    // long int currentTime = millis();
    if (true)
    {
        long currentTime = millis();
        if(currentTime-lastPollTimeMS >= pollIntervalMS) {
          lastPollTimeMS = currentTime;

            HTTPClient http;
            String url = "http://" + arenaIP + ":" + arenaPort + "/api/freezy/hub_status?alliance=" + allianceColor;
            http.setTimeout(1000);
            http.setConnectTimeout(1000);
            http.begin(url);
            int httpResponseCode = http.GET();

            if (httpResponseCode == HTTP_CODE_OK)
            {
                String response = http.getString();
                // Serial.printf("GET request successful! HTTP code: %d\n", httpResponseCode);
                // Serial.println("Response:");
                // Serial.println(response);

                // Parse and print JSON data
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, response);
                if (error) {
                    Serial.print("deserializeJson() failed: ");
                    Serial.println(error.f_str());
                    // Blink the error condition
                    setHubLight(CRGB::Yellow, true, 250);
                    return;
                }
                String allianceColorLower = allianceColor;
                allianceColorLower.toLowerCase();

                // Return an array of objects, 1 per hub (2 of them)
                JsonObject hubState = doc[allianceColorLower];
                bool blink = hubState["blink"].as<bool>();
                String colorString = hubState["color"].as<String>();
                CRGB color;
                if (colorString.length() != 0) {
                    color = toRGBColor(colorString);
                } else {
                    // Fallback if no color string, use the RGB value
                    JsonObject colorRGB = hubState["rgb"];
                    color = CRGB(colorRGB["r"], colorRGB["g"], colorRGB["b"]);
                }
                setHubLight(color, blink);
            } else {
                Serial.printf("GET request failed! HTTP code: %d\n", httpResponseCode);
                // Blink the error condition
                setHubLight(CRGB::Yellow, true, 250);
            }
        }
    }
    else
    {
        // Blink the error condition
        setHubLight(CRGB::Yellow, true, 250);
    }
}
#endif