# Pipsqueak v3 Initializer

This sub-project contains the code that is used to initialize a Pipsqueak's
virtual EEPROM with device-specific configuration settings.

## PlatformIO

This project uses [PlatformIO](https://platformio.org/), an embedded development
platform.

PlatformIO is set up within this project to use the Arduino framework, Espressif
8266 platform and Wemos D1 Mini board.

Pipsqueak v3 devices use an Espressif ESP-12S module with characteristics similar
to the Wemos D1 Mini. Programmers use a CH-340G USB to serial chip and DTR/RTS
reset modeled after the Wemos D1 Mini. As a result, a Pipsqueak v3 device
connected to a USB port via a programmer behaves the same as a Wemos D1 Mini.

# Usage

Update [InitializationParameters.h](./lib/InitializationParameters/InitializationParameters.h)
and [InitializationParameters.cpp](./lib/InitializationParameters/InitializationParameters.cpp),
then build, upload and run the program.

## Libraries

### [DS18B20Bus](./lib/DS18B20Bus/README.md)

Detects DS18B20 temperature sensors on the OneWire bus and configures their resolution.

### [PipsqueakConfig](./lib/PipsqueakConfig/README.md)

Encapsulates reading from and writing to the EEPROM.

### [InitializationParameters](./lib/InitializationParameters/README.md)

The user-modified configuration that drives what gets written to the Pipsqueak's
configuration.

## Dependencies

### [Arduino.h](https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h)

As with all Arduino applications, this one depends on the Arduino framework. The
code is sourced specifically from the esp8266 implementation.

### [OneWire](https://github.com/PaulStoffregen/OneWire)

Used to communicate with Maxim (formerly Texas Instruments) DS18B20 temperature
sensors using the OneWire protocol.
