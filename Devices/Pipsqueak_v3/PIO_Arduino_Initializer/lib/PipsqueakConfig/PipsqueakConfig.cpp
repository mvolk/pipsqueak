#include "PipsqueakConfig.h"
#include <EEPROM.h>
#include <InitializationParameters.h>

// Value in the first byte of the buffer if previously initialized
#define INITIALIZED_MARKER        0x0F

// Schema version value
#define SCHEMA_VERSION            3

PipsqueakConfig::PipsqueakConfig()
:
  _previouslyWrittenFlag { 0 },
  _writeCount { 0 },
  _schemaVersion { 0 },
  _configurationFlags { 0 },
  _deviceID { 0 },
  _setpoint { 0 },
  _hostPort { 0 },
  _enablePin { 0 },
  _oneWirePin { 0 },
  _redIndicatorPin { 0 },
  _greenIndicatorPin { 0 },
  _heaterPinCount { 0 },
  _heaterPins { new uint8_t[0] },
  _chillerPinCount { 0 },
  _chillerPins { new uint8_t[0] }
{
  memset(_hostIP, 0, 4);
  memset(_wifiSSID, 0, 32);
  memset(_wifiPassword, 0, 64);
  memset(_secretKey, 0, 32);
  memset(_remoteSensorAddress, 0, DS18B20_ADDRESS_SIZE);
  memset(_boardSensorAddress, 0, DS18B20_ADDRESS_SIZE);
}

/*
| Start     | Size   | Type      | Description
| --------- | ------ | --------- | ----------------------------------------------------
| 0         | 1      | uint8     | EEPROM previously written flag (0x0F if previously written, random value unknown if never written)
| 1         | 1      | uint8     | EEPROM schema version
| 2         | 1      | uint8     | Config flags - not relevant for Pipsqeakv3
| 3         | 1      | ---       | reserved
| 4         | 4      | uint32    | EEPROM write count - number of times that the EEPROM has been updated.
| 8         | 4      | uint32    | deviceID
| 12        | 4      | float     | setpoint in celsius
| 16        | 4      | uint8[4]  | host server's ipv4 address, expressed in octets
| 20        | 2      | uint16    | host server's port
| 22        | 32     | char[32]  | wifi ssid, up to 32 characters including null termination
| 54        | 64     | char[64]  | wifi password, at most 64 characters including null termination
| 118       | 32     | uint8[32] | secret encryption key specific to this device
| 150       | 1      | uint8     | signal enable GPIO pin number
| 151       | 1      | uint8     | 1Wire GPIO pin number for board sensor (not presently used)
| 152       | 8      | uint8[8]  | 1Wire address of onboard DS18B20 temperature sensor
| 160       | 1      | uint8     | 1Wire GPIO pin number for remote sensor
| 161       | 8      | uint8[8]  | 1Wire address of onboard DS18B20 temperature sensor (not presently used)
| 169       | 1      | uint8     | Red LED indicator signal GPIO pin number
| 170       | 1      | uint8     | Green LED indicator signal GPIO pin number
| 171       | 1      | uint8     | Heater pin count (n) - should be exactly 1 for Pipsqueak v3
| 172       | n      | uint8     | Heater signal GPIO pin numbers
| 172+n     | 1      | uint8     | Chiller pin count (m) - should be exactly 1 for Pipsqueak v3
| 172+n+1   | m      | uint8     | Chiller signal GPIO pin numbers
*/
void PipsqueakConfig::readContents() {
  if (!IGNORE_PREVIOUS_CONFIG) {
    uint8_t i;
    size_t cursor = 0;
    EEPROM.begin(256);
    EEPROM.get(cursor, _previouslyWrittenFlag);
    if (previouslyInitialized()) {
      cursor += 1;
      EEPROM.get(cursor, _schemaVersion);
      cursor += 1;
      EEPROM.get(cursor, _configurationFlags);
      cursor += 1;
      // reserved
      cursor += 1;
      EEPROM.get(cursor, _writeCount);
      cursor += 4;
      EEPROM.get(cursor, _deviceID);
      cursor += 4;
      EEPROM.get(cursor, _setpoint);
      cursor += 4;
      EEPROM.get(cursor, _hostIP);
      cursor += 4;
      EEPROM.get(cursor, _hostPort);
      cursor += 2;
      EEPROM.get(cursor, _wifiSSID);
      cursor += 32;
      EEPROM.get(cursor, _wifiPassword);
      cursor += 64;
      for (i = 0; i < 32; i++) EEPROM.get(cursor + i, _secretKey[i]);
      cursor += 32;
      EEPROM.get(cursor, _enablePin);
      cursor += 1;
      EEPROM.get(cursor, _oneWirePin); // onboard sensor
      cursor += 1;
      for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) EEPROM.get(cursor + i, _boardSensorAddress[i]);
      cursor += DS18B20_ADDRESS_SIZE;
      EEPROM.get(cursor, _oneWirePin); // remote sensor
      cursor += 1;
      for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) EEPROM.get(cursor + i, _remoteSensorAddress[i]);
      cursor += DS18B20_ADDRESS_SIZE;
      EEPROM.get(cursor, _redIndicatorPin);
      cursor += 1;
      EEPROM.get(cursor, _greenIndicatorPin);
      cursor += 1;
      EEPROM.get(cursor, _heaterPinCount);
      cursor += 1;
      _heaterPins = new uint8_t[_heaterPinCount];
      for (i = 0; i < _heaterPinCount; i++) EEPROM.get(cursor + i, _heaterPins[i]);
      cursor += _heaterPinCount;
      EEPROM.get(cursor, _chillerPinCount);
      cursor += 1;
      _chillerPins = new uint8_t[_chillerPinCount];
      for (i = 0; i < _chillerPinCount; i++) EEPROM.get(cursor + i, _chillerPins[i]);
      cursor += _chillerPinCount;
      EEPROM.end();
    }
  }

  if (IGNORE_PREVIOUS_CONFIG) {
    Serial.println("PipsqueakConfig.readContents(): aborted due to IGNORE_PREVIOUS_CONFIG option");
  } else if (previouslyInitialized()) {
    Serial.println("PipsqueakConfig.readContents(): Previous configuration detected & loaded");
  } else {
    Serial.println("PipsqueakConfig.readContents(): Fresh EEPROM state - no previous configuration detected");
  }
}

bool PipsqueakConfig::previouslyInitialized() {
  return _previouslyWrittenFlag == INITIALIZED_MARKER;
}

void PipsqueakConfig::applyUpdates() {
  _schemaVersion = SCHEMA_VERSION;
  _deviceID = CONFIG_DEVICE_ID;
  memcpy(_secretKey, &CONFIG_SECRET_KEY, 32);
  memcpy(_hostIP, &CONFIG_HOST_IP, 4);
  _hostPort = CONFIG_HOST_PORT;
  memcpy(_wifiSSID, CONFIG_WIFI_SSID, min(31, (int) strlen(CONFIG_WIFI_SSID)));
  memcpy(_wifiPassword, CONFIG_WIFI_SSID, min(63, (int) strlen(CONFIG_WIFI_SSID)));
  _setpoint = CONFIG_INITIAL_SETPOINT;
  if (CONFIG_HAS_BOARD_SENSOR) {
    _configurationFlags = 0x01;
  }
  _oneWirePin = CONFIG_ONE_WIRE_PIN;
  _enablePin = CONFIG_SIGNAL_ENABLE_PIN;
  _redIndicatorPin = CONFIG_RED_INDICATOR_PIN;
  _greenIndicatorPin = CONFIG_GREEN_INDICATOR_PIN;
  _chillerPinCount = 1;
  _chillerPins = new uint8_t[1] { CONFIG_CHILLER_PIN };
  _heaterPinCount = 1;
  _heaterPins = new uint8_t[1] { CONFIG_HEATER_PIN };
}

bool PipsqueakConfig::detectBoardSensor(byte * addresses, size_t count) {
  bool previouslyDetected = _boardSensorAddress[0] != 0x00;
  if (previouslyDetected) {
    for (size_t i = 0; i < count; i++) {
      if (memcmp(&addresses[i * DS18B20_ADDRESS_SIZE], _boardSensorAddress, DS18B20_ADDRESS_SIZE) == 0) {
        Serial.println("PipsqueakConfig.detectBoardSensor(): detected previously configured sensor");
        return true;
      }
    }
    Serial.println("PipsqueakConfig.detectBoardSensor(): failed to detect previously configured sensor");
  } else if (count == 1) {
    memcpy(_boardSensorAddress, addresses, DS18B20_ADDRESS_SIZE);
    Serial.println("PipsqueakConfig.detectBoardSensor(): detected previously unknown sensor");
    return true;
  } else if (count > 1) {
    Serial.println("PipsqueakConfig.detectBoardSensor(): failed to detected previously unknown sensor (more than one sensor found)");
  } else {
    Serial.println("PipsqueakConfig.detectBoardSensor(): failed to detected previously unknown sensor (no sensors found)");
  }
  return false;
}

byte * PipsqueakConfig::boardSensorAddress() {
  return _boardSensorAddress;
}

bool PipsqueakConfig::detectRemoteSensor(byte * addresses, size_t count) {
  bool previouslyDetectedBoardSensor = _boardSensorAddress[0] != 0x00;
  bool previouslyDetected = _remoteSensorAddress[0] != 0x00;
  if (!previouslyDetectedBoardSensor) {
    Serial.println("PipsqueakConfig.detectRemoteSensor(): unable to detect remote sensor before board sensor detection");
  } else if (previouslyDetected) {
    for (size_t i = 0; i < count; i++) {
      if (memcmp(&addresses[i * DS18B20_ADDRESS_SIZE], _remoteSensorAddress, DS18B20_ADDRESS_SIZE) == 0) {
        Serial.println("PipsqueakConfig.detectRemoteSensor(): detected previously configured sensor");
        return true;
      }
    }
    Serial.println("PipsqueakConfig.detectRemoteSensor(): failed to detect previously configured sensor");
  } else if (count == 2 && detectBoardSensor(addresses, count)) {
    for (size_t i = 0; i < count; i++) {
      if (memcmp(&addresses[i * DS18B20_ADDRESS_SIZE], _boardSensorAddress, DS18B20_ADDRESS_SIZE) != 0) {
        Serial.println("PipsqueakConfig.detectRemoteSensor(): detected previously unknown sensor");
        memcpy(_remoteSensorAddress, &addresses[i * DS18B20_ADDRESS_SIZE], DS18B20_ADDRESS_SIZE);
        return true;
      }
    }
    Serial.println("PipsqueakConfig.detectRemoteSensor(): impossible error?");
  } else if (count > 2) {
    Serial.println("PipsqueakConfig.detectBoardSensor(): failed to detected previously unknown sensor (more than two sensors found)");
  } else {
    Serial.println("PipsqueakConfig.detectBoardSensor(): failed to detected previously unknown sensor (fewer than two sensors found)");
  }
  return false;
}

byte * PipsqueakConfig::remoteSensorAddress() {
  return _remoteSensorAddress;
}

bool PipsqueakConfig::updated() {
  PipsqueakConfig * savedConfig = new PipsqueakConfig();
  savedConfig->readContents();
  uint8_t i;
  if (_previouslyWrittenFlag != INITIALIZED_MARKER) return true;
  if (_configurationFlags != savedConfig->_configurationFlags) return true;
  if (_deviceID != savedConfig->_deviceID) return true;
  if (_setpoint != savedConfig->_setpoint) return true;
  if (_hostIP[0] != savedConfig->_hostIP[0]) return true;
  if (_hostIP[1] != savedConfig->_hostIP[1]) return true;
  if (_hostIP[2] != savedConfig->_hostIP[2]) return true;
  if (_hostIP[3] != savedConfig->_hostIP[3]) return true;
  if (_hostPort != savedConfig->_hostPort) return true;
  if (strcmp(&_wifiSSID[0], &(savedConfig->_wifiSSID)[0]) != 0) return true;
  if (strcmp(&_wifiPassword[0], &(savedConfig->_wifiPassword)[0]) != 0) return true;
  for (i = 0; i < 32; i++) {
    if (_secretKey[i] != (savedConfig->_secretKey)[i]) return true;
  }
  if (_enablePin != savedConfig->_enablePin) return true;
  if (_oneWirePin != savedConfig->_oneWirePin) return true;
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
    if (_boardSensorAddress[i] != savedConfig->_boardSensorAddress[i]) return true;
  }
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
    if (_remoteSensorAddress[i] != savedConfig->_remoteSensorAddress[i]) return true;
  }
  if (_redIndicatorPin != savedConfig->_redIndicatorPin) return true;
  if (_greenIndicatorPin != savedConfig->_greenIndicatorPin) return true;
  if (_heaterPinCount != savedConfig->_heaterPinCount) return true;
  for (i = 0; i < _heaterPinCount; i++) {
    if (_heaterPins[i] != (savedConfig->_heaterPins)[i]) return true;
  }
  if (_chillerPinCount != savedConfig->_chillerPinCount) return true;
  for (i = 0; i < _chillerPinCount; i++) {
    if (_chillerPins[i] != (savedConfig->_chillerPins)[i]) return true;
  }
  delete savedConfig;
  return false;
}

void PipsqueakConfig::printContents() {
  uint8_t i;
  Serial.print("\n");
  Serial.println("-- CONTENTS -----------------------------------------------------------------");
  Serial.print("\n");
  Serial.printf("Previously Written Flag: 0x%02X\n", _previouslyWrittenFlag);
  Serial.printf("Schema Version: %u\n", _schemaVersion);
  Serial.printf("Configuration Flags: %u\n", _configurationFlags);
  Serial.printf("Write Count: %u\n", _writeCount);
  Serial.printf("Device ID: %u\n", _deviceID);
  Serial.printf("Setpoint: %f degrees C\n", _setpoint);
  Serial.printf("Host IP: %u.%u.%u.%u\n", _hostIP[0], _hostIP[1], _hostIP[2], _hostIP[3]);
  Serial.printf("Host Port: %u\n", _hostPort);
  Serial.printf("WiFi SSID: %s\n", _wifiSSID);
  Serial.printf("WiFi Password: %s\n", _wifiPassword);
  Serial.print("Secret key: ");
  for (i = 0; i < 32; i++) {
    Serial.printf("0x%02X", _secretKey[i]);
    if (i < 31) {
      Serial.print(" ");
    }
  }
  Serial.print("\n");
  Serial.printf("Enable Pin: GPIO%u\n", _enablePin);
  Serial.printf("1Wire Pin: GPIO%u\n", _oneWirePin);
  Serial.print("Onboard Temperature Sensor Address: ");
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
    Serial.printf("0x%02X", _boardSensorAddress[i]);
    if (i < DS18B20_ADDRESS_SIZE - 1) {
      Serial.print(" ");
    }
  }
  Serial.print("\n");
  Serial.print("Remote Temperature Sensor Address: ");
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
    Serial.printf("0x%02X", _remoteSensorAddress[i]);
    if (i < DS18B20_ADDRESS_SIZE - 1) {
      Serial.print(" ");
    }
  }
  Serial.print("\n");
  Serial.printf("Red Indicator Pin: GPIO%u\n", _redIndicatorPin);
  Serial.printf("Green Indicator Pin: GPIO%u\n", _greenIndicatorPin);
  Serial.printf("Heater pin count: %u\n", _heaterPinCount);
  Serial.print("Heater Pins: ");
  for (i = 0; i < _heaterPinCount; i++) {
    Serial.printf("GPIO%u", _heaterPins[i]);
    if (i < (_heaterPinCount - 1)) {
      Serial.print(", ");
    }
  }
  Serial.print("\n");
  Serial.printf("Chiller pin count: %u\n", _chillerPinCount);
  Serial.print("Chiller pins: ");
  for (i = 0; i < _chillerPinCount; i++) {
    Serial.printf("GPIO%u", _chillerPins[i]);
    if (i < (_chillerPinCount - 1)) {
      Serial.print(", ");
    }
  }
  Serial.print("\n");
  Serial.print("\n");
  Serial.println("-- END OF CONTENTS ----------------------------------------------------------");
  Serial.print("\n");
}

bool PipsqueakConfig::commitUpdates() {
  if (!updatesAreValid()) return false;
  if (DRY_RUN) return false;
  if (previouslyInitialized() && !ALLOW_UPDATE) return false;

  EEPROM.begin(256);
  size_t cursor;
  size_t i;

  // First nullify the entire buffer
  for (cursor = 0; cursor < 256; cursor++) EEPROM.put(cursor, 0x00);

  // Now backfill with new values
  cursor = 0;
  EEPROM.put(cursor, INITIALIZED_MARKER);
  cursor += 1;
  EEPROM.put(cursor, _schemaVersion);
  cursor += 1;
  EEPROM.put(cursor, _configurationFlags);
  cursor += 1;
  // reserved
  cursor += 1;
  if (previouslyInitialized()) {
    _writeCount = _writeCount + 1;
  } else {
    _writeCount = 1;
  }
  EEPROM.put(cursor, _writeCount);
  cursor += 4;
  EEPROM.put(cursor, _deviceID);
  cursor += 4;
  EEPROM.put(cursor, _setpoint);
  cursor += 4;
  for (i = 0; i < 4; i++) EEPROM.put(cursor + i, _hostIP[i]);
  cursor += 4;
  EEPROM.put(cursor, _hostPort);
  cursor += 2;
  EEPROM.put(cursor, _wifiSSID);
  cursor += 31;
  EEPROM.put(cursor, 0x00);
  cursor += 1;
  EEPROM.put(cursor, _wifiPassword);
  cursor += 63;
  EEPROM.put(cursor, 0x00);
  cursor += 1;
  for (i = 0; i < 32; i++) EEPROM.put(cursor + i, _secretKey[i]);
  cursor += 32;
  EEPROM.put(cursor, _enablePin);
  cursor += 1;
  EEPROM.put(cursor, _oneWirePin);
  cursor += 1;
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) EEPROM.put(cursor + i, _boardSensorAddress[i]);
  cursor += DS18B20_ADDRESS_SIZE;
  EEPROM.put(cursor, _oneWirePin);
  cursor += 1;
  for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) EEPROM.put(cursor + i, _remoteSensorAddress[i]);
  cursor += DS18B20_ADDRESS_SIZE;
  EEPROM.put(cursor, _redIndicatorPin);
  cursor += 1;
  EEPROM.put(cursor, _greenIndicatorPin);
  cursor += 1;
  EEPROM.put(cursor, _heaterPinCount);
  cursor += 1;
  for (i = 0; i < _heaterPinCount; i++) EEPROM.put(cursor + i, _heaterPins[i]);
  cursor += _heaterPinCount;
  EEPROM.put(cursor, _chillerPinCount);
  cursor += 1;
  for (i = 0; i < _chillerPinCount; i++) EEPROM.put(cursor + i, _chillerPins[i]);
  cursor += _chillerPinCount;
  EEPROM.commit();

  Serial.printf("PipsqueakConfig.commitUpdates(): completed flash write number %u\n", _writeCount);
  return true;
}

bool PipsqueakConfig::verifyContents() {
  PipsqueakConfig * savedConfig = new PipsqueakConfig();
  savedConfig->readContents();

  size_t errorCount = 0;
  size_t i;

  Serial.print("\n");
  Serial.println("-- VERIFICATION -------------------------------------------------------------");
  Serial.print("\n");
  if (savedConfig->_previouslyWrittenFlag == INITIALIZED_MARKER) {
    Serial.println("[ OK ] Previously Written Flag");
  } else {
    Serial.printf("[ !! ] Previously Written Flag: 0x%02X (expected 0x%02X)\n", savedConfig->_previouslyWrittenFlag, INITIALIZED_MARKER);
    errorCount += 1;
  }

  if (_schemaVersion == savedConfig->_schemaVersion) {
    Serial.println("[ OK ] Schema Version");
  } else {
    Serial.printf("[ !! ] Schema Version: %u (expected %u)\n", _schemaVersion, savedConfig->_schemaVersion);
    errorCount += 1;
  }

  if (_writeCount == savedConfig->_writeCount) {
    Serial.println("[ OK ] Write Count");
  } else {
    Serial.printf("[ !! ] Write Count: %u (expected %u)\n", _writeCount, savedConfig->_writeCount);
    errorCount += 1;
  }

  if (_configurationFlags == savedConfig->_configurationFlags) {
    Serial.println("[ OK ] Configuration Flags");
  } else {
    Serial.printf("[ !! ] Configuration Flags: %u (expected %u)\n", _configurationFlags, savedConfig->_configurationFlags);
    errorCount += 1;
  }

  if (_deviceID == savedConfig->_deviceID) {
    Serial.println("[ OK ] Device ID");
  } else {
    Serial.printf("[ !! ] Device ID: %u (expected %u)\n", _deviceID, savedConfig->_deviceID);
    errorCount += 1;
  }

  if (_setpoint == savedConfig->_setpoint) {
    Serial.println("[ OK ] Setpoint");
  } else {
    Serial.printf("[ !! ] Setpoint: %f (expected %f)\n", _setpoint, savedConfig->_setpoint);
    errorCount += 1;
  }

  if (memcmp(_hostIP, savedConfig->_hostIP, 4) == 0) {
    Serial.println("[ OK ] Host IP");
  } else {
    Serial.printf("[ !! ] Host IP: %u.%u.%u.%u (expected %u.%u.%u.%u)\n", _hostIP[0], _hostIP[1], _hostIP[2], _hostIP[3], savedConfig->_hostIP[0], savedConfig->_hostIP[1], savedConfig->_hostIP[2], savedConfig->_hostIP[3]);
    errorCount += 1;
  }

  if (_hostPort == savedConfig->_hostPort) {
    Serial.println("[ OK ] Host Port");
  } else {
    Serial.printf("[ !! ] Host Port: %u (expected %u)\n", _hostPort, savedConfig->_hostPort);
    errorCount += 1;
  }

  if (strlen(_wifiSSID) == strlen(savedConfig->_wifiSSID) && strcmp(_wifiSSID, savedConfig->_wifiSSID) == 0) {
    Serial.println("[ OK ] Wifi SSID");
  } else {
    Serial.printf("[ !! ] WiFi SSID: %s (expected %s)\n", _wifiSSID, savedConfig->_wifiSSID);
    errorCount += 1;
  }

  if (strlen(_wifiPassword) == strlen(savedConfig->_wifiPassword) && strcmp(_wifiPassword, savedConfig->_wifiPassword) == 0) {
    Serial.println("[ OK ] Wifi Password");
  } else {
    Serial.printf("[ !! ] WiFi Password: %s (expected %s)\n", _wifiPassword, savedConfig->_wifiPassword);
    errorCount += 1;
  }

  if (memcmp(_secretKey, savedConfig->_secretKey, 32) == 0) {
    Serial.println("[ OK ] Secret Key");
  } else {
    Serial.print("[ !! ] Secret Key   (Actual):  ");
    for (i = 0; i < 32; i++) {
      Serial.printf("0x%02X", savedConfig->_secretKey[i]);
      if (i < 31) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    Serial.print("[ !! ] Secret Key (Expected):  ");
    for (i = 0; i < 32; i++) {
      Serial.printf("0x%02X", savedConfig->_secretKey[i]);
      if (i < 31) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    errorCount += 1;
  }

  if (_enablePin == savedConfig->_enablePin) {
    Serial.println("[ OK ] Enable Pin");
  } else {
    Serial.printf("[ !! ] Enable Pin: GPIO%u (expected GPIO%u)\n", _enablePin, savedConfig->_enablePin);
    errorCount += 1;
  }

  if (_oneWirePin == savedConfig->_oneWirePin) {
    Serial.println("[ OK ] 1Wire Pin");
  } else {
    Serial.printf("[ !! ] 1Wire Pin: GPIO%u (expected GPIO%u)\n", _oneWirePin, savedConfig->_oneWirePin);
    errorCount += 1;
  }

  if (memcmp(_boardSensorAddress, savedConfig->_boardSensorAddress, 8) == 0) {
    Serial.println("[ OK ] Onboard Temperature Sensor Address");
  } else {
    Serial.print("[ !! ] Onboard Temperature Sensor Address   (Actual):  ");
    for (i = 0; i < 8; i++) {
      Serial.printf("0x%02X", _boardSensorAddress[i]);
      if (i < 7) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    Serial.print("[ !! ] Onboard Temperature Sensor Address (Expected):  ");
    for (i = 0; i < 8; i++) {
      Serial.printf("0x%02X", savedConfig->_boardSensorAddress[i]);
      if (i < 7) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    errorCount += 1;
  }

  if (memcmp(_remoteSensorAddress, savedConfig->_remoteSensorAddress, DS18B20_ADDRESS_SIZE) == 0) {
    Serial.println("[ OK ] Remote Temperature Sensor Address");
  } else {
    Serial.print("[ !! ] Remote Temperature Sensor Address   (Actual):  ");
    for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
      Serial.printf("0x%02X", _remoteSensorAddress[i]);
      if (i < DS18B20_ADDRESS_SIZE - 1) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    Serial.print("[ !! ] Remote Temperature Sensor Address (Expected):  ");
    for (i = 0; i < DS18B20_ADDRESS_SIZE; i++) {
      Serial.printf("0x%02X", savedConfig->_remoteSensorAddress[i]);
      if (i < DS18B20_ADDRESS_SIZE - 1) {
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    errorCount += 1;
  }

  if (_redIndicatorPin == savedConfig->_redIndicatorPin) {
    Serial.println("[ OK ] Red Indicator Pin");
  } else {
    Serial.printf("[ !! ] Red Indicator Pin: GPIO%u (expected GPIO%u)\n", _redIndicatorPin, savedConfig->_redIndicatorPin);
    errorCount += 1;
  }

  if (_greenIndicatorPin == savedConfig->_greenIndicatorPin) {
    Serial.println("[ OK ] Green Indicator Pin");
  } else {
    Serial.printf("[ !! ] Green Indicator Pin: GPIO%u (expected GPIO%u)\n", _greenIndicatorPin, savedConfig->_greenIndicatorPin);
    errorCount += 1;
  }

  if (_heaterPinCount == savedConfig->_heaterPinCount) {
    Serial.println("[ OK ] Heater Pin Count");
    for (i = 0; i < _heaterPinCount; i++) {
      if (_heaterPins[i] == savedConfig->_heaterPins[i]) {
        Serial.printf("[ OK ] Heater Pin %u\n", i);
      } else {
        Serial.printf("[ !! ] Heater Pin #%u: GPIO%u (expected %u)\n", i + 1, _heaterPins[i], savedConfig->_heaterPins[i]);
        errorCount += 1;
      }
    }
  } else {
    Serial.printf("[ !! ] Heater Pin Count: %u (expected %u)\n", _heaterPinCount, savedConfig->_heaterPinCount);
    errorCount += 1;
  }

  if (_chillerPinCount == savedConfig->_chillerPinCount) {
    Serial.println("[ OK ] Chiller Pin Count");
    for (i = 0; i < _chillerPinCount; i++) {
      if (_chillerPins[i] == savedConfig->_chillerPins[i]) {
        Serial.printf("[ OK ] Chiller Pin %u\n", i);
      } else {
        Serial.printf("[ !! ] Chiller Pin #%u: GPIO%u (expected GPIO%u)\n", i + 1, _chillerPins[i], savedConfig->_chillerPins[i]);
        errorCount += 1;
      }
    }
  } else {
    Serial.printf("[ !! ] Chiller Pin Count: %u (expected %u)\n", _chillerPinCount, savedConfig->_chillerPinCount);
    errorCount += 1;
  }

  Serial.print("\n");

  bool success;
  if (errorCount == 0) {
    Serial.println("No errors detected");
    success = true;
  } else if (errorCount == 1) {
    Serial.println("One error detected");
    return false;
  } else {
    Serial.printf("%u errors detected\n", errorCount);
    success = false;
  }

  Serial.print("\n");
  Serial.println("-- END OF VERIFICATION ------------------------------------------------------");
  Serial.print("\n");

  delete savedConfig;
  return success;
}

bool PipsqueakConfig::updatesAreValid() {
  if (_deviceID == 0) {
    Serial.println("PipsqueakConfig.updatesAreValid(): deviceID cannot be zero");
    return false;
  }
  if (nullCount((const byte *) _secretKey, 32) == 32) {
    Serial.println("PipsqueakConfig.updatesAreValid(): secret key is all nulls");
    return false;
  }
  if (nullCount((const byte *) _boardSensorAddress, DS18B20_ADDRESS_SIZE) == DS18B20_ADDRESS_SIZE) {
    Serial.println("PipsqueakConfig.updatesAreValid(): board sensor address not configured");
    return false;
  }
  if (_hostIP[0] == 127 && _hostIP[1] == 0 && _hostIP[2] == 0 && _hostIP[3] == 1) {
    Serial.println("PipsqueakConfig.updatesAreValid(): host IP is the loopback address");
    return false;
  }
  if (_hostPort == 0) {
    Serial.println("PipsqueakConfig.updatesAreValid(): host port is zero");
    return false;
  }
  if (strcmp(_wifiSSID, "PlaceholderForSSID") == 0 || strlen(CONFIG_WIFI_SSID) > 31) {
    Serial.println("PipsqueakConfig.updatesAreValid(): WiFi SSID is placeholder value or too long");
    return false;
  }
  if (strcmp(_wifiPassword, "PlaceholderForPassword") == 0 || strlen(CONFIG_WIFI_PASSWORD) > 63) {
    Serial.println("PipsqueakConfig.updatesAreValid(): WiFi password is placeholder value or too long");
    return false;
  }
  if (_setpoint <= 4 || _setpoint > 35) {
    Serial.println("PipsqueakConfig.updatesAreValid(): intial setpoint is too low or too high");
    return false;
  }
  if (_heaterPinCount != 1 || _chillerPinCount != 1) {
    Serial.println("PipsqueakConfig.updatesAreValid(): incorrect heater or chiller pin count (must be 1)");
    return false;
  }
  return true;
}

size_t PipsqueakConfig::nullCount(const byte * subject, size_t limit) {
  size_t count = 0;
  for (size_t i = 0; i < limit; i++) {
    if (subject[i] == 0x00) count += 1;
  }
  return count;
}
