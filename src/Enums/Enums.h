#pragma once

// Enum for the wifi state
enum class NetworkWiFiState
{
    StartWifi,
    SuperviseWiFiConnection,
    CheckWiFiDisconnect,
};


// Enum for the mqtt state
enum class NetworkMQTTState
{
    StartMqtt,
    SuperviseMqttConnection,
    CheckMqttDisconnect,
};

