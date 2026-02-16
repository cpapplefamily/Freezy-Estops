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
#ifndef BLINKSTATE_H
#define BLINKSTATE_H

#include <Arduino.h>

// Blink state variables (defined in BlinkState.h as extern)
boolean ledBlinkState = true;
long lastLedBlinkTime = 0;

inline void updateBlinkState(int rate_ms) {
    long t = millis();
    long elapsedTimeMS = t - lastLedBlinkTime;
    if (elapsedTimeMS > rate_ms) {
        ledBlinkState = !ledBlinkState;
        lastLedBlinkTime = t;
    }
}

#endif // BLINKSTATE_H
