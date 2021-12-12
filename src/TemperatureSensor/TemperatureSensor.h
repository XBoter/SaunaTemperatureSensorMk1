#pragma once

// Arduino Lib Includes
#include <OneWire.h>    // @installed via Arduino Library Manger    GitHub => https://github.com/PaulStoffregen/OneWire

// Includes
#include <Arduino.h>

// Interface
#include "../Interface/IBaseClass.h"

class TemperatureSensor : public IBaseClass
{
    // ================================ Constructor / Important ================================ //
public:
    TemperatureSensor();
    void setReference();
    bool init = false;

    // ================================ Interface ================================ //
private:
public:
    virtual bool Init();
    virtual void Run();

    // ================================ Data ================================ //
private:
public:
    String temperature;

    // ================================ Methods ================================ //
private:
public:
  
};