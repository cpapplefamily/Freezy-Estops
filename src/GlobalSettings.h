#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>

// Declare the deviceRole variable
String deviceRole;

// Declare the arenaIP variable
String arenaIP;

// Declare the arenaPort variable
String arenaPort;

// Declare the useDHCP variable
bool useDHCP;

// Declare the deviceIP variable
String deviceIP;
String deviceGWIP;

// Shared Constants
const size_t JSON_CAPACITY = 200;      // Capacity for StaticJsonDocument in HTTP requests


#endif // GLOBALSETTINGS_H