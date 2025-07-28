#include "WebServerSetup.h"

// Define the web server and preferences objects
AsyncWebServer server(80);
Preferences preferences;
extern String allianceColor;
extern String arenaIP;
extern bool useDHCP;
extern String deviceIP;
extern String deviceGWIP;
extern String arenaPort;

void setupWebServer() {
    // Load the alliance color from preferences
    preferences.begin("settings", false);
    allianceColor = preferences.getString("allianceColor", "Red");
    arenaIP = preferences.getString("arenaIP", "10.0.100.5");
    arenaPort = preferences.getString("arenaPort", "8080");

    // Load IP address and DHCP/Static configuration from preferences
    deviceIP = preferences.getString("deviceIP", "");
    deviceGWIP = preferences.getString("deviceGateway", "10.0.100.1");
    useDHCP = preferences.getBool("useDHCP", true);

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

    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request){
        // Serve the HTML form for alliance color and network configuration
        String html = "<html><body>"
                      "<h1>Setup Configuration</h1>"
                      "<form action=\"/setConfig\" method=\"POST\">"
                      "<label for=\"color\">Choose an alliance color: </label>"
                      "<select name=\"color\" id=\"color\">"
                      "<option value=\"Red\"" + String(allianceColor == "Red" ? " selected" : "") + ">Red</option>"
                      "<option value=\"Blue\"" + String(allianceColor == "Blue" ? " selected" : "") + ">Blue</option>"
                      "<option value=\"Field\"" + String(allianceColor == "Field" ? " selected" : "") + ">Field</option>"
                      "</select><br><br>"
                      "<input type=\"checkbox\" id=\"dhcp\" name=\"dhcp\" " + String(useDHCP ? "checked" : "") + " onchange=\"toggleIPInput()\">"
                      "<label for=\"dhcp\">Use DHCP</label><br><br>"
                      
                      "<label for=\"ip\">Device IP: </label>"
                      "<input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + deviceIP + "\"" + (useDHCP ? " disabled" : "") + "><br><br>"

                      "<label for=\"gw\">Gateway IP: </label>"
                      "<input type=\"text\" id=\"gw\" name=\"gw\" value=\"" + deviceGWIP + "\"" + (useDHCP ? " disabled" : "") + "><br><br>"

                      "<label for=\"arenaIP\">Arena IP: </label>"
                      "<input type=\"text\" id=\"arenaIP\" name=\"arenaIP\" value=\"" + arenaIP + "\">"
                      "<label for=\"arenaPort\"> Port: </label>"
                      "<input type=\"text\" id=\"arenaPort\" name=\"arenaPort\" value=\"" + String(arenaPort) + "\"><br><br>"
                      "<input type=\"submit\" value=\"Submit\">"
                      "</form>"
                      "<script>"
                      "function toggleIPInput() {"
                      "  var dhcpCheckbox = document.getElementById('dhcp');"
                      "  var ipInput1 = document.getElementById('ip');"
                      "  ipInput1.disabled = dhcpCheckbox.checked;"
                      "  var ipInput2 = document.getElementById('gw');"
                      "  ipInput2.disabled = dhcpCheckbox.checked;"
                      "}"
                      "</script>"
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
            deviceIP = request->getParam("ip", true)->value();
            preferences.putString("deviceIP", deviceIP);
        }
        if (request->hasParam("gw", true)) {
            deviceGWIP = request->getParam("gw", true)->value();
            preferences.putString("deviceGWIP", deviceGWIP);
        }
        if (request->hasParam("arenaIP", true)) {
            arenaIP = request->getParam("arenaIP", true)->value();
            preferences.putString("arenaIP", arenaIP);
        }
        if (request->hasParam("arenaPort", true)) {
            arenaPort = request->getParam("arenaPort", true)->value();
            preferences.putString("arenaPort", arenaPort);
        }
        useDHCP = request->hasParam("dhcp", true);
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