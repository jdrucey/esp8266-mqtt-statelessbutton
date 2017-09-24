# ESP8266 MQTT Stateless Button Client

An MQTT client for an ESP8266 based board. Takes three stateless button press types: "Single Press", "Double Press" and "Long Press", and sends them to a MQTT server/broker.

These JSON messages include an `eventValue` that matches up to HomeKits "Stateless programable switch" values for simple integration with [HomeBridge](https://github.com/nfarina/homebridge) via the [homebridge-mqtt-statelessswitch](https://github.com/jdrucey/homebridge-mqtt-statelessswitch).

## Uploading
This script depends on the MQTT library by Adafruit, so be sure to download and install the library from here: https://github.com/adafruit/Adafruit_MQTT_Library/

Then simply insert your WiFi details into the variables at the top of the script and upload to your board!