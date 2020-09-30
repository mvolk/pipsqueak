#include "PipsqueakSensors.h"

// amount of time that a temperature reading is considered "current"
#define BOARD_READING_TTL           1000 // ms
#define REMOTE_READING_TTL          5000 // ms

#define BOARD_SENSOR_RESOLUTION     9  // bits
#define REMOTE_SENSOR_RESOLUTION    12 // bits

PipsqueakSensors::PipsqueakSensors(PipsqueakState * state)
:
  _boardSensor { NULL },
  _remoteSensor { NULL },
  _sensorToggle { false }
{
  _state = state;
  _config = state->getConfig();
}

void PipsqueakSensors::setup() {
  _oneWire = new OneWire(_config->getOneWirePin());
}

void PipsqueakSensors::loop() {
  if (!(_boardSensor || _remoteSensor)) {
    detectSensors();
    return;
  }
  if (_boardSensor->isReadyToRead() && _boardSensor->read()) {
    _state->setBoardTemperature(_boardSensor->getTemperature());
  }
  if (_remoteSensor->isReadyToRead() && _remoteSensor->read()) {
    _state->setBoardTemperature(_remoteSensor->getTemperature());
  }
  if (!(_boardSensor->isSensing() || _remoteSensor->isSensing())) {
    if (_sensorToggle) {
      _remoteSensor->startSensing();
    } else {
      _boardSensor->startSensing();
    }
    _sensorToggle = !_sensorToggle;
  }
}

void PipsqueakSensors::detectSensors() {
  byte addressBuffer[2 * DS18B20_ADDRESS_SIZE];
  size_t countOfSensorsDetected = DS18B20::detect(_oneWire, addressBuffer, 2);
  if (countOfSensorsDetected == 0) {
    #ifdef DEBUG_PIPSQUEAK_SENSORS
    Serial.println("PipsqueakSensors.detectSensors(): no devices detected");
    #endif
    return;
  }
  if (countOfSensorsDetected > 2) {
    #ifdef DEBUG_PIPSQUEAK_SENSORS
    Serial.printf("PipsqueakSensors.detectSensors(): %u devices detected\n", countOfSensorsDetected);
    #endif
    return;
  }

  // First find the board sensor
  if (!_boardSensor) {
    for (size_t i = 0; i < 2; i++) {
      if (_config->isBoardSensorAddress(&addressBuffer[i * DS18B20_ADDRESS_SIZE])) {
        #ifdef DEBUG_PIPSQUEAK_SENSORS
        Serial.println("PipsqueakSensors.detectSensors(): board sensor detected");
        #endif
        _boardSensor = new DS18B20(_oneWire, &addressBuffer[i * DS18B20_ADDRESS_SIZE], BOARD_SENSOR_RESOLUTION, BOARD_READING_TTL);
        _state->setBoardSensorDetected(true);
      }
    }
  }

  // Then find the "other" sensor - which is the remote sensor
  if (_boardSensor) {
    for (size_t i = 0; i < 2; i++) {
      if (!_config->isBoardSensorAddress(&addressBuffer[i * DS18B20_ADDRESS_SIZE])) {
        #ifdef DEBUG_PIPSQUEAK_SENSORS
        Serial.println("PipsqueakSensors.detectSensors(): remote sensor detected");
        #endif
        _remoteSensor = new DS18B20(_oneWire, &addressBuffer[i * DS18B20_ADDRESS_SIZE], REMOTE_SENSOR_RESOLUTION, REMOTE_READING_TTL);
        _state->setRemoteSensorDetected(true);
      }
    }
  }

  #ifdef DEBUG_PIPSQUEAK_SENSORS
  if (!_boardSensor) {
    Serial.println("PipsqueakSensors.detectSensors(): onboard DS18B20 not found");
  }
  if (!_remoteSensor) {
    Serial.println("PipsqueakSensors.detectSensors(): remote DS18B20 not found");
  }
  #endif
}
