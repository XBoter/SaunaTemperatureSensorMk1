#pragma once

// Arduino Lib Includes
#include <OneWire.h> // @installed via Arduino Library Manger    GitHub => https://github.com/PaulStoffregen/OneWire

// Includes
#include <Arduino.h>
#include "../Network/Network.h"

// Interface
#include "../Interface/IBaseClass.h"

// Blueprint for compiler. Problem => circular dependency
class Network;

// Classes
class TemperatureSensor : public IBaseClass
{
    // ================================ Constructor / Important ================================ //
public:
    TemperatureSensor();
    void setReference(Network *network);
    bool init = false;

    // ================================ Interface ================================ //
private:
public:
    virtual bool Init();
    virtual void Run();

    // ================================ Data ================================ //
private:
    Network *network; // Pointer to used network instance
    byte address[8];
    byte data[12];
    byte i = 0;
    float temperatureCelsius = 0.0;
    float temperatureFahrenheit = 0.0;
    bool foundTemperatureSensor = false;
    int state = 0;
    int maxTempData = 5;
    float rawTempData[5];
    int counterTempData = 0;

    // Prev Millis
    unsigned long PrevMillis_NextDataRead = 0;

    // Timeout
    const unsigned long TimeOut_NextDataRead = 1000; // 5 sec

public:
    // ================================ Methods ================================ //
private:
public:
    float getTemperatureCelsius();
    void setTemperatureCelsius(float temperatureCelsius);
    float getTemperatureFahrenheit();
    void setTemperatureFahrenheit(float temperatureFahrenheit);
};