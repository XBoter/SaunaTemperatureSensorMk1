#include "TemperatureSensor.h"

/**
 * Empty constructor
 */
TemperatureSensor::TemperatureSensor(){
};

/**
 * Sets reference to external components
 */
void TemperatureSensor::setReference()
{
};

/**
 * Does init stuff for the Temperature Sensor component
 * 
 * @return True if successfull, false if not 
 */
bool TemperatureSensor::Init()
{
    if (!init)
    {

        Serial.println(F("Temperature Sensor initialized"));
        init = true;
    }

    return init;
};

/**
 * Runs the Temperature Sensor component. 
 */
void TemperatureSensor::Run()
{
   
};