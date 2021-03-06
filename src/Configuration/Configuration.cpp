#include "Configuration.h"

// Webserver instance and handle
ESP8266WebServer server(80);
/*
    Gets called on access to url 192.168.1.1/
*/

/**
 * Empty constructor
 */
Configuration::Configuration(){

};

/**
 * Sets reference to external components
 */
void Configuration::setReference() {
};

/**
 * Does init stuff for the Configuration component
 *
 * @return True if successfull, false if not
 */
bool Configuration::Init() {
    if (!init) {
        // Configure Flash button on the nodemcu as reset putton
        pinMode(0, INPUT_PULLUP);

        // Enable blink led
        digitalWrite(LED_BUILTIN, HIGH);
        pinMode(LED_BUILTIN, OUTPUT);

        // Start LittleFS
        Serial.println("Mount LittleFS");
        if (!LittleFS.begin()) {
            Serial.println("LittleFS mount Failed!");
            return init;
        } else {
            Serial.println("LittleFS mount OK!");
            Serial.println("");
        }

        // Create config.txt if missing
        createConfig();

        // List all found files
        listFiles();
        // Load config
        loadConfig();

        // Check if controller is already configured
        if (!data.isConfigured) {
            resetOrNotConfigured = true;
        } else {
            // All okay we can procedure with the rest
            isFinished = true;
        }

        init = true;
    }

    return init;
};

/**
 * Runs the Configuration component
 */
void Configuration::Run() {
    if (!init) {
        return;
    }

    unsigned long curMillis = millis();
    // Check if button is pressed longer then 10 sec
    if (digitalRead(0) == 0) {
        if (curMillis - prevMillisReset >= timeoutRest) {
            isFinished = false;
            resetOrNotConfigured = true;
        }

        if (!resetOrNotConfigured) {
            prevMillisResetBlink = curMillis;
        }
    } else {
        // Wait for button release or if not configured start configuration
        if (resetOrNotConfigured) {
            // ================ CONFIGURATION PROCEDURE ================ //
            switch (state) {
                case 0:
                    Serial.println("");
                    Serial.println("Sauna Temperature Sensor Mk1 in configuration mode");
                    state++;
                    break;

                    // Save that we are not configured or doing a reset
                case 1:
                    data.isConfigured = false;
                    state++;
                    break;

                    // Stop wifi connection
                case 2:
                    if (WiFi.status() == WL_CONNECTED) {
                        WiFi.disconnect();
                    } else {
                        state++;
                    }
                    break;

                    // Create Access Point
                case 3:
                    WiFi.mode(WIFI_AP);
                    WiFi.softAPConfig(this->apIP, this->apIP, IPAddress(255, 255, 255, 0));
                    WiFi.softAP("Sauna Temperature Sensor Mk1","",0,0,1);
                    state++;
                    break;

                    // Start webserver
                case 4:
                    server.on("/", [this]() { this->inputForm(); });
                    server.on("/final", [this]() { this->inputFormFilled(); });
                    server.begin();
                    Serial.println("Waiting for user configuration");
                    state++;
                    break;

                    // Wait for user configuration
                case 5:
                    server.handleClient();
                    prevMillisAPShutdown = curMillis;
                    break;

                    // Wait before shuting down AP
                case 6:
                    if (curMillis - prevMillisAPShutdown >= timeoutAPShutdown) {
                        Serial.println("User finished configuration");
                        state = 99;
                    }
                    break;

                    // Reset everything and we are good to go
                case 99:
                    WiFi.softAPdisconnect(true);
                    ledOn = false;
                    digitalWrite(LED_BUILTIN, HIGH);
                    resetOrNotConfigured = false;
                    data.isConfigured = true;
                    resetConfig();
                    saveConfig();
                    isFinished = true;
                    Serial.println("Sauna Temperature Sensor Mk1 finished configuration");
                    Serial.println("");
                    state = 0;
                    break;
            }
        }

        prevMillisReset = curMillis;
    }

    // Blink onboard led as long as we are in configuration
    if (resetOrNotConfigured == true) {
        if (curMillis - prevMillisResetBlink >= timeoutResetBlink) {
            if (ledOn) {
                digitalWrite(LED_BUILTIN, LOW);
            } else {
                digitalWrite(LED_BUILTIN, HIGH);
            }
            ledOn = !ledOn;

            prevMillisResetBlink = curMillis;
        }
    }
};

void Configuration::inputForm() {
    String WEP_PAGE =
        "<!DOCTYPE html><html><head> <meta name='viewport' "
        "content='width=device-width, initial-scale=1.0, maximum-scale=2.0, "
        "user-scalable=no'> <style> body { background-color: #616161; width: "
        "100vw; height: 100vh; display: flex; justify-content: center; "
        "align-items: center; } form { display: flex; justify-content: center; "
        "align-items: center; flex-direction: column; } h1, label, input { "
        "color: #ffffff; font-family: Arial, Helvetica, sans-serif; font-size: "
        "20px; } h1 { text-align: center; font-size: 35px; padding: 10px; "
        "color: "
        "#aeea00; } input { color: #000000; } #submitButton { border-width: 0; "
        "outline: none; border-radius: 2px; box-shadow: 0 1px 4px rgba(0, 0, "
        "0, "
        ".6); margin-top: 10px; padding: 10px; width: 50%; background-color: "
        "#2ecc71; color: #ecf0f1; } td { padding: 5px; } </style></head><body> "
        "<div> <h1>Sauna Controller Configuration</h1> <form action='/final' "
        "method='post'> <table> <tr> <td><label>WiFi SSID</label></td> "
        "<td><input type='text' required name='wifiSSID'></td> </tr> <tr> "
        "<td><label>WiFi Password</label></td> <td><input type='text' required "
        "name='wifiPassword'></td> </tr> <tr> <td><label>MQTT Broker IP "
        "Address</label></td> <td><input type='text' required "
        "pattern='^([0-9]{1,3}\\.){3}[0-9]{1,3}$' name='mqttBrokerIpAddress'> "
        "</td> </tr> <tr> <td><label>MQTT Broker Port</label></td> <td><input "
        "type='number' required min='1' name='mqttBrokerPort'></td> </tr> <tr> "
        "<td><label>MQTT Broker Username</label></td> <td><input type='text' "
        "required name='mqttBrokerUsername'></td> </tr> <tr> <td><label>MQTT "
        "Broker Password</label></td> <td><input type='text' required "
        "name='mqttBrokerPassword'></td> </tr> <tr> <td><label>MQTT Client "
        "Name</label></td> <td><input type='text' required "
        "name='mqttClientName'></td> </tr> </table> <input id='submitButton' "
        "type='submit' value='Submit'> </form> </div></body></html>";

    server.send(200, "text/html", WEP_PAGE);
};

/*
    Gets called on access to url 192.168.4.1/final
*/
void Configuration::inputFormFilled() {
    String WEP_PAGE =
        "<!DOCTYPE html><html><head> <meta name='viewport' "
        "content='width=device-width, initial-scale=1.0, maximum-scale=2.0, "
        "user-scalable=no'> <style> body { background-color: #616161; width: "
        "100vw; height: 100vh; display: flex; justify-content: center; "
        "align-items: center; } h1 { color: #ffffff; font-family: Arial, "
        "Helvetica, sans-serif; text-align: center; font-size: 35px; "
        "border-width: 0; outline: none; border-radius: 2px; box-shadow: 0 1px "
        "4px rgba(0, 0, 0, .6); margin-top: 10px; padding: 10px; width: 50%; "
        "background-color: #2ecc71; color: #ecf0f1; } </style></head><body> "
        "<div> <h1>Configuration Submitted you can now close the Browser</h1> "
        "</div></body></html>";

    if (server.hasArg("wifiSSID")) {
        data.wifiSSID = server.arg("wifiSSID");
    }
    if (server.hasArg("wifiPassword")) {
        data.wifiPassword = server.arg("wifiPassword");
    }
    if (server.hasArg("mqttBrokerIpAddress")) {
        data.mqttBrokerIpAddress = server.arg("mqttBrokerIpAddress");
    }
    if (server.hasArg("mqttBrokerUsername")) {
        data.mqttBrokerUsername = server.arg("mqttBrokerUsername");
    }
    if (server.hasArg("mqttBrokerPassword")) {
        data.mqttBrokerPassword = server.arg("mqttBrokerPassword");
    }
    if (server.hasArg("mqttBrokerPort")) {
        data.mqttBrokerPort =
            strtol(server.arg("mqttBrokerPort").c_str(), NULL, 0);
    }
    if (server.hasArg("mqttClientName")) {
        data.mqttClientName = server.arg("mqttClientName");
    }
    data.isConfigured = false;

    server.send(200, "text/html", WEP_PAGE);
    // Go to next state
    state++;
};

/**
 * Returns the current config data
 *
 * @return ConfiguredData
 */
ConfiguredData Configuration::getData() { return data; };

/**
 * Saves the current config to a persist data storage
 */
void Configuration::saveConfig() {
    Serial.println(F("Saving Config"));
    Serial.println(F(""));

    File file = LittleFS.open("/config.txt", "w");
    if (!file) {
        Serial.println(F("Failed to open file for writing"));
        return;
    }

    // ==== wifiSSID ==== //
    if (!file.println(data.wifiSSID)) {
        Serial.println(F("wifiSSID failed to save"));
    }

    // ==== wifiPassword ==== //
    if (!file.println(data.wifiPassword)) {
        Serial.println(F("wifiPassword failed to save"));
    }

    // ==== mqttBrokerIpAddress ==== //
    if (!file.println(data.mqttBrokerIpAddress)) {
        Serial.println(F("mqttBrokerIpAddress failed to save"));
    }

    // ==== mqttBrokerPort ==== //
    if (!file.println(String(data.mqttBrokerPort))) {
        Serial.println(F("mqttBrokerPort failed to save"));
    }

    // ==== mqttBrokerUsername ==== //
    if (!file.println(data.mqttBrokerUsername)) {
        Serial.println(F("mqttBrokerUsername failed to save"));
    }

    // ==== mqttBrokerPassword ==== //
    if (!file.println(data.mqttBrokerPassword)) {
        Serial.println(F("mqttBrokerPassword failed to save"));
    }

    // ==== mqttClientName ==== //
    if (!file.println(data.mqttClientName)) {
        Serial.println(F("mqttClientName failed to save"));
    }

    // ==== isConfigured ==== //
    if (!file.println(String(data.isConfigured))) {
        Serial.println(F("isConfigured failed to save"));
    }

    delay(2000);  // Make sure the CREATE and LASTWRITE times are different
    file.close();
};

/**
 * Loads the saved config from a persist data storage
 */
void Configuration::loadConfig() {
    Serial.println(F("Loading Config"));
    Serial.println("");

    File file = LittleFS.open("/config.txt", "r");
    if (!file) {
        Serial.println(F("Failed to open file for reading"));
        return;
    }

    uint8_t state = 0;
    String message = "";
    char symbol = '\0';
    while (file.available()) {
        int x = file.read();

        // Check for ascii symbol 13 => Carriage Return
        if (x == 13) {
            x = file.read();
            // Check for ascii symbol 10 => Line Feed
            if (x == 10) {
                switch (state) {
                        // ==== wifiSSID ==== //
                    case 0:
                        data.wifiSSID = message;
                        state++;
                        break;
                        // ==== wifiPassword ==== //
                    case 1:
                        data.wifiPassword = message;
                        state++;
                        break;
                        // ==== mqttBrokerIpAddress ==== //
                    case 2:
                        data.mqttBrokerIpAddress = message;
                        state++;
                        break;
                        // ==== mqttBrokerPort ==== //
                    case 3:
                        data.mqttBrokerPort = strtol(message.c_str(), NULL, 0);
                        state++;
                        break;
                        // ==== mqttBrokerUsername ==== //
                    case 4:
                        data.mqttBrokerUsername = message;
                        state++;
                        break;
                        // ==== mqttBrokerPassword ==== //
                    case 5:
                        data.mqttBrokerPassword = message;
                        state++;
                        break;
                        // ==== mqttClientName ==== //
                    case 6:
                        data.mqttClientName = message;
                        state++;
                        break;
                        // ==== isConfigured ==== //
                    case 7:
                        data.isConfigured = bool(message);
                        break;
                }
                message = "";
            }
        } else {
            if (x != 0) {
                symbol = char(x);
                message += String(symbol);
            }
        }
    }
    file.close();
};

/**
 * Lists all configs found on the persits data storage
 */
void Configuration::listFiles() {
    Serial.println(F("Found files in root dir:"));

    Dir root = LittleFS.openDir("/");

    while (root.next()) {
        File file = root.openFile("r");
        Serial.print(F("  FILE: "));
        Serial.print(root.fileName());
        Serial.print(F("  SIZE: "));
        Serial.println(file.size());
        file.close();
    }
};

/**
 * Lists all configs found on the persits data storage
 */
void Configuration::createConfig() {
    Serial.println(F("Create config.txt file if missing"));
    if (!LittleFS.exists("/config.txt")) {
        Serial.println(F("Creating new config.txt file"));
        File file = LittleFS.open("/config.txt", "w+");
        file.close();
    } else {
        Serial.println(F("Found existing config.txt file"));
    }
};

/**
 * Resets the config file
 */
void Configuration::resetConfig() {
    Serial.println(F("Reset config.txt file"));
    if (LittleFS.exists("/config.txt")) {
        File file = LittleFS.open("/config.txt", "w");
        file.close();
    }
};
