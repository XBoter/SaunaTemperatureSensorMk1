#pragma once

#include "../Enums/Enums.h"

/**
 * Holds the data for the current time
 */
struct NetworkTimeData
{
    uint8_t hour = 12;
    uint8_t minute = 0;
    uint8_t second = 0;
    unsigned long unix = 0;
};

/**
 * Hold the configuration data
 */
struct ConfiguredData
{
    String wifiSSID = "";
    String wifiPassword = "";
    String mqttBrokerIpAddress = "";
    uint16_t mqttBrokerPort = 0;
    String mqttBrokerUsername = "";
    String mqttBrokerPassword = "";
    String mqttClientName = "";
    bool isConfigured = false;
};
