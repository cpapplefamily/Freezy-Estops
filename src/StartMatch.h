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
#include "GlobalSettings.h"

// Constants
const char *START_MATCH_ENDPOINT = "/api/freezy/startMatch";

void startMatchPost()
{
    // Create JSON payload
    StaticJsonDocument<JSON_CAPACITY> payload;
    JsonObject channel = payload.to<JsonObject>();
    channel["match"] = "start";

    String jsonPayload;
    serializeJson(payload, jsonPayload);

    // Send the request
    sendHttpPost(START_MATCH_ENDPOINT, jsonPayload, "postElement", false);
}

#endif // STARTMATCH_H