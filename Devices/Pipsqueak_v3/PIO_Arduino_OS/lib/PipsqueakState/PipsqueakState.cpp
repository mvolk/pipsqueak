#include "PipsqueakState.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Errors.h>
#include <TimeLib.h>

PipsqueakState::PipsqueakState()
:
  _config(),
  _statusEvent(),
  _systemInitialized { false },
  _wifiInitialized { false },
  _clockInitialized { false },
  _clockSynchronized { false },
  _boardSensorInitialized { false },
  _boardSensorDetected { false },
  _boardTemperatureInitialized { false },
  _boardTemperature { NAN },
  _overheated { false },
  _remoteSensorInitialized { false },
  _remoteSensorDetected { false },
  _remoteTemperatureInitialized { false },
  _remoteTemperature { NAN },
  _statusEventQueueCursor { 0 },
  _statusEventQueueDepth { 0 }
{
  memset(_statusEventQueue, 0, STATUS_EVENT_QUEUE_SIZE);
}

void PipsqueakState::setup() {
  _config.setup();
}

void PipsqueakState::loop() {
  bool overheated = !isnan(_boardTemperature) && _boardTemperature > _config.getBoardTemperatureLimit();
  if (!_overheated && overheated) {
    _overheated = true;
    recordError(ErrorType::Pipsqueak, DEVICE_OVERHEATED_ERROR);
  } else if (_overheated && !overheated) {
    _overheated = false;
  }

  if (!_systemInitialized) {
    if (
      _wifiInitialized &&
      _clockInitialized &&
      _boardSensorInitialized &&
      _boardTemperatureInitialized &&
      _remoteSensorInitialized &&
      _remoteTemperatureInitialized
    ) {
      _systemInitialized = true;
    }
  }

  if (!_systemInitialized && millis() > INITIALIZATION_WINDOW_MILLIS) {
    if (!_wifiInitialized) {
      _wifiInitialized = true;
      if (!WiFi.isConnected()) {
        recordError(ErrorType::Pipsqueak, WIFI_CONNECTION_ERROR);
      }
    }
    if (!_clockInitialized) {
      _clockInitialized = true;
      recordError(ErrorType::Pipsqueak, CLOCK_SYNC_ERROR);
    }
    if (!_boardSensorInitialized) {
      _boardSensorInitialized = true;
      recordError(ErrorType::Pipsqueak, BOARD_SENSOR_DETECTION_ERROR);
    }
    if (!_boardTemperatureInitialized) {
      _boardTemperatureInitialized = true;
      recordError(ErrorType::Pipsqueak, BOARD_TEMPERATURE_NAN_ERROR);
    }
    if (!_remoteSensorInitialized) {
      _remoteSensorInitialized = true;
      recordError(ErrorType::Pipsqueak, REMOTE_SENSOR_DETECTION_ERROR);
    }
    if (!_remoteTemperatureInitialized) {
      _remoteTemperatureInitialized = true;
      recordError(ErrorType::Pipsqueak, REMOTE_TEMPERATURE_NAN_ERROR);
    }
    _systemInitialized = true;
  }
}

PipsqueakConfig * PipsqueakState::getConfig() {
  return &_config;
}

bool PipsqueakState::isInitialized() {
  return _systemInitialized;
}

bool PipsqueakState::isSafeToOperate() {
  if (!_boardSensorDetected) return false;
  if (isnan(_boardTemperature)) return false;
  if (_boardTemperature > _config.getBoardTemperatureLimit()) return false;
  if (!_remoteSensorDetected) return false;
  if (isnan(_remoteTemperature)) return false;
  return true;
}

bool PipsqueakState::isClockSynchronized() {
  return _clockSynchronized;
}

void PipsqueakState::setClockSynchronized(bool synchronized) {
  if (!synchronized && (_clockSynchronized || !_clockInitialized)) {
    recordError(ErrorType::Pipsqueak, CLOCK_SYNC_ERROR);
  }
  if (!_clockInitialized) {
    _clockInitialized = true;
  }
  if (synchronized && !_clockSynchronized) {
    if (_boardTemperatureInitialized) {
      // TODO: board temperature status event
    }
    if (_remoteTemperatureInitialized) {
      _statusEvent.temperatureObservation(now(), _remoteTemperature);
      enqueueStatusEvent();
    }
    _statusEvent.temperatureSetpoint(now(), _config.getTemperatureSetpoint());
    enqueueStatusEvent();
  }
  _clockSynchronized = synchronized;
}

void PipsqueakState::setBoardSensorDetected(bool sensorDetected) {
  if (!sensorDetected && (_boardSensorDetected || !_boardSensorInitialized)) {
    recordError(ErrorType::Pipsqueak, BOARD_SENSOR_DETECTION_ERROR);
  }
  if (!_boardSensorInitialized) _boardSensorInitialized = true;
  _boardSensorDetected = sensorDetected;
}

bool PipsqueakState::isBoardSensorDetected() {
  return _boardSensorDetected;
}

float PipsqueakState::getBoardTemperature() {
  return _boardTemperature;
}

void PipsqueakState::setBoardTemperature(float temperature) {
  if (!_boardTemperatureInitialized) {
    _boardTemperatureInitialized = true;
  } else if (isnan(_boardTemperature) && isnan(temperature)) {
    return;
  } else if (_boardTemperature == temperature) {
    return;
  }

  if (isnan(temperature)) {
    recordError(ErrorType::Pipsqueak, BOARD_TEMPERATURE_NAN_ERROR);
  }

  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakStatesetBoardTemperature(): temperature from %f to %f\n", _boardTemperature, temperature);
  #endif
  _boardTemperature = temperature;

  if (_clockSynchronized) {
    // TODO: board temperature status event
  }
}

void PipsqueakState::setRemoteSensorDetected(bool sensorDetected) {
  if (!sensorDetected && (_remoteSensorDetected || !_remoteSensorInitialized)) {
    recordError(ErrorType::Pipsqueak, REMOTE_SENSOR_DETECTION_ERROR);
  }
  if (!_remoteSensorInitialized) _remoteSensorInitialized = true;
  _remoteSensorDetected = sensorDetected;
}

bool PipsqueakState::isRemoteSensorDetected() {
  return _remoteSensorDetected;
}

float PipsqueakState::getRemoteTemperature() {
  return _remoteTemperature;
}

void PipsqueakState::setRemoteTemperature(float temperature) {
  if (!_remoteTemperatureInitialized) {
    _remoteTemperatureInitialized = true;
  } else if (isnan(_remoteTemperature) && isnan(temperature)) {
    return;
  } else if (_remoteTemperature == temperature) {
    return;
  }

  if (isnan(temperature)) {
    recordError(ErrorType::Pipsqueak, REMOTE_TEMPERATURE_NAN_ERROR);
  }

  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakStatesetBoardTemperature(): temperature from %f to %f\n", _boardTemperature, temperature);
  #endif
  _remoteTemperature = temperature;

  if (_clockSynchronized) {
    #ifdef DEBUG_PIPSQUEAK_STATE
    Serial.printf("PipsqueakState.setRemoteTemperature(): temperature observation event @ %fC\n", temperature);
    #endif
    _statusEvent.temperatureObservation(now(), temperature);
    enqueueStatusEvent();
  }
}

void PipsqueakState::setRemoteTemperatureSetpoint(float setpoint) {
  if (_config.getTemperatureSetpoint() != setpoint) {
    #ifdef DEBUG_PIPSQUEAK_STATE
    float previousSetpoint = _config.getTemperatureSetpoint();
    #endif
    _config.setTemperatureSetpoint(setpoint);
    if (_clockSynchronized) {
      #ifdef DEBUG_PIPSQUEAK_STATE
      Serial.printf("PipsqueakState.setRemoteTemperatureSetpoint(): setpoint update event @ %fC\n", setpoint);
      #endif
      _statusEvent.temperatureSetpoint(now(), setpoint);
      enqueueStatusEvent();
    }
    #ifdef DEBUG_PIPSQUEAK_STATE
    Serial.printf("Model.setRemoteSetpoint(): Setpoint updated from %f to %f\n", previousSetpoint, setpoint);
    #endif
  }
}

void PipsqueakState::recordError(ErrorType errorType, int8_t errorCode) {
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("Model.recordError(%d, %d)\n", errorType, errorCode);
  #endif
  time_t timestamp = _clockSynchronized ? now() : 0;
  _statusEvent.error(timestamp, errorType, errorCode);
  enqueueStatusEvent();
}

void PipsqueakState::recordErrors(Response * response) {
  for (size_t i = 0; i < response->errorCount(); i++) {
    recordError(response->getErrorType(i), response->getErrorCode(i));
  }
}

void PipsqueakState::recordChillerPulse(uint32_t pulseDuration, uint32_t recoveryDuration) {
  if (_clockSynchronized) {
    _statusEvent.chillerPulse(now(), pulseDuration, recoveryDuration);
    enqueueStatusEvent();
  }
}

void PipsqueakState::recordHeaterPulse(
  uint32_t pulseDuration,
  uint8_t percentPower,
  uint32_t recoveryDuration
) {
  if (_clockSynchronized) {
    _statusEvent.heaterPulse(now(), pulseDuration, percentPower, recoveryDuration);
    enqueueStatusEvent();
  }
}

bool PipsqueakState::hasStatusEvents() {
  return _statusEventQueueDepth > 0;
}

StatusEvent * PipsqueakState::dequeueStatusEvent() {
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.println("PipsqueakState.dequeueStatusEvent()");
  #endif
  if (!hasStatusEvents()) return NULL;
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakState.dequeueStatusEvent() from offset %u\n", _statusEventQueueCursor);
  #endif
  _statusEvent.read(&_statusEventQueue[_statusEventQueueCursor]);
  advanceStatusEventQueueCursor();
  _statusEventQueueDepth -= 1;
  return &_statusEvent;
}

void PipsqueakState::enqueueStatusEvent() {
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakState.enqueueStatusEvent() cursor=%u depth=%u eventSize=%u queueSize=%u\n", _statusEventQueueCursor, _statusEventQueueDepth, STATUS_EVENT_SIZE, STATUS_EVENT_QUEUE_SIZE);
  #endif
  size_t head = (_statusEventQueueCursor + (_statusEventQueueDepth * STATUS_EVENT_SIZE)) % (STATUS_EVENT_QUEUE_SIZE);
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakState.enqueueStatusEvent() at offset %u\n", head);
  #endif
  _statusEvent.write(&_statusEventQueue[head]);
  if (_statusEventQueueDepth == STATUS_EVENT_QUEUE_DEPTH_LIMIT) {
    // cursor (tail) needs to advance - we just overwrote the oldest event
    advanceStatusEventQueueCursor();
  } else {
    _statusEventQueueDepth += 1;
  }
}

void PipsqueakState::advanceStatusEventQueueCursor() {
  #ifdef DEBUG_PIPSQUEAK_STATE
  size_t was = _statusEventQueueCursor;
  #endif
  _statusEventQueueCursor = (_statusEventQueueCursor + STATUS_EVENT_SIZE) % (STATUS_EVENT_QUEUE_SIZE);
  #ifdef DEBUG_PIPSQUEAK_STATE
  Serial.printf("PipsqueakState.advanceStatusEventQueueCursor() %u to %u\n", was, _statusEventQueueCursor);
  #endif
}
