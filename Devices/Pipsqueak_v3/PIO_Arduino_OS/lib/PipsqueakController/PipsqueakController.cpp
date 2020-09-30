#include "PipsqueakController.h"

#define HEATER_PULSE_DURATION 10000
#define HEATER_PULSE_POWER 100
#define HEATER_RECOVERY_DURATION 120000

#define CHILLER_PULSE_DURATION 1000
#define CHILLER_RECOVERY_DURATION 60000

#define INITIAL_QUIET_PERIOD 30000

// 12-bit DS18B20 resolution leads to 0.0625 degree C steps
#define REMOTE_SENSOR_RESOLUTION 0.0625

#define TEMPERATURE_TOLERANCE REMOTE_SENSOR_RESOLUTION

PipsqueakController::PipsqueakController(PipsqueakState * state)
:
  _heating { false },
  _chilling { false },
  _pulseDuration { 0 },
  _recoveryDuration { INITIAL_QUIET_PERIOD },
  _lastToggled { 0 }
{
  _state = state;
  _config = state->getConfig();
}

void PipsqueakController::setup() {
  pinMode(_config->getHeaterPin(), OUTPUT);
  digitalWrite(_config->getHeaterPin(), LOW);
  pinMode(_config->getChillerPin(), OUTPUT);
  digitalWrite(_config->getChillerPin(), LOW);
}

void PipsqueakController::loop() {
  if (shouldHeat()) heaterPulse();
  if (shouldChill()) chillerPulse();
  if (shouldStopRunning()) stopRunning();
  if (_heating) modulateHeater();
}

bool PipsqueakController::isRunning() {
  return _heating || _chilling;
}

bool PipsqueakController::shouldHeat() {
  if (isRunning() || isRecovering()) return false;
  if (!_state->isSafeToOperate()) return false;
  float temperature = _state->getRemoteTemperature();
  if (isnan(temperature)) return false;
  return temperature < lowerLimit();
}

void PipsqueakController::heaterPulse() {
  #ifdef DEBUG_PIPSQUEAK_CONTROLLER
  Serial.println("PipsqueakController.heaterPulse()");
  #endif
  _heating = true;
  _pulseDuration = HEATER_PULSE_DURATION;
  _recoveryDuration = HEATER_RECOVERY_DURATION;
  _lastToggled = millis();
  _state->recordHeaterPulse(HEATER_PULSE_DURATION, HEATER_PULSE_POWER, HEATER_RECOVERY_DURATION);
}

void PipsqueakController::modulateHeater() {
  if (!_heating) return;
  // Use poor man's pulse-width modulation to reduce power delivery
  // 100ms period, deliver power for the first _percentPower ms of each period
  // Note that this neatly takes care of % power values > 100
  digitalWrite(_config->getHeaterPin(), (millis() % 100) < HEATER_PULSE_POWER ? HIGH : LOW);
  #ifdef DEBUG_PIPSQUEAK_CONTROLLER
  Serial.printf("PipsqueakController.modulateHeater(): %s\n", (millis() % 100) < HEATER_PULSE_POWER ? "HIGH" : "LOW");
  #endif
}

bool PipsqueakController::shouldChill() {
  if (isRunning() || isRecovering()) return false;
  if (!_state->isSafeToOperate()) return false;
  float temperature = _state->getRemoteTemperature();
  if (isnan(temperature)) return false;
  return temperature > upperLimit();
}

void PipsqueakController::chillerPulse() {
  #ifdef DEBUG_PIPSQUEAK_CONTROLLER
  Serial.println("PipsqueakController.chillerPulse()");
  #endif
  _chilling = true;
  _pulseDuration = CHILLER_PULSE_DURATION;
  _recoveryDuration = CHILLER_RECOVERY_DURATION;
  _lastToggled = millis();
  _state->recordChillerPulse(CHILLER_PULSE_DURATION, CHILLER_RECOVERY_DURATION);
  digitalWrite(_config->getChillerPin(), HIGH);
}

bool PipsqueakController::isRecovering() {
  return !isRunning() && millis() - _lastToggled < _recoveryDuration;
}

bool PipsqueakController::shouldStopRunning() {
  if (!isRunning()) return false;
  if (!_state->isSafeToOperate()) return true;
  return millis() - _lastToggled >= _pulseDuration;
}

void PipsqueakController::stopRunning() {
  #ifdef DEBUG_PIPSQUEAK_CONTROLLER
  Serial.println("PipsqueakController.stopRunning()");
  #endif
  digitalWrite(_config->getHeaterPin(), LOW);
  _heating = false;
  digitalWrite(_config->getChillerPin(), LOW);
  _chilling = false;
  _lastToggled = millis();
}

float PipsqueakController::upperLimit() {
  return _config->getTemperatureSetpoint() + TEMPERATURE_TOLERANCE;
}

float PipsqueakController::lowerLimit() {
  return _config->getTemperatureSetpoint() - TEMPERATURE_TOLERANCE;
}
