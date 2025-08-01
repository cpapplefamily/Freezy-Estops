#include "WebServerSetup.h"

// Define the web server and preferences objects
AsyncWebServer server(80);
Preferences preferences;
extern String deviceRole;
extern String arenaIP;
extern bool useDHCP;
extern String deviceIP;
extern String deviceGWIP;
extern String arenaPort;

void setupWebServer()
{
    // Load the alliance DeviceRole from preferences
    preferences.begin("settings", false);
    deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
    arenaIP = preferences.getString("arenaIP", "10.0.100.5");
    arenaPort = preferences.getString("arenaPort", "8080");

    // Load IP address and DHCP/Static configuration from preferences
    deviceIP = preferences.getString("deviceIP", "");
    deviceGWIP = preferences.getString("deviceGateway", "");
    useDHCP = preferences.getBool("useDHCP", true);

    // Set up the web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Serve the initial page with a button to go to the settings page
        String html = "<html><body>"
                      "<h1>Welcome to Freezy Estops</h1>"
                      "<p>This is the initial page.</p>"
                      "<button onclick=\"location.href='/setup'\">Go to Settings</button>"
                      "</body></html>";
        request->send(200, "text/html", html); });

    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Serve the HTML form for alliance DeviceRole and network configuration
        String html = "<html><body>"
                      "<h1>Setup Configuration</h1>"
                      "<form action=\"/setConfig\" method=\"POST\">"
                      "<label for=\"deviceRole\">Choose Device Role: </label>"
                      "<select name=\"deviceRole\" id=\"deviceRole\">"
                      "<option value=\"RED_ALLIANCE\"" + String(deviceRole == "RED_ALLIANCE" ? " selected" : "") + ">RED_ALLIANCE</option>"
                      "<option value=\"BLUE_ALLIANCE\"" + String(deviceRole == "BLUE_ALLIANCE" ? " selected" : "") + ">BLUE_ALLIANCE</option>"
                      "<option value=\"FMS_TABLE\"" + String(deviceRole == "FMS_TABLE" ? " selected" : "") + ">FMS_TABLE</option>"
                      "</select><br><br>"
                      "<input type=\"checkbox\" id=\"dhcp\" name=\"dhcp\" " + String(useDHCP ? "checked" : "") + " onchange=\"toggleIPInput()\">"
                      "<label for=\"dhcp\">Use DHCP</label><br><br>"

                      "<fieldset id=\"staticIPFields\" " + String(useDHCP ? "disabled" : "") + ">"
                      "<legend>Static Network Configuration</legend>"
                      "<label for=\"ip\">Device IP: </label>"
                      "<input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + deviceIP + "\"><br><br>"
                      "<label for=\"gw\">Gateway IP: </label>"
                      "<input type=\"text\" id=\"gw\" name=\"gw\" value=\"" + deviceGWIP + "\"><br><br>"
                      "</fieldset>"

                      "<br><br>"
                      "<fieldset id=\"arenaIPFields\">"
                      "<legend>Freezy Arena Network Configuration</legend>"
                      "<label for=\"arenaIP\">Arena IP: </label>"
                      "<input type=\"text\" id=\"arenaIP\" name=\"arenaIP\" value=\"" + arenaIP + "\">"
                      "<label for=\"arenaPort\"> Port: </label>"
                      "<input type=\"text\" id=\"arenaPort\" name=\"arenaPort\" value=\"" + String(arenaPort) + "\"><br><br>"
                      "</fieldset>"

                      "<br><br>"
                      "<input type=\"submit\" value=\"Submit\">"
                      "</form>"
                      "<script>"
                      "function toggleIPInput() {"
                      "  var dhcpCheckbox = document.getElementById('dhcp');"
                      "  var staticIPFields = document.getElementById('staticIPFields');"
                      "  staticIPFields.disabled = dhcpCheckbox.checked;"
                      "}"
                      "</script>"
                      "</body></html>";
        request->send(200, "text/html", html); });

    server.on("/setConfig", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        // Handle the form submission and update the configuration
        if (request->hasParam("deviceRole", true)) {
            deviceRole = request->getParam("deviceRole", true)->value();
            preferences.putString("deviceRole", deviceRole);
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
        request->send(200, "text/html", html); });

    // Start the server
    server.begin();
}