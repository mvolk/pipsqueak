# Pipsqueak v3 Hardware Test

This simple program is designed as a "smoke test" for new boards. It
enables signals (D1/GPIO5 => HIGH), illuminates the green LED
(D6/GPIO12 => HIGH) and alternately turns on the heater
and chiller outputs (D8/GPIO15 & D5/GPIO14 => HIGH/LOW). It illuminates
the red LED (D7/GPIO13) just prior to alternating the heater and chiller
outputs, and leaves it on until just after the switch is complete.

Note that the board must be provided with 12V power in order for the
yellow and blue LEDs to light when the heater and chiller outputs,
respectively, are on.
