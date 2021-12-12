#include "Network.h"

/**
 * Empty constructor
 */
Network::Network(String codeVersion){
    this->codeVersion = codeVersion;
};

/**
 * Sets reference to external components
 */
void Network::setReference(Configuration *configuration,
                           Information *information,
                           TemperatureSensor *temperatureSensor)
{
    this->configuration = configuration;
    this->information = information;
    this->temperatureSensor = temperatureSensor;
};

/**
 * Does init stuff for the Network component
 * 
 * @return True if successfull, false if not 
 */
bool Network::Init()
{
    if (!init)
    {
        data = configuration->getData();

        mqttClient.setClient(wifiMqtt);
        mqttClient.setServer(data.mqttBrokerIpAddress.c_str(),
                             data.mqttBrokerPort);
        //mqttClient.setCallback(mqttCallback);
        mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length) {
            this->MqttCallback(topic, payload, length);
        });

        timeClient.begin();

        wifiState = NetworkWiFiState::StartWifi;
        mqttState = NetworkMQTTState::StartMqtt;

        virtualPIRSensorTriggered = false;

        Serial.println(F("Network initialized"));
        init = true;
    }

    return init;
};

/**
 * Runs the Network component. 
 */
void Network::Run()
{
    if (!init)
    {
        return;
    }

    mqttClient.loop();

    // -- Wifi
    HandleWifi();
    if (wifiConnected != memWifiConnected)
    {
        memWifiConnected = wifiConnected;
        wifiOneTimePrint = true;
    }
    if (wifiOneTimePrint)
    {
        if (wifiConnected)
        {
            ipAddress = WiFi.localIP().toString().c_str();
            subnetmask = WiFi.subnetMask().toString().c_str();
            macAddress = WiFi.macAddress();
            gateway = WiFi.gatewayIP().toString().c_str();
            hostname = WiFi.hostname();
        }
        wifiOneTimePrint = false;
    }

    // -- Mqtt
    HandleMqtt();
    if (mqttConnected != memMqttConnected)
    {
        memMqttConnected = mqttConnected;
        mqttOneTimePrint = true;
    }
    if (mqttOneTimePrint)
    {
        clientName = data.mqttClientName.c_str();
        brokerIpAddress = data.mqttBrokerIpAddress.c_str();
        brokerPort = data.mqttBrokerPort;
        mqttOneTimePrint = false;
    }

    // -- NTP
    HandleNTP();

    // -- Republish
    HandleRepublish();
};

/**
 * Handels the WiFi connection.
 * Auto reconnects on dc
 */
void Network::HandleWifi()
{
    switch (wifiState)
    {
    case NetworkWiFiState::StartWifi:
        WiFi.mode(WIFI_STA);
        WiFi.hostname(data.mqttClientName.c_str());
        WiFi.begin(data.wifiSSID.c_str(),
                   data.wifiPassword.c_str());
        wifiState = NetworkWiFiState::SuperviseWiFiConnection;
        break;

    case NetworkWiFiState::SuperviseWiFiConnection:
        if (WiFi.status() != WL_CONNECTED)
        {
            wifiState = NetworkWiFiState::CheckWiFiDisconnect; // Check if dc occurred
            PrevMillis_WiFiTimeout = millis();                 // Set time for WiFi timeout check
        }
        else
        {
            wifiConnected = true;
        }
        break;

    case NetworkWiFiState::CheckWiFiDisconnect:
        if (WiFi.status() != WL_CONNECTED)
        {
            // Wait for timeout. After timeout restart WiFi
            unsigned long CurMillis_WiFiTimeout = millis();
            if (CurMillis_WiFiTimeout - PrevMillis_WiFiTimeout >= TimeOut_WiFiTimeout)
            {
                wifiConnected = false;
                PrevMillis_WiFiTimeout = CurMillis_WiFiTimeout;
                WiFi.disconnect(); // Disconnect WiFi and start new connection
                wifiState = NetworkWiFiState::StartWifi;
            }
        }
        else
        {
            wifiState = NetworkWiFiState::SuperviseWiFiConnection; // WiFi reconnected
        }
        break;

    default:
        Serial.println(F("Wifi State Error!"));
        break;
    }
};

/**
 * Handels the MQTT connection after the wifi is connected
 * Subscribes to a list pre defined topics
 * Auto reconnects after dc and resubscribes to the defined topics
 */
void Network::HandleMqtt()
{

    // Client state for information print
    clientState = mqttClient.state();

    switch (mqttState)
    {
    case NetworkMQTTState::StartMqtt:
        // Only try reconnect when WiFi is connected
        if (wifiConnected)
        {
            if (mqttClient.connect(data.mqttClientName.c_str(),
                                   data.mqttBrokerUsername.c_str(),
                                   data.mqttBrokerPassword.c_str()))
            {
                // ================ HomeAssistant ================ //
                /*
                    These Mqtt paths are for the paths given for the desired behavior when controlloing over homeassistant
                */
                // ==== Global ==== //

                // Code Version
                // The installed code version of the LED Controller Mk4.1 is published under the following path on connect
                // "SaunaController/" + data.mqttClientName + "/Version"

                // ==== Specific ==== //
                
                // ==== Virtual ==== //

                // ================ Json ================ //
                /*
                    These Mqtt paths are for the paths given for the desired behavior when controlloing over custom json data
                */
                // ==== Global ==== //

                // ==== Specific ==== //

                // ==== Virtual ==== //

                // === Republish == //
                PublishTemperatureData();
                PublishCodeVersion();

                mqttState = NetworkMQTTState::SuperviseMqttConnection;
            }
        }
        break;

    case NetworkMQTTState::SuperviseMqttConnection:
        if (!mqttClient.connected())
        {
            mqttState = NetworkMQTTState::CheckMqttDisconnect; // Check if dc occurred
            PrevMillis_MQTTTimeout = millis();                 // Set time for WiFi timeout check
        }
        else
        {
            mqttConnected = true;
        }
        break;

    case NetworkMQTTState::CheckMqttDisconnect:
        if (!mqttClient.connected())
        {
            // Wait for timeout. After timeout restart WiFi
            unsigned long CurMillis_MQTTTimeout = millis();
            if (CurMillis_MQTTTimeout - PrevMillis_MQTTTimeout >= TimeOut_MQTTTimeout)
            {
                mqttConnected = false;
                PrevMillis_MQTTTimeout = CurMillis_MQTTTimeout;
                mqttClient.disconnect(); // Disconnect MQTT and start new connection
                mqttState = NetworkMQTTState::StartMqtt;
            }
        }
        else
        {
            mqttState = NetworkMQTTState::SuperviseMqttConnection; // WiFi reconnected
        }
        break;

    default:
        Serial.println(F("Mqtt State Error!"));
        break;
    }
};

/**
 * Handles the Network Time Protocol for accurate time updates
 */
void Network::HandleNTP()
{
    // Get Time update
    unsigned long CurMillis_NTPTimeout = millis();
    if (CurMillis_NTPTimeout - PrevMillis_NTPTimeout >= TimeOut_NTPTimeout)
    {
        PrevMillis_NTPTimeout = CurMillis_NTPTimeout;
        bool updateSuccessful = timeClient.update();

        if (updateSuccessful)
        {
            stNetworkTimeData.hour = timeClient.getHours();
            stNetworkTimeData.minute = timeClient.getMinutes();
            stNetworkTimeData.second = timeClient.getSeconds();
            stNetworkTimeData.unix = timeClient.getEpochTime();
        }
    }
};

/**
 * MQTT callback function.
 * Processes all the receives commands from the subscribed topics
 * 
 * @parameter *topic    A pointer to a char array containing the mqtt topic that calles this function with new data
 * @parameter *payload  A pointer to a byte array with data send over the mqtt topic
 * @parameter length    The length of the byte data array
 **/
void Network::MqttCallback(char *topic, byte *payload, unsigned int length)
{

    //-- Get Message and add terminator
    char message[length + 1];    // Main data received
    char memMessage[length + 1]; // Mem of main data because main data gets changed after json deserialize
    for (unsigned int i = 0; i < length; i++)
    {
        message[i] = (char)payload[i];
        memMessage[i] = (char)payload[i];
    }
    message[length] = '\0';
    memMessage[length] = '\0';

    ConfiguredData configuredData = configuration->getData();

    // # ================================ HomeAssistant ================================ //
    /*
        Callback stuff for data received by home assistant endpoints
    */

    // ================ Global ================ //
   
    // ================ Specific ================ //
    
    // ================ Virtual ================ //


    // # ================================ JSON ================================ //
    /*
        Callback stuff for data received by json endpoints
    */

    // ================ Global ================ //
   
    // ================ Specific ================ //
    
    // ================ Virtual ================ //
    
}

void Network::HandleRepublish()
{
    // For now disabled
    return;

    unsigned long curMillis = millis();

    // == Temperature Data
    if (curMillis - prevMillisPublishTemperatureData >= timeoutPublishTemperatureData)
    {
        PublishTemperatureData();
    }

    // == Heartbeat
    if (curMillis - prevMillisPublishHeartbeat >= timeoutPublishHeartbeat)
    {
        PublishHeartbeat();
    }

    // == Network
    if (curMillis - prevMillisPublishNetwork >= timeoutPublishNetwork)
    {
        PublishNetwork();
    }
}

void Network::PublishHomeassistantHeartbeat()
{
    String message = "pulse";

    // ==== Heartbeat
    mqttClient.publish(("SaunaController/" + data.mqttClientName + "/HomeAssistant/Heartbeat/state").c_str(), message.c_str());
}

void Network::PublishHomeassistantTemperatureData()
{
    String message = "";

    // ==== Temperature Data
    message = String(stNetworkMotionData.motionDetectionEnabled);
    mqttClient.publish(("SaunaController/" + data.mqttClientName + "/HomeAssistant/MotionDetection/Enable/state").c_str(), message.c_str());
 }

void Network::PublishHomeassistantNetwork()
{
    String message = "";

    // ==== Network
    message = ipAddress;
    mqttClient.publish(("SaunaController/" + data.mqttClientName + "/HomeAssistant/Network/IPAddress/state").c_str(), message.c_str());
    message = macAddress;
    mqttClient.publish(("SaunaController/" + data.mqttClientName + "/HomeAssistant/Network/MACAddress/state").c_str(), message.c_str());
}

void Network::PublishJsonHeartbeat()
{
}

void Network::PublishJsonTemperatureData()
{
}

void Network::PublishJsonNetwork()
{
}

/**
 * Publishes a heartbeat
 */
void Network::PublishHeartbeat()
{
    prevMillisPublishHeartbeat = millis();

    // == Home Assistant
    PublishHomeassistantHeartbeat();

    // == JSON
    PublishJsonHeartbeat();
}

/**
 * Publishes information about the temperature sensor data
 */
void Network::PublishTemperatureData()
{
    prevMillisPublishTemperatureData = millis();

    // == Home Assistant
    PublishHomeassistantTemperatureData();

    // == JSON
    PublishJsonTemperatureData();
}

/**
 * Publishes information about the network interface  
 */
void Network::PublishNetwork()
{
    prevMillisPublishNetwork = millis();

    // == Home Assistant
    PublishHomeassistantNetwork();

    // == JSON
    PublishJsonNetwork();
}

/**
 * Publishes the current installed code version 
 */
void Network::PublishCodeVersion()
{
    mqttClient.publish(("SaunaController/" + data.mqttClientName + "/Version").c_str(), codeVersion.c_str());
} 
