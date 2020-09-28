# Pipsqueak State Library

The Pipsqueak State Library provides the [PipsqueakState class](./PipsqueakState.h).
This class is used to share system state and encapsulate common state-related features
such as status event generation.

## Useage

Fundamentally:

1. Call PipsqueakState.setup() in the main program's setup() function
   before passing a reference to any other function/constructor and
   before invoking any method of the class.
2. Call PipsqueakState.loop() in each iteration of the main program's
   loop() function.
3. Use PipsqueakState.setRemoteTemperatureSetpoint(float) and not
   PipsqueakState.getConfig()->setTemperatureSetpoint(float). The
   latter will not generate setpoint update status events.

See API details in the [PipsqueakState header file](./PipsqueakState.h)
and in the [PipsqueakConfig library](../PipsqueakConfig/README.md).
