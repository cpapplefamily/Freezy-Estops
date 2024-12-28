#ifndef WEBSERVERSETUP_H
#define WEBSERVERSETUP_H

#include <ESPAsyncWebServer.h>
#include <Preferences.h>

// Create an instance of the web server
extern AsyncWebServer server;

// Create a Preferences object
extern Preferences preferences;

void setupWebServer();

#endif // WEBSERVERSETUP_H