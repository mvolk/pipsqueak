# Pipsqueak Config Library

Pipsqueak devices have durable memory that, while not technical EEPROM,
is accessed via the Arduino EEPROM library. This memory is initialized
once using a specialized Arduino sketch that is executed prior to flashing
the operating system onto the device. The operating system reads this
memory to obtain device-specific values, WiFi credentials, the address
of the server, etc. The OS writes to this memory when the temperature
setpoint changes so that the device can perform its temperature control
function after reboot even if WiFi or the server are down, as might
happen immediately following a power interruption for example.

This library encapsulates reads and writes to the "EEPROM" memory,
providing an API that the rest of the operating system can use to
access and update durable configuration.

## Data Layout

Version 3 of the configuration memory layout is used in Pipsqueak v3
devices. Of the available 512 bytes of memory, 256 are allocated for
configuration storage. Layout is as follows, where units of measurement
are bytes:

| Start     | Size   | Type      | Description
| --------- | ------ | --------- | ----------------------------------------------------
| 0         | 1      | uint8     | EEPROM previously written flag (0x0F if previously written, random value unknown if never written)
| 1         | 1      | uint8     | EEPROM schema version
| 2         | 1      | uint8     | Config flags - not relevant for Pipsqeakv3
| 3         | 1      | ---       | reserved
| 4         | 4      | uint32    | EEPROM write count - number of times that the EEPROM has been updated.
| 8         | 4      | uint32    | deviceID
| 12        | 4      | float     | setpoint in celsius
| 16        | 4      | uint8[4]  | host server's ipv4 address, expressed in octets
| 20        | 2      | uint16    | host server's port
| 22        | 32     | char[32]  | wifi ssid, up to 32 characters including null termination
| 54        | 64     | char[64]  | wifi password, at most 64 characters including null termination
| 118       | 32     | uint8[32] | secret encryption key specific to this device
| 150       | 1      | uint8     | signal enable GPIO pin number
| 151       | 1      | uint8     | 1Wire GPIO pin number for board sensor (not presently used)
| 152       | 8      | uint8[8]  | 1Wire address of onboard DS18B20 temperature sensor
| 160       | 1      | uint8     | 1Wire GPIO pin number for remote sensor
| 161       | 8      | uint8[8]  | 1Wire address of onboard DS18B20 temperature sensor (not presently used)
| 169       | 1      | uint8     | Red LED indicator signal GPIO pin number
| 170       | 1      | uint8     | Green LED indicator signal GPIO pin number
| 171       | 1      | uint8     | Heater pin count (n) - should be exactly 1 for Pipsqueak v3
| 172       | n      | uint8     | Heater signal GPIO pin numbers
| 172+n     | 1      | uint8     | Chiller pin count (m) - should be exactly 1 for Pipsqueak v3
| 172+n+1   | m      | uint8     | Chiller signal GPIO pin numbers

Note that n + m are limited to 83 in order to remain within the 256 byte
allocation block. Both should be 1 for Pipsqueak v3, however.

## Useage

Ensure that the setup() function is called before any other method is invoked.

See API details in the [PipsqueakConfig header file](./PipsqueakConfig.h).
