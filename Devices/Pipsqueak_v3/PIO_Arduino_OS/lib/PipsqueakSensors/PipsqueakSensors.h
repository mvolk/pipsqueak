#ifndef PipsqueakSensors_h
#define PipsqueakSensors_h

#include <Arduino.h>
#include <PipsqueakState.h>
#include <OneWire.h>
#include <DS18B20.h>

// Un-comment to enable detailed debug statements
// #define DEBUG_PIPSQUEAK_SENSORS true

/**
 * Manages coordination between and collection/distribution
 * of readings from, the Pipsqueak's temperature sensors.
 */
class PipsqueakSensors {
  public:
    PipsqueakSensors(PipsqueakState * state);

    /**
     * Initializes the OneWire bus.
     * Invoke in the main program's setup() function.
     * Call only after invoking PipsqueakState.setup().
     */
    void setup();

    /**
     * Detects sensors, takes measurements, and updates
     * PipsqueakState.
     *
     * Invoke in the main program's loop() function.
     */
    void loop();

  private:
    PipsqueakState * _state;
    PipsqueakConfig * _config;
    OneWire * _oneWire;
    DS18B20 * _boardSensor;
    DS18B20 * _remoteSensor;
    bool _sensorToggle;

    void detectSensors();
};

#endif // PipsqueakSensors_h
