#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>

// Declare the deviceRole variable
String deviceRole;

// Declare the useDHCP variable
bool useDHCP;

// Declare the deviceIP variable
String deviceIP;
String deviceGWIP;

// LED Colors
const CRGB RED_COLOR    = CRGB(255, 0, 0);
const CRGB BLUE_COLOR   = CRGB(0, 0, 255);
const CRGB VIOLET_COLOR = CRGB(255, 0, 255);
const CRGB ORANGE_COLOR = CRGB(150, 100, 0);
const CRGB GREEN_COLOR  = CRGB(0, 255, 0);
const CRGB WHITE_COLOR  = CRGB(20, 20, 20);

#endif // GLOBALSETTINGS_H