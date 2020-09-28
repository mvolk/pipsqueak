#ifndef PipsqueakIndicators_h
#define PipsqueakIndicators_h

#include <Arduino.h>
#include <PipsqueakState.h>

// long on - short off
#define BLINK_PATTERN_DASH_DOT 0
// long on - long off
#define BLINK_PATTERN_DASH_DASH 1
// short on - short off
#define BLINK_PATTERN_DOT_DOT 2
// short on - long off
#define BLINK_PATTERN_DOT_DASH 3
// long on - short off - short on - short off
#define BLINK_PATTERN_DASH_DOT_DOT_DOT 4

class PipsqueakIndicators {
  public:
    PipsqueakIndicators(PipsqueakState * state);
    void setup();
    void loop();

  private:
    PipsqueakState * _state;
    PipsqueakConfig * _config;

    uint16_t getPhase(uint16_t duration, uint16_t count);
    uint8_t blink(uint8_t pattern);
};

#endif // PipsqueakIndicators_h
