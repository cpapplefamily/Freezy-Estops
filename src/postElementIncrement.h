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

// Constants
const char *ELEMENT_INCREMENT_ENDPOINT = "/freezy/alternateio/increment";

/**
 * Sends an HTTP POST request to update the stop status.
 *
 * @param alliance
 * @param element
 */
void postElement(String alliance, String element)
{
    // Create JSON payload
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonObject channel = payload.to<JsonObject>();
    channel["alliance"] = alliance;
    channel["element"] = element;

    String jsonPayload;
    serializeJson(payload, jsonPayload);

    // Send the request
    sendHttpPost(ELEMENT_INCREMENT_ENDPOINT, jsonPayload, "postElement", false);
}

#endif // POSTELEMENTINCREMENT_H