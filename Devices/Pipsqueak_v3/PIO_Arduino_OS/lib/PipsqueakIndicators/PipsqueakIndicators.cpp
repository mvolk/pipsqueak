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

  #ifdef DEBUG_PIPSQUEAK_INDICATORS
  Serial.println("PipsqueakIndicators.loop(): begin loop -----------------------");
  #endif

  if (!_state->isInitialized()) {
    #ifdef DEBUG_PIPSQUEAK_INDICATORS
    Serial.println("PipsqueakIndicators.loop(): initializing");
    #endif
    greenState = blink(BLINK_PATTERN_DOT_DOT);
  } else {
    if (!WiFi.isConnected()) {
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): no wifi connection");
      #endif
      redState = blink(BLINK_PATTERN_DOT_DASH);
    } else if (!_state->isClockSynchronized()) {
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): clock is not synchronized");
      #endif
      redState = blink(BLINK_PATTERN_DASH_DOT_DOT_DOT);
    } else if (!_state->isServerConnectionHealthy()) {
      redState = blink(BLINK_PATTERN_DASH_DOT);
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): server connection is not healthy");
      #endif
    }

    if (!_state->isBoardSensorDetected() || isnan(_state->getBoardTemperature())) {
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): board sensor problem");
      #endif
      greenState = LOW;
      redState = blink(BLINK_PATTERN_DOT_DASH);
    } else if (!_state->isRemoteSensorDetected() || isnan(_state->getRemoteTemperature())) {
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): remote sensor problem");
      #endif
      greenState = LOW;
      redState = blink(BLINK_PATTERN_DASH_DOT_DOT_DOT);
    }

    if (_state->getBoardTemperature() > _config->getBoardTemperatureLimit()) {
      #ifdef DEBUG_PIPSQUEAK_INDICATORS
      Serial.println("PipsqueakIndicators.loop(): overheated");
      #endif
      greenState = LOW;
      redState = HIGH;
    }
  }

  digitalWrite(_config->getGreenIndicatorPin(), greenState);
  digitalWrite(_config->getRedIndicatorPin(), redState);

  #ifdef DEBUG_PIPSQUEAK_INDICATORS
  Serial.println("PipsqueakIndicators.loop(): end loop -------------------------");
  #endif
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
