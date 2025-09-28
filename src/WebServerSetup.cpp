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
extern bool stopButtonStates[6]; // Declare external array from Main.cpp
extern bool startButtonState;   // Declare external start button state from Main.cpp
extern bool sonarAlertSent;   // Declare external sonarAlertSent state from Main.cpp
extern float sonarDistance;     // Declare external sonar distance from Main.cpp
extern unsigned long alertTrigCm; // Declare external alert threshold from Main.cpp
extern unsigned long alertHoldMs; // Declare external alert hold time from Main.cpp
extern unsigned long minOffMs;   // Declare external min off time from Main.cpp

void setupWebServer()
{
    // Load the alliance DeviceRole and other settings from preferences
    preferences.begin("settings", false);
    deviceRole = preferences.getString("deviceRole", "RED_ALLIANCE");
    arenaIP = preferences.getString("arenaIP", "10.0.100.5");
    arenaPort = preferences.getString("arenaPort", "8080");
    deviceIP = preferences.getString("deviceIP", "");
    deviceGWIP = preferences.getString("deviceGateway", "");
    useDHCP = preferences.getBool("useDHCP", true);
    alertTrigCm = preferences.getULong("alertTrigCm", 30); // Default: 30 cm
    alertHoldMs = preferences.getULong("alertHoldMs", 1000); // Default: 1000 ms
    minOffMs = preferences.getULong("minOffMs", 500); // Default: 500 ms
    //preferences.end(); // Close preferences to ensure data is saved

    // Set up the web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Serve the initial page with a button to go to the settings page and LED indicators
        String html = "<html><body>"
                      "<h1>Welcome to Freezy Estops</h1>"
                      "<p>Current Device Role: " + deviceRole + "</p>";
        
        // Add sonar distance and LED indicators based on deviceRole
        if (deviceRole == "RED_ALLIANCE" || deviceRole == "BLUE_ALLIANCE") {
            html += "<p>Sonar Distance: <span id='sonarDistance'>0.00</span> cm</p>"
                    "<div style='text-align: center;'>"
                    "<div id='ledsonarAlertSent' style='width: 30px; height: 30px; border-radius: 50%; background-color: gray;'></div>"
                    "</div>"
                    "<p>sonarAlertSent</p>"
                    "<h2>Stop Button States</h2>"
                    "<div id='buttonStates' style='display: flex; flex-wrap: wrap; gap: 20px;'>";
            for (int i = 1; i <= 6; i++) {
                html += "<div style='text-align: center;'>"
                        "<div id='led" + String(i) + "' style='width: 30px; height: 30px; border-radius: 50%; background-color: gray;'></div>"
                        "<p>Button " + String(i) + "</p>"
                        "</div>";
            }
            html += "</div>"
                    "<script>"
                    "function updateLEDs() {"
                    "  fetch('/buttonStates')"
                    "    .then(response => response.json())"
                    "    .then(data => {"
                    "      for (let i = 1; i <= 6; i++) {"
                    "        let led = document.getElementById('led' + i);"
                    "        let ledsonarAlertSent = document.getElementById('ledsonarAlertSent');"
                    "        led.style.backgroundColor = data['button' + i] ? 'red' : 'gray';"
                    "        ledsonarAlertSent.style.backgroundColor = data['sonarAlert'] ? 'green' : 'gray';"
                    "      }"
                    "      document.getElementById('sonarDistance').textContent = data['sonarDistance'].toFixed(2);"
                    "    });"
                    "}"
                    "setInterval(updateLEDs, 500);" // Update every 500ms
                    "updateLEDs();" // Initial update
                    "</script>";
        }
        else if (deviceRole == "FMS_TABLE") {
            html += "<h2>Button States</h2>"
                    "<div id='buttonStates' style='display: flex; flex-wrap: wrap; gap: 20px;'>"
                    "<div style='text-align: center;'>"
                    "<div id='ledStop' style='width: 30px; height: 30px; border-radius: 50%; background-color: gray;'></div>"
                    "<p>FMS Stop Button</p>"
                    "</div>"
                    "<div style='text-align: center;'>"
                    "<div id='ledStart' style='width: 30px; height: 30px; border-radius: 50%; background-color: gray;'></div>"
                    "<p>FMS Start Button</p>"
                    "</div>"
                    "</div>"
                    "<script>"
                    "function updateLEDs() {"
                    "  fetch('/buttonStates')"
                    "    .then(response => response.json())"
                    "    .then(data => {"
                    "        let ledStop = document.getElementById('ledStop');"
                    "        let ledStart = document.getElementById('ledStart');"
                    "        ledStop.style.backgroundColor = data['button1'] ? 'red' : 'gray';"
                    "        ledStart.style.backgroundColor = data['startButton'] ? 'green' : 'gray';"
                    "    });"
                    "}"
                    "setInterval(updateLEDs, 500);" // Update every 500ms
                    "updateLEDs();" // Initial update
                    "</script>";
        }

        html += "<button onclick=\"location.href='/setup'\">Go to Settings</button>"
                "</body></html>";
        request->send(200, "text/html", html); });

    // Route to handle favicon.ico requests
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        request->send(204); // No Content response to suppress favicon error
    });

    // Route to provide button states and sonar distance as JSON
    server.on("/buttonStates", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        for (int i = 0; i < 6; i++) {
            json += "\"button" + String(i + 1) + "\":" + String(stopButtonStates[i]);
            if (i < 5) json += ",";
        }
        json += ",\"startButton\":" + String(startButtonState);
        json += ",\"sonarDistance\":" + String(sonarDistance, 2); // Format to 2 decimal places
        json += ",\"sonarAlert\":" + String(sonarAlertSent);
        json += "}";
        request->send(200, "application/json", json);
    });

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
                      "<option value=\"BARGE_LIGHTS\"" + String(deviceRole == "BARGE_LIGHTS" ? " selected" : "") + ">BARGE_LIGHTS</option>"
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
                      "<br><br>";
        // Add sensor settings based on deviceRole             
        if (deviceRole == "RED_ALLIANCE" || deviceRole == "BLUE_ALLIANCE") {
           html +=    "<fieldset id=\"alertFields\">"
                      "<legend>Sensor Configuration</legend>"
                      "<label for=\"alertTrigCm\">Alert Threshold (cm): </label>"
                      "<input type=\"number\" id=\"alertTrigCm\" name=\"alertTrigCm\" value=\"" + String(alertTrigCm) + "\"><br><br>"
                      "<label for=\"alertHoldMs\">Alert Hold Time (ms): </label>"
                      "<input type=\"number\" id=\"alertHoldMs\" name=\"alertHoldMs\" value=\"" + String(alertHoldMs) + "\"><br><br>"
                      "<label for=\"minOffMs\">Minimum Off Time (ms): </label>"
                      "<input type=\"number\" id=\"minOffMs\" name=\"minOffMs\" value=\"" + String(minOffMs) + "\"><br><br>"
                      "</fieldset>";
        } else {
            
        }

        html +=       "<br><br>"
                      "<input type=\"submit\" value=\"Submit\">"
                      "<button type=\"button\" onclick=\"location.href='/'\">Cancel</button>"
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
        //preferences.begin("settings", false); // Reopen preferences for writing
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
        if (request->hasParam("alertTrigCm", true)) {
            alertTrigCm = request->getParam("alertTrigCm", true)->value().toInt();
            preferences.putULong("alertTrigCm", alertTrigCm);
        }
        if (request->hasParam("alertHoldMs", true)) {
            alertHoldMs = request->getParam("alertHoldMs", true)->value().toInt();
            preferences.putULong("alertHoldMs", alertHoldMs);
        }
        if (request->hasParam("minOffMs", true)) {
            minOffMs = request->getParam("minOffMs", true)->value().toInt();
            preferences.putULong("minOffMs", minOffMs);
        }
        useDHCP = request->hasParam("dhcp", true);
        preferences.putBool("useDHCP", useDHCP);
        //preferences.end(); // Close preferences to ensure data is saved
        
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