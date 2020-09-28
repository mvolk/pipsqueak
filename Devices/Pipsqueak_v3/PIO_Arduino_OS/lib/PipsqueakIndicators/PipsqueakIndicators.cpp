#include "PipsqueakIndicators.h"
#include <ESP8266WiFi.h>

PipsqueakIndicators::PipsqueakIndicators(PipsqueakState * state) {
  _state = state;
  _config = state->getConfig();
}

void PipsqueakIndicators::setup() {
  pinMode(_config->getRedIndicatorPin(), OUTPUT);
  digitalWrite(_config->getRedIndicatorPin(), LOW);
  pinMode(_config->getGreenIndicatorPin(), OUTPUT);
  digitalWrite(_config->getGreenIndicatorPin(), LOW);
}

void PipsqueakIndicators::loop() {
  uint8_t greenState = HIGH;
  uint8_t redState = LOW;

  if (!_state->isInitialized()) {
    greenState = blink(BLINK_PATTERN_DOT_DOT);
  } else {
    if (!WiFi.isConnected()) {
      redState = blink(BLINK_PATTERN_DOT_DASH);
    } else if (!_state->isClockSynchronized()) {
      redState = blink(BLINK_PATTERN_DASH_DOT_DOT_DOT);
    } else if (!_state->isServerConnectionHealthy()) {
      redState = blink(BLINK_PATTERN_DASH_DOT);
    }

    if (!_state->isBoardSensorDetected() || isnan(_state->getBoardTemperature())) {
      greenState = LOW;
      redState = blink(BLINK_PATTERN_DOT_DASH);
    } else if (!_state->isRemoteSensorDetected() || isnan(_state->getRemoteTemperature())) {
      greenState = LOW;
      greenState = blink(BLINK_PATTERN_DASH_DOT_DOT_DOT);
    }

    if (_state->getBoardTemperature() > _config->getBoardTemperatureLimit()) {
      greenState = LOW;
      redState = HIGH;
    }
  }

  digitalWrite(_config->getGreenIndicatorPin(), greenState);
  digitalWrite(_config->getRedIndicatorPin(), redState);
}

uint16_t PipsqueakIndicators::getPhase(uint16_t duration, uint16_t count) {
  return (millis() / duration) % count;
}

uint8_t PipsqueakIndicators::blink(uint8_t pattern) {
  switch (pattern) {
    case BLINK_PATTERN_DASH_DOT:
      // 0.75s: [ ON, ON, OFF ]: 500ms on / 250ms off
      return getPhase(250, 3) < 2 ? HIGH : LOW;
    case BLINK_PATTERN_DASH_DASH:
      // 1s: [ ON, OFF ]: 500ms on / 500ms off
      return getPhase(500, 2) < 1 ? HIGH : LOW;
    case BLINK_PATTERN_DOT_DOT:
      // 0.5s: [ ON, OFF ]: 250ms on / 250ms off
      return getPhase(250, 2) < 1 ? HIGH : LOW;
    case BLINK_PATTERN_DOT_DASH:
      // 0.75s: [ ON, OFF, OFF ]: 500ms on / 250ms off
      return getPhase(250, 3) < 1 ? HIGH : LOW;
    case BLINK_PATTERN_DASH_DOT_DOT_DOT:
      // 0.75s: [ ON, ON, OFF, ON, OFF ]: 500ms on / 250ms off / 250ms on / 250ms off
      return getPhase(250, 5) < 2 || getPhase(250, 5) == 3 ? HIGH : LOW;
    default:
      return LOW;
  }
}
