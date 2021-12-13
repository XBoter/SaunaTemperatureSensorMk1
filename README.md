# SaunaTemperatureSensorMk1

![build](https://github.com/XBoter/SaunaTemperatureSensorMk1/workflows/build/badge.svg)

## Features

- On power up measures the internal temperature of the sauna with a DS18B20 temperature sensor
- Sends every 5 seconds an temperature update via mqtt
- Flash storage of configuration against power lose
- Webpage based configuration via access point
- Controllable via Homeassistant and JSON
- Programmed in C++ and Arduino via VS Code
- Nodemcu ESP8266 micro controller with WiFi
- OTA Sketch updates possible

## Quick Start

#### 1. Flash ESP

#### 2. Configure ESP under 192.168.1.1 with your network and MQTT broker

#### 3. Use Homeassistant or JSON temperature data over MQTT

## MQTT paths for temperature data access

### Normal Value

```
"SaunaController/INSERT_MQTT_CLIENT_NAME/HomeAssistant/Temperature/Celcius/value"
"SaunaController/INSERT_MQTT_CLIENT_NAME/HomeAssistant/Temperature/Fahrenheit/value"
```

### JSON

```
"SaunaController/INSERT_MQTT_CLIENT_NAME/JSON/Temperature"
```

#### Data template

```
{
 "celsius": 42.0,
 "fahrenheit": 69.0
}
```

## Homeassistant

### Sensor

- Example MQTT temperature sensor in Homeassistant YAML

```
- platform: mqtt
  name: Sauna Temperature
  unit_of_measurement: "°C"
  state_topic: "SaunaController/INSERT_MQTT_CLIENT_NAME/HomeAssistant/Temperature/Celcius/value"
```

### Card

- Example Homeassistant sensor card in YAML

```
- type: sensor
  entity: sensor.sauna_temperature
  unit: "°C"
  graph: line
  detail: 2
  hours_to_show: 2
  name: Sauna Temperature
```

## Warning

I am not responsible for any failure or fire, destruction or other damage caused by the Sauna controller.
Use at your own risk

## Buy me a coffee or sponsor the project

[![PayPal](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=5UD82M4V6M2XC)
