#include "Information.h"

Information::Information(){

};

/**
 * Sets reference to external components
 */
void Information::setReference(Network *network,
                               Network *memNetwork,
                               TemperatureSensor *temperatureSensor,
                               TemperatureSensor *memTemperatureSensor)
{
    this->network = network;
    this->memNetwork = memNetwork;
    this->temperatureSensor = temperatureSensor;
    this->memTemperatureSensor = memTemperatureSensor;
};

/**
 * Does init stuff for the Information component
 * 
 * @return True if successfull, false if not 
 */
bool Information::Init()
{
    if (!init)
    {
        Serial.println(F("Information initialized"));
        init = true;
    }

    return init;
};

/**
 * Runs the Information component. 
 */
void Information::Run()
{
    if (!init)
    {
        return;
    }

    // ---- Check for changes in Network parameter to memNetwork parameter
    // -- Time
    if (this->network->stNetworkTimeData.hour != memNetwork->stNetworkTimeData.hour ||
        this->network->stNetworkTimeData.minute != memNetwork->stNetworkTimeData.minute ||
        this->network->stNetworkTimeData.second != memNetwork->stNetworkTimeData.second ||
        this->network->stNetworkTimeData.unix != memNetwork->stNetworkTimeData.unix)
    {
        FormatPrintTime("Time",
                        String(this->network->stNetworkTimeData.hour),
                        String(this->network->stNetworkTimeData.minute),
                        String(this->network->stNetworkTimeData.second),
                        String(this->network->stNetworkTimeData.unix));

        memNetwork->stNetworkTimeData.hour = this->network->stNetworkTimeData.hour;
        memNetwork->stNetworkTimeData.minute = this->network->stNetworkTimeData.minute;
        memNetwork->stNetworkTimeData.second = this->network->stNetworkTimeData.second;
        memNetwork->stNetworkTimeData.unix = this->network->stNetworkTimeData.unix;
    }

    // -- Network WiFi
    if (this->network->wifiConnected != memNetwork->wifiConnected ||
        this->network->hostname != memNetwork->hostname ||
        this->network->ipAddress != memNetwork->ipAddress ||
        this->network->subnetmask != memNetwork->subnetmask ||
        this->network->macAddress != memNetwork->macAddress ||
        this->network->gateway != memNetwork->gateway)
    {
        FormatPrintNetworkWiFi(BollToConnectionState(this->network->wifiConnected),
                               this->network->hostname,
                               this->network->ipAddress,
                               this->network->subnetmask,
                               this->network->macAddress,
                               this->network->gateway);

        memNetwork->wifiConnected = this->network->wifiConnected;
        memNetwork->hostname = this->network->hostname;
        memNetwork->ipAddress = this->network->ipAddress;
        memNetwork->subnetmask = this->network->subnetmask;
        memNetwork->macAddress = this->network->macAddress;
        memNetwork->gateway = this->network->gateway;
    }

    // -- Network MQTT
    if (this->network->mqttConnected != memNetwork->mqttConnected ||
        this->network->clientState != memNetwork->clientState ||
        this->network->brokerIpAddress != memNetwork->brokerIpAddress ||
        this->network->brokerPort != memNetwork->brokerPort ||
        this->network->clientName != memNetwork->clientName)
    {
        FormatPrintNetworkMQTT(BollToConnectionState(this->network->mqttConnected),
                               IntToClientStateInfo(this->network->clientState),
                               this->network->brokerIpAddress,
                               this->network->brokerPort,
                               this->network->clientName);

        memNetwork->mqttConnected = this->network->mqttConnected;
        memNetwork->clientState = this->network->clientState;
        memNetwork->brokerIpAddress = this->network->brokerIpAddress;
        memNetwork->brokerPort = this->network->brokerPort;
        memNetwork->clientName = this->network->clientName;
    }

    // -- Temperature Celsius
    if (!this->isEqual(this->temperatureSensor->getTemperatureCelsius(), this->memTemperatureSensor->getTemperatureCelsius()))
    {
        FormatPrintTemperature(String(this->temperatureSensor->getTemperatureCelsius()), "Celsius");

        this->memTemperatureSensor->setTemperatureCelsius(this->temperatureSensor->getTemperatureCelsius());
    }

    // -- Temperature Fahrenheit
    if (!this->isEqual(this->temperatureSensor->getTemperatureFahrenheit(), this->memTemperatureSensor->getTemperatureFahrenheit()))
    {
        FormatPrintTemperature(String(this->temperatureSensor->getTemperatureFahrenheit()), "Fahrenheit");

        this->memTemperatureSensor->setTemperatureFahrenheit(this->temperatureSensor->getTemperatureFahrenheit());
    }
};

/**
 * Prints a single parameter/component formatted message to serial
 * 
 * @param name      The name of the parameter or component
 * @param value     The value of the given parameter or component
 **/
void Information::FormatPrintSingle(String name,
                                    String value)
{
    TopSpacerPrint();

    // Paramter name
    InsertPrint();
    Serial.print(F("Paramter/Component/Mode '"));
    Serial.print(name);
    Serial.println(F("' changed"));

    // Parameter value
    InsertPrint();
    Serial.print(F("Value : "));
    Serial.println(value);

    BottomSpacerPrint();
};

/**
 * Prints a time formatted message to serial
 * 
 * @param name      The name of the parameter or component
 * @param hour      The value of the current hour
 * @param minute    The value of the current minute
 * @param second    The value of the current second
 **/
void Information::FormatPrintTime(String name,
                                  String hour,
                                  String minute,
                                  String second,
                                  String unix)
{
    TopSpacerPrint();

    // Paramter name
    InsertPrint();
    Serial.print(F("Paramter/Component/Mode '"));
    Serial.print(name);
    Serial.println(F("' changed"));

    // Parameter value
    InsertPrint();
    Serial.print(F("Hour    : "));
    Serial.println(hour);
    Serial.print(F("Minute  : "));
    Serial.println(minute);
    Serial.print(F("Second  : "));
    Serial.println(second);
    Serial.print(F("Unix    : "));
    Serial.println(unix);

    BottomSpacerPrint();
};

/**
 * Prints a temperature data formatted message to serial
 * 
 * @param temperature    The value of the current temperature
 * @param unit           The unit of the temperature
**/
void Information::FormatPrintTemperature(String temperature,
                                         String unit)
{
    TopSpacerPrint();

    // Value name
    InsertPrint();
    Serial.print(F("Temperature changed "));

    // Parameter value
    InsertPrint();
    Serial.print(F("Value : "));
    Serial.print(temperature);
    Serial.print(F(" "));
    Serial.println(unit);

    BottomSpacerPrint();
};

/**
 * Prints a network wifi formatted message to serial
 * 
 * @param connectionState   The current connection state of the WiFi connection
 * @param hostname          The current hostname of the client if connected
 * @param ipAddress         The current ip address of the esp if connected
 * @param subnetmask        The current subnetmask of the esp if connected
 * @param macAddress        The current macAddress of the esp
 * @param gateway           The current gateway of the esp if connected
 **/
void Information::FormatPrintNetworkWiFi(String connectionState,
                                         String hostname,
                                         String ipAddress,
                                         String subnetmask,
                                         String macAddress,
                                         String gateway)
{
    TopSpacerPrint();

    // Network WiFi changed
    InsertPrint();
    Serial.println(F("Network WiFi changed"));

    // Connection State
    InsertPrint();
    Serial.print(F("Connection State        : "));
    Serial.println(connectionState);

    // Hostname
    InsertPrint();
    Serial.print(F("Hostname                : "));
    Serial.println(hostname);

    // IP Address
    InsertPrint();
    Serial.print(F("IP Address              : "));
    Serial.println(ipAddress);

    // Subnetmask
    InsertPrint();
    Serial.print(F("Subnetmask              : "));
    Serial.println(subnetmask);

    // Mac Address
    InsertPrint();
    Serial.print(F("Mac Address             : "));
    Serial.println(macAddress);

    // gateway
    InsertPrint();
    Serial.print(F("Gateway                 : "));
    Serial.println(gateway);

    BottomSpacerPrint();
};

/**
 * Prints a network mqtt formatted message to serial
 * 
 * @param connectionState   The current connection state of the mqtt connection
 * @param clientStateInfo   The current client state info
 * @param brokerIpAddress   The broker ip address to connect to
 * @param clientName        The mqtt client name
 **/
void Information::FormatPrintNetworkMQTT(String connectionState,
                                         String clientStateInfo,
                                         String brokerIpAddress,
                                         int brokerPort,
                                         String clientName)
{
    TopSpacerPrint();

    // Network MQTT changed
    InsertPrint();
    Serial.println(F("Network MQTT changed"));

    // Connection State
    InsertPrint();
    Serial.print(F("Connection State        : "));
    Serial.println(connectionState);

    // Client State Info
    InsertPrint();
    Serial.print(F("Client State Info       : "));
    Serial.println(clientStateInfo);

    // Broker IP Address
    InsertPrint();
    Serial.print(F("Broker IP Address       : "));
    Serial.println(brokerIpAddress);

    // Broker Port
    InsertPrint();
    Serial.print(F("Broker Port             : "));
    Serial.println(brokerPort);

    // Client Name
    InsertPrint();
    Serial.print(F("Client Name             : "));
    Serial.println(clientName);

    BottomSpacerPrint();
};

/**
 * Prints a formatted top spacer message to serial
 **/
void Information::TopSpacerPrint()
{
    Serial.println(F(""));
    Serial.print(" # ");
    for (uint8_t i = 0; i < spacerLength; i++)
    {
        Serial.print(symbol);
    }
    Serial.print(" #");
    Serial.println(F(""));
};

/**
 * Prints a formatted bottom spacer message to serial
 **/
void Information::BottomSpacerPrint()
{
    Serial.print(" # ");
    for (uint8_t i = 0; i < spacerLength; i++)
    {
        Serial.print(symbol);
    }
    Serial.print(" #");
    Serial.println(F(""));
};

/**
 * Converts a given bool value to the corresponding string message
 * 
 * @param b     The bool value to convert
 * 
 * @return The value of the bool as string
 **/
String Information::BoolToString(bool b)
{
    if (b)
    {
        return "true";
    }
    else
    {
        return "false";
    }
};

/**
 * Converts a given bool value to the corresponding connection state
 * 
 * @param b     The bool value to convert
 * 
 * @return The value of the bool as connection state
 **/
String Information::BollToConnectionState(bool b)
{
    if (b)
    {
        return "Connected";
    }
    else
    {
        return "Disconnected";
    }
};

/**
 * Converts a given int value to the corresponding client state info
 * 
 * @param i     The int value to convert
 * 
 * @return The value of the int as client state info
 **/
String Information::IntToClientStateInfo(int i)
{
    switch (i)
    {

    case -4:
        return "Connection Timeout";

    case -3:
        return "Connection Lost";

    case -2:
        return "Connect Failed";

    case -1:
        return "Disconnected";

    case 0:
        return "Connected";

    case 1:
        return "Connect Bad Protocol";

    case 2:
        return "Connect Bad Client ID";

    case 3:
        return "Connect Unavailable";

    case 4:
        return "Connect Bad Credentials";

    case 5:
        return "Connect Unauthorized";

    default:
        return "Unknown";
    }
};

/**
 * Prints a string message with length of insertLength * spaces to serial
 **/
void Information::InsertPrint()
{
    for (uint8_t i = 0; i < insertLength; i++)
    {
        Serial.print("");
    }
};

/**
 * @brief Compares to floating numbers and returns true if they are equal
 * 
 * @param f1 
 * @param f2 
 * @return boolean true when equal
 */
boolean Information::isEqual(float f1, float f2)
{
    return ((int)(f1 * 100)) == ((int)(f2 * 100));
}