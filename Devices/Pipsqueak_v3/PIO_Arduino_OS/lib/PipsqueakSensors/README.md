# Pipsqueak Sensors Library

Manages coordination between and collection/distribution of readings from, the
Pipsqueak's temperature sensors.

This library leverages [this DS18B20](../DS18B20/README.md) library.

## Usage

* Construct PipsqueakSensors with the singleton
  [PipsqueakState](../PipsqueakState/README.md) instance.
* Invoke PipsqueakSensors.setup() in the main program's setup()
  method after first invoking the setup() method of the
  PipsqueakState.
* Invoke PipsqueakSensors.loop() in the main program's loop()
  method.
