#include "SaunaTemperatureSensorMk1.h"

SaunaTemperatureSensorMk1::SaunaTemperatureSensorMk1()
{
}

/**
 * Setup function for Arduino file to call in setup
 */
void SaunaTemperatureSensorMk1::_setup()
{
    // Setup Serial
    Serial.begin(BAUDRATE);
    delay(100);

    // Info and Version print
    Serial.println("# ======================== #");
    Serial.print("Name          : ");
    Serial.println(Name);
    Serial.print("Programmer    : ");
    Serial.println(Programmer);
    Serial.print("Created       : ");
    Serial.println(Created);
    Serial.print("Last Modifed  : ");
    Serial.println(LastModifed);
    Serial.print("Version       : ");
    Serial.println(Version);
    Serial.println("# ======================== #");
    Serial.println("");

    // Set references to external components
    ota.setReference(&network,
                     &configuration);

    network.setReference(&configuration,
                         &information,
                         &temperatureSensor);

    temperatureSensor.setReference(&network);

    information.setReference(&network,
                             &memNetwork,
                             &temperatureSensor,
                             &memTemperatureSensor);

    configuration.setReference();

    // Init configuration
    configuration.Init();

    Serial.println("# ==== Setup finished ==== #");
    Serial.println("");
};

/**
 * Loop function for Arduino file to call in loop
 */
void SaunaTemperatureSensorMk1::_loop()
{
    //  Messure loop time
    /*
    unsigned long curMicros = micros();
    Serial.println("Loop time : " + String(curMicros - PrevMicros_Loop));
    PrevMicros_Loop = curMicros;
    */

    // Run configuration before all other components
    configuration.Run();
    if (configuration.isFinished)
    {
        switch (state)
        {
        case 0:
            // Reset init flag
            ota.init = false;
            temperatureSensor.init = false;
            network.init = false;
            information.init = false;
            state++;
            break;

        case 1:
            // Init all components
            ota.Init();
            temperatureSensor.Init();
            network.Init();
            information.Init();
            state++;
            break;

        case 2:
            // Run all components
            ota.Run();
            yield(); 
            temperatureSensor.Run();
            yield(); 
            network.Run();
            yield(); 
            information.Run();
            yield(); 

            break;
        }
    }
    else
    {
        state = 0;
    }
};