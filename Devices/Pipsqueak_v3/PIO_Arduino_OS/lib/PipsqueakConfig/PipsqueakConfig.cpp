#include "PipsqueakConfig.h"
#include <Arduino.h>
#include <EEPROM.h>

#define SCHEMA_VERSION 3
#define DEFAULT_PORT 9001
#define DEFAULT_SETPOINT 15

#define BOARD_TEMPERATURE_LIMIT 40

#define INITIALIZED_FLAG 0x0F

PipsqueakConfig::PipsqueakConfig()
  :
  _hostIP { NULL },
  _hostPort { DEFAULT_PORT },
  _deviceID { 0 },
  _configurationFlags { 0 },
  _enablePin { 0 },
  _oneWirePin { 0 },
  _redIndicatorPin { 0 },
  _greenIndicatorPin { 0 },
  _heaterPin { 0 },
  _chillerPin { 0 },
  _setpoint { DEFAULT_SETPOINT }
{
  memset(_wifiSSID, 0, WIFI_SSID_BUFFER_SIZE);
  memset(_wifiPassword, 0, WIFI_PASSWORD_BUFFER_SIZE);
  memset(_secretKey, 0, SECRET_KEY_BUFFER_SIZE);
  memset(_boardSensorAddress, 0, BOARD_SENSOR_ADDRESS_SIZE);
}

void PipsqueakConfig::setup() {
  size_t i;
  size_t cursor = 0;
  uint8_t initializedFlag;
  uint8_t schemaVersion;

  EEPROM.begin(256);
  EEPROM.get(cursor, initializedFlag);
  cursor += 1;
  if (initializedFlag != INITIALIZED_FLAG) {
    #ifdef DEBUG_PIPSQUEAK_CONFIG
    Serial.println("Fatal Error: EEPROM has not been initialized");
    #endif
    delay(5000);
    ESP.restart();
  }
  EEPROM.get(cursor, schemaVersion);
  cursor += 1;
  if (schemaVersion != SCHEMA_VERSION) {
    #ifdef DEBUG_PIPSQUEAK_CONFIG
    Serial.printf("Fatal Error: unsupported EEPROM schema version: %u\n", schemaVersion);
    #endif
    delay(5000);
    ESP.restart();
  }
  EEPROM.get(cursor, _configurationFlags);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): configurationFlags = %u\n", _configurationFlags);
  #endif
  // skip reserved byte
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  uint32_t writeCount;
  EEPROM.get(cursor, writeCount);
  Serial.printf("PipsqueakConfig.setup(): writeCount = %u\n", writeCount);
  #endif
  cursor += 4;
  EEPROM.get(cursor, _deviceID);
  cursor += 4;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): deviceID = %u\n", _deviceID);
  #endif
  EEPROM.get(cursor, _setpoint);
  cursor += 4;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): setpoint = %f degrees C\n", _setpoint);
  #endif
  uint8_t hostIP[4];
  EEPROM.get(cursor, hostIP);
  _hostIP = new IPAddress(hostIP);
  cursor += 4;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): hostIP = %u.%u.%u.%u\n", hostIP[0], hostIP[1], hostIP[2], hostIP[3]);
  #endif
  EEPROM.get(cursor, _hostPort);
  cursor += 2;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): hostPort = %u\n", _hostPort);
  #endif
  EEPROM.get(cursor, _wifiSSID);
  cursor += WIFI_SSID_BUFFER_SIZE;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): wifiSSID = %s\n", _wifiSSID);
  #endif
  EEPROM.get(cursor, _wifiPassword);
  cursor += WIFI_PASSWORD_BUFFER_SIZE;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): wifiPassword = %s\n", _wifiPassword);
  #endif
  for (i = 0; i < SECRET_KEY_BUFFER_SIZE; i++) {
    EEPROM.get(cursor + i, _secretKey[i]);
  }
  cursor += SECRET_KEY_BUFFER_SIZE;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.print("PipsqueakConfig.setup(): secretkey = ");
  for (i = 0; i < 32; i++) {
    Serial.printf("0x%02X", _secretKey[i]);
    if (i < 31) Serial.print(" ");
  }
  Serial.print("\n");
  #endif
  EEPROM.get(cursor, _enablePin);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): enablePin = GPIO %02u\n", _enablePin);
  #endif
  // skip 1wire pin for board sensor - same as for remote sensor
  cursor += 1;
  for (i = 0; i < BOARD_SENSOR_ADDRESS_SIZE; i++) {
    EEPROM.get(cursor + i, _boardSensorAddress[i]);
  }
  cursor += BOARD_SENSOR_ADDRESS_SIZE;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.print("PipsqueakConfig.setup(): boardSensorAddress = ");
  for (i = 0; i < 8; i++) {
    Serial.printf("0x%02X", _boardSensorAddress[i]);
    if (i < 7) Serial.print(" ");
  }
  Serial.print("\n");
  #endif
  EEPROM.get(cursor, _oneWirePin);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): oneWirePin = GPIO %02u\n", _oneWirePin);
  #endif
  // skip remote sensor address - autodetected
  cursor += 8;
  EEPROM.get(cursor, _redIndicatorPin);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): redIndicatorPin = GPIO %02u\n", _redIndicatorPin);
  #endif
  EEPROM.get(cursor, _greenIndicatorPin);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): greenIndicatorPin = GPIO %02u\n", _greenIndicatorPin);
  #endif
  uint8_t heaterPinCount;
  EEPROM.get(cursor, heaterPinCount);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): heaterPinCount = %u\n", heaterPinCount);
  #endif
  if (heaterPinCount != 1) {
    #ifdef DEBUG_PIPSQUEAK_CONFIG
    Serial.printf("Fatal Error: exactly one configured heater pin required, %u found\n", heaterPinCount);
    #endif
    delay(5000);
    ESP.restart();
  }
  EEPROM.get(cursor, _heaterPin);
  cursor += heaterPinCount;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): heaterPin = GPIO %02u\n", _heaterPin);
  #endif
  uint8_t chillerPinCount;
  EEPROM.get(cursor, chillerPinCount);
  cursor += 1;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): chillerPinCount = %u\n", chillerPinCount);
  #endif
  if (chillerPinCount != 1) {
    #ifdef DEBUG_PIPSQUEAK_CONFIG
    Serial.printf("Fatal Error: exactly one configured chiller pin required, %u found\n", chillerPinCount);
    #endif
    delay(5);
    ESP.restart();
  }
  EEPROM.get(cursor, _chillerPin);
  cursor += chillerPinCount;
  #ifdef DEBUG_PIPSQUEAK_CONFIG
  Serial.printf("PipsqueakConfig.setup(): chillerPin = GPIO %02u\n", _chillerPin);
  #endif
  EEPROM.end();
}

const char * PipsqueakConfig::getWifiSSID() {
  return _wifiSSID;
}

const char * PipsqueakConfig::getWifiPassword() {
  return _wifiPassword;
}

IPAddress * PipsqueakConfig::getHostIP() {
  return _hostIP;
}

uint16_t PipsqueakConfig::getHostPort() {
  return _hostPort;
}

uint32_t PipsqueakConfig::getDeviceID() {
  return _deviceID;
}

const byte * PipsqueakConfig::getSecretKey() {
  return _secretKey;
}

uint8_t PipsqueakConfig::getSignalEnablePin() {
  return _enablePin;
}

uint8_t PipsqueakConfig::getOneWirePin() {
  return _oneWirePin;
}

uint8_t PipsqueakConfig::getRedIndicatorPin() {
  return _redIndicatorPin;
}

uint8_t PipsqueakConfig::getGreenIndicatorPin() {
  return _greenIndicatorPin;
}

uint8_t PipsqueakConfig::getHeaterPin() {
  return _heaterPin;
}

uint8_t PipsqueakConfig::getChillerPin() {
  return _chillerPin;
}

void PipsqueakConfig::setTemperatureSetpoint(float setpoint) {
  if (setpoint != _setpoint) {
    _setpoint = setpoint;
    persist();
  }
}

float PipsqueakConfig::getTemperatureSetpoint() {
  return _setpoint;
}

bool PipsqueakConfig::isBoardSensorAddress(uint8_t * address) {
  return memcmp(address, _boardSensorAddress, BOARD_SENSOR_ADDRESS_SIZE) == 0;
}

uint8_t * PipsqueakConfig::getBoardSensorAddress() {
  return _boardSensorAddress;
}

float PipsqueakConfig::getBoardTemperatureLimit() {
  return BOARD_TEMPERATURE_LIMIT;
}

void PipsqueakConfig::persist() {
  EEPROM.begin(256);
  uint32_t writeCount;
  EEPROM.get(4, writeCount);
  EEPROM.put(4, writeCount + 1);
  EEPROM.put(12, _setpoint);
  EEPROM.end();
}
