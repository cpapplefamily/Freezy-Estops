#include "WebServerSetup.h"

// Define the web server and preferences objects
AsyncWebServer server(80);
Preferences preferences;
String allianceColor;

void setupWebServer() {
    // Load the alliance color from preferences
    preferences.begin("settings", false);
    allianceColor = preferences.getString("allianceColor", "Red");

    // Load IP address and DHCP/Static configuration from preferences
    String ipAddress = preferences.getString("ipAddress", "");
    bool useDHCP = preferences.getBool("useDHCP", true);

    // Set up the web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        // Serve the initial page with a button to go to the settings page
        String html = "<html><body>"
                      "<h1>Welcome to Freezy Estops</h1>"
                      "<p>This is the initial page.</p>"
                      "<button onclick=\"location.href='/setup'\">Go to Settings</button>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/setup", HTTP_GET, [ipAddress, useDHCP](AsyncWebServerRequest *request){
        // Serve the HTML form for alliance color and network configuration
        String html = "<html><body>"
                      "<h1>Setup Configuration</h1>"
                      "<form action=\"/setConfig\" method=\"POST\">"
                      "<label for=\"color\">Choose an alliance color:</label>"
                      "<select name=\"color\" id=\"color\">"
                       "<option value=\"Red\"" + String(allianceColor == "Red" ? " selected" : "") + ">Red</option>"
                      "<option value=\"Blue\"" + String(allianceColor == "Blue" ? " selected" : "") + ">Blue</option>"
                      "<option value=\"Field\"" + String(allianceColor == "Field" ? " selected" : "") + ">Field</option>"
                      "</select><br><br>"
                      "<label for=\"ip\">IP Address:</label>"
                      "<input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ipAddress + "\"><br><br>"
                      "<input type=\"checkbox\" id=\"dhcp\" name=\"dhcp\" " + String(useDHCP ? "checked" : "") + ">"
                      "<label for=\"dhcp\">Use DHCP</label><br><br>"
                      "<input type=\"submit\" value=\"Submit\">"
                      "</form>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/setConfig", HTTP_POST, [](AsyncWebServerRequest *request){
        // Handle the form submission and update the configuration
        if (request->hasParam("color", true)) {
            allianceColor = request->getParam("color", true)->value();
            preferences.putString("allianceColor", allianceColor);
        }
        if (request->hasParam("ip", true)) {
            String ipAddress = request->getParam("ip", true)->value();
            preferences.putString("ipAddress", ipAddress);
        }
        bool useDHCP = request->hasParam("dhcp", true);
        preferences.putBool("useDHCP", useDHCP);

           // Serve the configuration updated page with a button to return home
        String html = "<html><body>"
                      "<h1>Configuration Updated</h1>"
                      "<p>The configuration has been updated. Please restart the device.</p>"
                      "<button onclick=\"location.href='/'\">Return Home</button>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });

    // Start the server
    server.begin();
}