# Pipsqueak Controller Library

Not a controller as in MVC, but rather a temperature controller - this libary
controls the state of a peripheral heater and chiller to regulate the temperature
of a ferment (or other medium), keeping that temperature to within close tolerances
of the setpoint.

At present, this is an overly simplistic "bang bang" controller, not a more
sophisticated PID algorithm. Thus, the parameters hard-coded in this controller, while
suitable for the reference design hardware and one-gallon ferment, are likely to
perform poorly with alternative hardware and fermentation batch sizes.

## Usage

* Construct PipsqueakController with the singleton
  [PipsqueakState](../PipsqueakState/README.md) instance.
* Invoke PipsqueakController.setup() in the main program's setup()
  method after first invoking the setup() method of the
  PipsqueakState.
* Enable signals in the setup function:
    ``` cpp
    pinMode(state->getConfig()->getSignalEnablePin(), OUTPUT);
    digitalWrite(state->getConfig()->getSignalEnablePin(), HIGH);
    ```
* Invoke PipsqueakController.loop() in the main program's loop()
  method.
