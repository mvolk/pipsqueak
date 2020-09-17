/**
 * Interim replacement for <Arduino.h> that supports native unit
 * testing of code that relies on a limited subset of Arduino features.
 *
 * When compiling for the Arduino framework, including this header is
 * equivalent to including <Arduino.h>.
 *
 * In the longer run, time permitting (!), a better solution may be
 * a custom platform and board that simulates (not emulates) the ESP8266
 * package and WEMOS D1 Mini board (the relevant package & board for
 * this project). That, however, is big chunk of work that is
 * tangential to current priorities. #TechDebt
 */

#ifndef ArduinoAdapter_h
#define ArduinoAdapter_h

#ifdef ARDUINO

#include <Arduino.h>

#else // Not ARDUINO

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

// Determines the max number of loop calls
// Actual count could be less due to exceptions/crashes
#define MAX_LOOP_COUNT 1

void setup(void);
void loop(void);
void yield(void);

#define HIGH 0x1
#define LOW  0x0

typedef uint8_t byte;

#endif // ARDUINO

#endif // ArduinoAdapter.h
