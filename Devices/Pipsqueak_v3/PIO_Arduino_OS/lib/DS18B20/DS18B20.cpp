#include "DS18B20.h"
#include <math.h>

// DS18B20 identifier byte (first byte in address)
#define DS18B20_FAMILY_CODE         0x28

// DS18B20 command bytes
#define COMMAND_BEGIN_CONVERSION    0x44
#define COMMAND_RECALL_MEMORY       0xB8
#define COMMAND_READ_SCRATCHPAD     0xBE
#define COMMAND_WRITE_SCRATCHPAD    0x4E
#define COMMAND_COPY_SCRATCHPAD     0x48

// DS18B20 location of significant bytes read from scratchpad
#define INDEX_LSB                   0
#define INDEX_MSB                   1
#define INDEX_CONFIG                4
#define INDEX_CRC                   8

// DS18B20 alarm on extremes only (-55 / 125)
#define ALARM_HIGH_BYTE             0x7D
#define ALARM_LOW_BYTE              0xC9

// DS18B20 resolution values for DATA_CONFIG_INDEX byte
#define CONFIG_9_BIT                0x1F
#define CONFIG_10_BIT               0x3F
#define CONFIG_11_BIT               0x5F
#define CONFIG_12_BIT               0x7F

// DS18B20 time to convert
#define CONVERSION_DURATION_9_BIT   94  // ms
#define CONVERSION_DURATION_10_BIT  188 // ms
#define CONVERSION_DURATION_11_BIT  375 // ms
#define CONVERSION_DURATION_12_BIT  750 // ms

// how many times to retry after crc failure
#define MAX_READ_ATTEMPTS           10

// minimum number of readings before we have confidence in our observation
#define DS18B20_MIN_READINGS 3

DS18B20::DS18B20(OneWire * oneWire, byte * address, byte resolution, uint32_t readingTTL)
:
  _sensing { false },
  _sensingStartMillis { 0 },
  _readAttempts { 0 },
  _lastSuccessfulRead { 0 },
  _countOfReadings { 0 },
  _cursor { 0 },
  _currentReading { NAN }
{
  _oneWire = oneWire;
  _resolution = constrain(resolution, 9, 12);
  _readingTTL = readingTTL;
  memcpy(_address, address, DS18B20_ADDRESS_SIZE);
  memset(_scratchpad, 0, DS18B20_SCRATCHPAD_SIZE);
  for (uint8_t i = 0; i < DS18B20_HISTORY_SIZE; i++) _readings[i] = NAN;
  setSensorResolution();
}

size_t DS18B20::detect(OneWire * oneWire, byte * buffer, size_t maxCount) {
  memset(buffer, 0, maxCount * DS18B20_ADDRESS_SIZE);
  size_t count = 0;
  byte * addressBuffer = buffer;
  while (oneWire->search(addressBuffer)) {
    if (!OneWire::crc8(addressBuffer, 7) == addressBuffer[7]) {
      #ifdef DEBUG_DS18B20
      Serial.println("DS18B20.detect(): device detected on 1Wire bus, but address fails CRC check");
      #endif
    } else if (addressBuffer[0] != DS18B20_FAMILY_CODE) {
       #ifdef DEBUG_DS18B20
       Serial.println("DS18B20.detect(): device detected on 1Wire bus, but it is not a DS18B20");
       #endif
    } else if (count >= maxCount) {
      count += 1;
    } else {
      count += 1;
      addressBuffer += DS18B20_ADDRESS_SIZE;
    }
    #ifdef DEBUG_DS18B20
    if (count > maxCount) {
      Serial.printf("DS18B20.detect(): found %u sensors; max of %u expected\n", count, maxCount);
    }
    #endif
    yield();
  }
  oneWire->reset_search();
  return count;
}

void DS18B20::startSensing() {
  if (_sensing) return;
  _oneWire->reset();
  _oneWire->select(_address);
  _oneWire->write(COMMAND_BEGIN_CONVERSION);
  _sensingStartMillis = millis();
  _sensing = true;
}

bool DS18B20::isSensing() {
  return _sensing;
}

bool DS18B20::isReadyToRead() {
  return _sensing && (millis() - _sensingStartMillis) >= getConversionTime();
}

bool DS18B20::read() {
  if (!_sensing) return true;

  if (_readAttempts > MAX_READ_ATTEMPTS) {
    #ifdef DEBUG_DS18B20
    Serial.println("DS18B20.read(): bad CRC on all read attempts");
    #endif
    doneReading();
    return true;
  }

  _readAttempts += 1;

  if (!readScratchpad()) {
    return false;
  }

  // require the specified resolution
  if (_scratchpad[INDEX_CONFIG] != getResolutionConfigValue()) {
    #ifdef DEBUG_DS18B20
    Serial.printf("DS18B20.readSensor(): sensor is not configured for %u-bit resolution\n", _resolution);
    #endif
    setSensorResolution();
    doneReading();
    return true;
  }

  // interpret the raw value
  uint8_t lsb = _scratchpad[INDEX_LSB];
  uint8_t msb = _scratchpad[INDEX_MSB];
  switch (_resolution) {
      case 9:
          lsb &= 0xF8;
          break;
      case 10:
          lsb &= 0xFC;
          break;
      case 11:
          lsb &= 0xFE;
          break;
  }
  uint8_t sign = msb & 0x80;
  int16_t temp = (msb << 8) + lsb;
  if (sign) {
    temp = ((temp ^ 0xffff) + 1) * -1;
  }
  float reading = temp / 16.0;

  #ifdef DEBUG_DS18B20
  Serial.printf("DS18B20.readSensor(): successful read on attempt #%u: %f degrees C\n", _readAttempts, reading);
  #endif
  updateReading(reading);
  doneReading();
  return true;
}

float DS18B20::getTemperature() {
  if (_countOfReadings > 0 && millis() - _lastSuccessfulRead > _readingTTL) {
    _countOfReadings = 0;
    for (uint8_t i = 0; i < DS18B20_HISTORY_SIZE; i++) _readings[i] = NAN;
    _currentReading = NAN;
  }
  return _currentReading;
}

byte DS18B20::getResolutionConfigValue() {
  switch (_resolution) {
    case 9: return CONFIG_9_BIT;
    case 10: return CONFIG_10_BIT;
    case 11: return CONFIG_11_BIT;
    case 12:
    default:
      return CONFIG_12_BIT;
  }
}

uint32_t DS18B20::getConversionTime() {
  switch (_resolution) {
    case 9: return CONVERSION_DURATION_9_BIT;
    case 10: return CONVERSION_DURATION_10_BIT;
    case 11: return CONVERSION_DURATION_11_BIT;
    case 12:
    default:
      return CONVERSION_DURATION_12_BIT;
  }
}

void DS18B20::updateReading(float newReading) {
  // 85.0 C is the power-on reset value of the DS18B20's temperature register, and is read if the sensor isn't properly powered
  if (newReading == 85.0) {
    #ifdef DEBUG_DS18B20
    Serial.println("DS18B20.updateReading(): reading discarded; value is considered an indicator of faulty power to the sensor");
    #endif
    return;
  }

  _readings[_cursor] = newReading;
  _cursor = (_cursor + 1) % DS18B20_HISTORY_SIZE;
  _lastSuccessfulRead = millis();
  if (_countOfReadings < DS18B20_HISTORY_SIZE) _countOfReadings++;

  if (_countOfReadings == DS18B20_MIN_READINGS) {
    #ifdef DEBUG_DS18B20
    Serial.printf("DS18B20.updateReading(): %u readings; VALUE ACCEPTED\n", _countOfReadings);
    #endif
    _currentReading = newReading;
  } else if (_countOfReadings >= DS18B20_MIN_READINGS && newReading != _currentReading) {
    float difference = _currentReading - newReading;
    if (difference < -0.1 || difference > 0.1) {
      // if the difference is large (more than one step), assume this is an immediately meaningful change
      #ifdef DEBUG_DS18B20
      Serial.printf("DS18B20.updateReading(): %u readings; difference sufficient: %f; VALUE ACCEPTED\n", _countOfReadings, difference);
      #endif
      _currentReading = newReading;
    } else {
      // if the difference is small, wait until the running average over 15 seconds is closer to this value than the previous value
      float average = averageReading();
      if (isnan(average)) {
        #ifdef DEBUG_DS18B20
        Serial.println("DS18B20.updateReading(): unexpected NAN on average reading");
        #endif
        return;
      }
      float delta = average - newReading;
      if (delta > -0.03125 && delta < 0.03125) {
        #ifdef DEBUG_DS18B20
        Serial.printf("DS18B20.updateReading(): %u readings; difference insufficient: %f: delta from average sufficient: %f; VALUE ACCEPTED\n", _countOfReadings, difference, delta);
        #endif
        _currentReading = newReading;
      } else {
        #ifdef DEBUG_DS18B20
        Serial.printf("DS18B20.updateReading(): %u readings; difference insufficient: %f: delta from average insufficient: %f; VALUE REJECTED\n", _countOfReadings, difference, delta);
        #endif
      }
    }
  } else {
    #ifdef DEBUG_DS18B20
    Serial.printf("DS18B20.updateReading(): %u readings; no difference from last reading; VALUE UNCHANGED\n", _countOfReadings);
    #endif
  }
}

float DS18B20::averageReading() {
  if (_countOfReadings < DS18B20_MIN_READINGS) return NAN;

  float sum = 0.0;
  size_t i = (_cursor == 0 ? DS18B20_HISTORY_SIZE : _cursor) - 1;
  size_t count = 0;
  #ifdef DEBUG_DS18B20
  Serial.print("DS18B20.averageReading(): average of [");
  #endif
  do {
    #ifdef DEBUG_DS18B20
    Serial.printf("{%u: %f}, ", i, _readings[i]);
    #endif
    sum += _readings[i];
    count++;
    i = (i == 0 ? DS18B20_HISTORY_SIZE : i) - 1;
  } while (count < _countOfReadings);
  float average = sum / (float) _countOfReadings;
  #ifdef DEBUG_DS18B20
  Serial.printf("] = %f; cursor=%u\n", average, _cursor);
  #endif
  return average;
}

void DS18B20::doneReading() {
  _readAttempts = 0;
  _sensing = false;
}

byte DS18B20::getSensorResolution() {
  bool successfulRead = false;
  do {
    successfulRead = readScratchpad();
    #ifdef DEBUG_DS18B20
    Serial.printf("DS18B20.getSensorResolution(): %ssuccessful scratchpad read\n", successfulRead ? "" : "un");
    #endif
    yield();
  } while (!successfulRead);

  return _scratchpad[INDEX_CONFIG];
}

void DS18B20::setSensorResolution() {
  byte originalResolution = getSensorResolution();
  byte currentResolution = originalResolution;
  while (currentResolution != getResolutionConfigValue()) {
    #ifdef DEBUG_DS18B20
    Serial.printf("DS18B20.setSensorResolution(): attempting to update resolution config from 0x%02X to 0x%02X\n", currentResolution, getResolutionConfigValue());
    #endif

    // write the new configuration
    _oneWire->reset();
    _oneWire->select(_address);
    _oneWire->write(COMMAND_WRITE_SCRATCHPAD);
    _oneWire->write(ALARM_HIGH_BYTE);
    _oneWire->write(ALARM_LOW_BYTE);
    _oneWire->write(getResolutionConfigValue());

    // ask the sensor to persist the configuration to EEPROM
    _oneWire->reset();
    _oneWire->select(_address);
    _oneWire->write(COMMAND_COPY_SCRATCHPAD);

    currentResolution = getSensorResolution();
  }

  #ifdef DEBUG_DS18B20
  if (originalResolution == currentResolution) {
    Serial.printf("DS18B20.setSensorResolution(): already configured at %u bit resolution\n", _resolution);
  } else {
    Serial.printf("DS18B20.setSensorResolution(): updated configuration to %u bit resolution\n", _resolution);
  }
  #endif
}

bool DS18B20::readScratchpad() {
  _oneWire->reset();
  _oneWire->select(_address);
  _oneWire->write(COMMAND_READ_SCRATCHPAD);
  for (uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++) {
    _scratchpad[i] = _oneWire->read();
  }
  return OneWire::crc8(_scratchpad, INDEX_CRC) == _scratchpad[INDEX_CRC];
}
