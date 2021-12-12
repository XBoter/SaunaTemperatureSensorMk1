#pragma once

// Includes
#include <Arduino.h>
#include "../Network/Network.h"
#include "../Enums/Enums.h"
#include "../TemperatureSensor/TemperatureSensor.h"

// Interface
#include "../Interface/IBaseClass.h"

// Blueprint for compiler. Problem => circular dependency
class Network;
class TemperatureSensor;

// Classes
class Information : public IBaseClass
{
    // ## Constructor / Important ## //
public:
    Information();
    void setReference(Network *network,
                      Network *memNetwork,
                      TemperatureSensor *temperatureSensor,
                      TemperatureSensor *memTemperatureSensor);
    bool init = false;

    // ## Interface ## //
private:
public:
    virtual bool Init();
    virtual void Run();

    // ## Data ## //
private:
    TemperatureSensor *temperatureSensor;    // Pointer to used temperatureSensor instance
    TemperatureSensor *memTemperatureSensor; // Memory version of temperatureSensor instance
    Network *network;        // Pointer to used network instance
    Network *memNetwork;     // Memory version of network instance
    const String symbol = "=";
    const uint8_t spacerLength = 40; // x * Spaces
    const uint8_t insertLength = 4;  // x * Spaces
public:
    // ## Functions ## //
private:
    // Format print functions
    void FormatPrintSingle(String name,
                           String paramterValue);

    void FormatPrintTime(String name,
                         String hour,
                         String minute,
                         String second,
                         String unix);

    void FormatPrintTemperature(String temperature,
                                String unit);

    void FormatPrintNetworkWiFi(String connectionState,
                                String hostname,
                                String ipAddress,
                                String subnetmask,
                                String macAddress,
                                String gateway);

    void FormatPrintNetworkMQTT(String connectionState,
                                String clientStateInfo,
                                String brokerIpAddress,
                                int brokerPort,
                                String clientName);

    // Help functions
    void TopSpacerPrint();
    void BottomSpacerPrint();
    void InsertPrint();

public:
    // Help functions
    String BoolToString(bool b);
    String BollToConnectionState(bool b);
    String IntToClientStateInfo(int i);
    boolean isEqual(float f1, float f2);
};
