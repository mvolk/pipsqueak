# Pipsqueak Indicators Library

A simple library the controls the state of the green and red indicator LEDs to
signal system status and health.

## Useage

1. Call PipsqueakIndicators.setup() in the main program's setup() function
   after calling PipsqueakState.setup().
2. Enable signals in the setup function:
    ``` cpp
    pinMode(state->getConfig()->getSignalEnablePin(), OUTPUT);
    digitalWrite(state->getConfig()->getSignalEnablePin(), HIGH);
    ```
3. Call PipsqueakIndicators.loop() in each iteration of the main program's
   loop() function.

See API details in the [PipsqueakIndicators header file](./PipsqueakIndicators.h).
