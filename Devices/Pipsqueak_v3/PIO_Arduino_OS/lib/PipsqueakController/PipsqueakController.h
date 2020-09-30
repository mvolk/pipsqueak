#ifndef PipsqueakController_h
#define PipsqueakController_h

#include <Arduino.h>
#include <PipsqueakState.h>

// Un-comment to enable detailed debug logging
// #define DEBUG_PIPSQUEAK_CONTROLLER

class PipsqueakController {
  public:
    PipsqueakController(PipsqueakState * state);
    void setup();
    void loop();

  private:
    PipsqueakState * _state;
    PipsqueakConfig * _config;
    bool _heating;
    bool _chilling;
    uint32_t _pulseDuration;
    uint32_t _recoveryDuration;
    uint32_t _lastToggled;

    bool isRunning();
    bool shouldHeat();
    void heaterPulse();
    void modulateHeater();
    bool shouldChill();
    void chillerPulse();
    bool isRecovering();
    bool shouldStopRunning();
    void stopRunning();
    float lowerLimit();
    float upperLimit();
};

#endif // PipsqueakController_h
