# Pipsqueak v3 Operating System

This sub-project contains the operating system code for Pipsqueak v3 devices.

## PlatformIO

This project uses [PlatformIO](https://platformio.org/), an embedded development
platform.

PlatformIO is set up within this project to use the Arduino framework, Espressif
8266 platform and Wemos D1 Mini board.

Pipsqueak v3 devices use an Espressif ESP-12S module with characteristics similar
to the Wemos D1 Mini. Programmers use a CH-340G USB to serial chip and DTR/RTS
reset modeled after the Wemos D1 Mini. As a result, a Pipsqueak v3 device connected
to a USB port via a programmer behaves the same as a Wemos D1 Mini.

## Pipsqueak Libraries

### Hmac

The Pipsqueak Protocol relies on SHA-256 HMACs. This library computes them.

### PipsqueakClient

Pipsqueak devices communicate with a server using a custom binary protocol over
TCP/IP. The [PipsqueakClient](./lib/PipsqueakClient/README.md) encapsulates the
details of this protocol, exposing a relatively simple API for use within the
operating system code.

This library encapsulates the following component libraries:

* [Errors.h](./lib/Errors/README.md) - defines error types and codes
* [Request.h](./lib/Request/README.md) - defines the Request base class
* [Response.h](./lib/Response/README.md) - defines the Response base class
* [TimeProtocol.h](./lib/TimeProtocol/README.md) - defines the TimeRequest and
  TimeResponse classes
* [RebootProtocol.h](./lib/RebootProtocol/README.md) - defines the
  ReportRebootRequest and ReportRebootResponse classes
* [SetpointProtocol.h](./lib/SetpointProtocol/README.md) - defines the
  SetpointRequest and SetpointResponse classes
* [TelemetryProtocol.h](./lib/TelemetryProtocol/README.md) - defines the
  TelemetryRequest and TelemetryResponse classes

## Dependencies

### [Arduino.h](https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Arduino.h)

As with all Arduino applications, this one depends on the Arduino framework. The
code is sourced specifically from the esp8266 implementation.

### [ESP8266WiFi.h](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)

The Arduino implementation for the esp8266 platform includes ESP8266WiFi.h, a
library that exposes the WiFi capabilities of the esp8266. This "comes for free"
with Arduino for the esp8266.

### [user_interface.h](https://github.com/esp8266/Arduino/blob/master/tools/sdk/include/user_interface.h)

Similar to ESP8266WiFi.h, this one is specific to Arduino for esp8266 and comes
for free with PlatformIO's Arduino framework configuration with the Espressif 8266
platform selected. This library provides access to esp8266-specific reboot
cause information used to make exceptions and reboot causes observable.

### [ESPAsyncTCP.h](https://github.com/me-no-dev/ESPAsyncTCP)

Unlike most other low-level TCP client implementation for Arduino esp8266, this
third-party library support asynchronous communications with Interrupt Service
Routine (ISR) callbacks. While trickier to use correctly, this allows the OS
to avoid blocking on requests/responses, allowing functions such as
self-monitoring, temperature observation and temperature control to continue
working even when network conditions cause mayhem for requests and responses.

### [TimeLib.h](https://github.com/PaulStoffregen/Time)

This library is used to maintain a psuedo system clock and obtain UNIX
timestamps.
