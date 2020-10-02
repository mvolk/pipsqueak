# Pipsqueak Version 3

This, the current Pipsqueak hardware version, is designed with the intent of being
able to replace the controller in SS Brewtech's FTSS system. As such, it handles
peripheral currents up to 7 amps and main current up to 8 amps. An onboard
temperature sensor is added to monitor for overheating, and the duty cycle is
intended to be modest.

## Contents

### [PIO_Arduino_OS](./PIO_Arduino_OS/README.md)

The Pipsqueak v3 operating system. This is the program that is run on a Pipsqueak v3
when it is in normal operation.

### [PIO_Arduino_Initializer](./PIO_Arduino_Initializer/README.md)

Writes configuration settings to the non-volatile memory of a Pipsqueak v3 device.
Typically flashed to a Pipsqueak v3 after a hardware test and prior to flashing
the operating system.

### [PIO_Arduino_Wiper](./PIO_Arduino_Wiper/README.md)

Clears non-volatile memory so that the device can be repurposed, re-homed, or
recycled.

### [PIO_Arduino_HardwareTest](./PIO_Arduino_HardwareTest/README.md)

Basic smoke test for new hardware.

## Future Contents

The follow content will be added in the future:
* EAGLE files and related hardware design documentation for the sensor module,
power module and programmer
* STL files for a "production" device, device used for development, and for the
programmer.
 
