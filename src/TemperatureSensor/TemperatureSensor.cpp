#include "TemperatureSensor.h"

// OneWire instance for reading temperature sensor
OneWire oneWire(4);

/**
 * Empty constructor
 */
TemperatureSensor::TemperatureSensor(){};

/**
 * Sets reference to external components
 */
void TemperatureSensor::setReference(Network *network)
{
    this->network = network;
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

        oneWire.reset_search();
        this->foundTemperatureSensor = oneWire.search(this->address);

        if (this->foundTemperatureSensor)
        {
            Serial.print(F("Found DS18B20 temperature sensor with address: 0x"));
            for (i = 0; i < 8; i++)
            {
                Serial.print(this->address[i], HEX);
            }
            Serial.println("");

            // Check CRC
            if (OneWire::crc8(this->address, 7) != this->address[7])
            {
                Serial.println("CRC of DS18B20 is invalid!");
                this->foundTemperatureSensor = false;
            }
        }
        else
        {
            Serial.println(F("No DS18B20 temperature sensor found!"));
        }

        Serial.println(F("Temperature sensor initialized"));
        init = true;
    }

    return init;
};

/**
 * Runs the Temperature Sensor component. 
 */
void TemperatureSensor::Run()
{

    unsigned long curMillis = millis();

    if (this->foundTemperatureSensor)
    {

        byte cfg = 0;
        int16_t raw = 0;

        switch (this->state)
        {

        // ================================ Reset and start conversion ================================ //
        case 0:
            oneWire.reset();
            oneWire.select(this->address);
            oneWire.write(0x44, 1);
            this->state = 10;
            this->PrevMillis_NextDataRead = curMillis;
            break;

        // ================================ Wait for coneversion to finish ================================ //
        case 10:
            if (curMillis - this->PrevMillis_NextDataRead >= this->TimeOut_NextDataRead)
            {
                this->state = 20;
            }
            break;

        // ================================ Read scratchpad and convert data ================================ //
        case 20:
            oneWire.reset();
            oneWire.select(this->address);
            oneWire.write(0xBE);

            for (i = 0; i < 9; i++)
            {
                this->data[i] = oneWire.read();
            }

            raw = (this->data[1] << 8) | this->data[0];
            cfg = (this->data[4] & 0x60);
            if (cfg == 0x00)
            {
                raw = raw & ~7;
            }
            else if (cfg == 0x20)
            {
                raw = raw & ~3;
            }
            else if (cfg == 0x40)
            {
                raw = raw & ~1;
            }

            if (this->counterTempData >= 0 && this->counterTempData < this->maxTempData)
            {
                this->rawTempData[this->counterTempData] = raw;
            }

            this->state = 30;
            break;

        // ================================ Check if we are ready to send data ================================ //
        case 30:
            this->counterTempData++;
            if (this->counterTempData >= this->maxTempData)
            {
                this->counterTempData = 0;
                this->state = 40;
            }
            else
            {
                this->state = 0;
            }
            break;

        // ================================ Send average temp data ================================ //
        case 40:
            this->temperatureCelsius = 0;
            for (i = 0; i < this->maxTempData; i++)
            {
                this->temperatureCelsius += this->rawTempData[i];
            }
            this->temperatureCelsius = (float)this->temperatureCelsius / this->maxTempData / 16.0;
            this->temperatureFahrenheit = this->temperatureCelsius * 1.8 + 32.0;

            this->network->PublishTemperatureData();

            this->state = 0;
            break;
        }
    }
    else
    {
        this->state = 0;
        this->temperatureCelsius = 0.0;
        this->temperatureFahrenheit = 0.0;
    }
};

/**
 * @brief Returns the current temperature in Celsius
 * 
 * @return Temperature Celsius as float
 */
float TemperatureSensor::getTemperatureCelsius()
{

    return this->temperatureCelsius;
}

/**
 * @brief Returns the current temperature in Fahrenheit
 * 
 * @return Temperature Fahrenheit as float
 */
float TemperatureSensor::getTemperatureFahrenheit()
{
    return this->temperatureFahrenheit;
}

/**
 * @brief Sets the current temperature in Celsius
 * 
 * @param temperatureCelsius 
 */
void TemperatureSensor::setTemperatureCelsius(float temperatureCelsius)
{
    this->temperatureCelsius = temperatureCelsius;
}

/**
 * @brief Sets the current temperature in Fahrenheit
 * 
 * @param temperatureFahrenheit 
 */
void TemperatureSensor::setTemperatureFahrenheit(float temperatureFahrenheit)
{
    this->temperatureFahrenheit = temperatureFahrenheit;
}