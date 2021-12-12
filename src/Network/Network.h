#pragma once

// Arduino Lib Includes
#include <NTPClient.h>    // @installed via Arduino Library Manger    GitHub => https://github.com/arduino-libraries/NTPClient
#include <PubSubClient.h> // @installed via Arduino Library Manger    GitHub => https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>  // @installed via Arduino Library Manger    GitHub => https://github.com/bblanchon/ArduinoJson

// Includes
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "../Enums/Enums.h"
#include "../Structs/Structs.h"
#include "../Configuration/Configuration.h"
#include "../Information/Information.h"
#include "../TemperatureSensor/TemperatureSensor.h"

// Interface
#include "../Interface/IBaseClass.h"

// Blueprint for compiler. Problem => circular dependency
class Configuration;
class Information;
class TemperatureSensor;

// Classes
class Network : public IBaseClass
{
    // ## Constructor / Important ## //
public:
    Network(String codeVersion);
    void setReference(Configuration *configuration,
                      Information *information,
                      TemperatureSensor *temperatureSensor);
    bool init = false;

    // ## Interface ## //
private:
public:
    virtual bool Init();
    virtual void Run();

    // ## Data ## //
private:
    // External components
    Configuration *configuration;
    Information *information;
    TemperatureSensor *temperatureSensor;

    ConfiguredData data = {};

    WiFiClient wifiMqtt;
    WiFiUDP ntpUDP;
    const long utcOffsetInSeconds = 3600; // UTC +1 (Germany) => 1 * 60 * 60 => 3600
    NTPClient timeClient = NTPClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

    // Prev Millis
    unsigned long PrevMillis_WiFiTimeout = 0;
    unsigned long PrevMillis_MQTTTimeout = 0;
    unsigned long PrevMillis_NTPTimeout = 0;
    unsigned long PrevMillis_HeartbeatTimeout = 0;

    // Timeout
    const unsigned long TimeOut_WiFiTimeout = 5000;      // 5 sec
    const unsigned long TimeOut_MQTTTimeout = 5000;      // 5 sec
    const unsigned long TimeOut_NTPTimeout = 60000;      // 60 sec
    const unsigned long TimeOut_HeartbeatTimeout = 5000; // 5 sec

    bool wifiOneTimePrint = true;
    bool mqttOneTimePrint = true;
    bool memWifiConnected = false;
    bool memMqttConnected = false;

    String codeVersion = "";

public:
    DynamicJsonDocument doc = DynamicJsonDocument(2048);
    PubSubClient mqttClient;
    NetworkWiFiState wifiState = NetworkWiFiState::StartWifi;
    NetworkMQTTState mqttState = NetworkMQTTState::StartMqtt;
    bool wifiConnected = false;
    bool mqttConnected = false;

    // Network WiFi Info
    String ipAddress = "";
    String macAddress = "";
    String subnetmask = "";
    String gateway = "";
    String hostname = "";

    // Network MQTT Info
    String clientName = "";
    String brokerIpAddress = "";
    int brokerPort = -1; // Unknown
    int clientState = -99; // Unknown

    // MQTT Data
    // Time
    NetworkTimeData stNetworkTimeData = {};

    // ==== Republish / Publish functions
    unsigned long prevMillisPublishTemperatureData = 0;
    unsigned long prevMillisPublishHeartbeat = 0;
    unsigned long prevMillisPublishNetwork = 0;

    uint32_t timeoutPublishTemperatureData = 5000;  // 5 Seconds
    uint32_t timeoutPublishHeartbeat = 5000;        // 5 Seconds
    uint32_t timeoutPublishNetwork = 600000;        // 10 Minutes

    // ## Functions ## //
private:
    // Network handles
    void HandleWifi();
    void HandleMqtt();
    void HandleNTP();

    void MqttCallback(char *topic, byte *payload, unsigned int length);

    // ==== Republish / Publish functions
    // == Handle
    void HandleRepublish();

    // == Homeassistant
    void PublishHomeassistantTemperatureData();
    void PublishHomeassistantNetwork();
    void PublishHomeassistantHeartbeat();

    // == Json
    void PublishJsonTemperatureData();
    void PublishJsonNetwork();
    void PublishJsonHeartbeat();

public:
    // ==== Republish / Publish functions
    void PublishTemperatureData();
    void PublishNetwork();
    void PublishHeartbeat();
    void PublishCodeVersion();
};
