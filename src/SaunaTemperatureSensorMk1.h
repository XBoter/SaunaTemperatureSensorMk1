#pragma once

#include "Configuration/Configuration.h"
#include "Enums/Enums.h"
#include "Information/Information.h"
#include "Interface/IBaseClass.h"
#include "Network/Network.h"
#include "OTA/OTA.h"
#include "Structs/Structs.h"
#include "TemperatureSensor/TemperatureSensor.h"

//-------------------- Basic Information --------------------//
#define Name "Sauna Temperature Sensor Mk1"
#define Programmer "Nico Weidenfeller"
#define Created "11.12.2021"
#define LastModifed "12.12.2021"
#define Version "1.0.0"

//++++ Global Defines ++++//
#define BAUDRATE 115200
#define DS18B20_TEMPERATURE_SENSOR_PIN D2

class SaunaTemperatureSensorMk1
{

public:
    SaunaTemperatureSensorMk1();
    void _setup();
    void _loop();

private:
    // ## Data ## //
    unsigned long PrevMicros_Loop = 0;
    uint8_t state = 0;

    // ## Components ## //
    Configuration configuration = Configuration();
    TemperatureSensor temperatureSensor = TemperatureSensor();
    TemperatureSensor memTemperatureSensor = TemperatureSensor();
    Network network = Network(Version);
    Network memNetwork = Network(Version); // Memory Version Instance for Information
    OTA ota = OTA();
    Information information = Information();
};
