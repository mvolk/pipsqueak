#include "DS18B20Bus.h"
#include <InitializationParameters.h>

// The Maxim DS18B20 identifier byte (first byte in chip's 1Wire address)
#define DS18B20_FAMILY_CODE       0x28

// DS18B20 command bytes
#define COMMAND_READ_SCRATCHPAD   0xBE
#define COMMAND_WRITE_SCRATCHPAD  0x4E
#define COMMAND_COPY_SCRATCHPAD   0x48

// DS18B20 location of bytes in the scratchpad
#define INDEX_CONFIG              4
#define INDEX_CRC                 8

// DS18B20 alarm on extremes only (-55 / 125)
#define ALARM_HIGH_BYTE           0x7D
#define ALARM_LOW_BYTE            0xC9

// DS18B20 DATA_CONFIG_INDEX byte
#define CONFIG_9_BIT              0x1F
#define CONFIG_10_BIT             0x3F
#define CONFIG_11_BIT             0x5F
#define CONFIG_12_BIT             0x7F


DS18B20Bus::DS18B20Bus() {
  _oneWire = new OneWire(CONFIG_ONE_WIRE_PIN);
  memset(_scratchpad, 0, DS18B20_SCRATCHPAD_SIZE);
}

size_t DS18B20Bus::search(byte * buffer, size_t maxCount) {
  bool crcError = false;
  byte addressBuffer[8];
  size_t count = 0;
  do {
    Serial.println("Begin 1Wire device detection attempt");
    memset(buffer, 0, DS18B20_ADDRESS_SIZE * maxCount);
    memset(addressBuffer, 0, DS18B20_ADDRESS_SIZE * maxCount);
    crcError = false;
    while (_oneWire->search(addressBuffer)) {
      if (!OneWire::crc8(addressBuffer, 7) == addressBuffer[7]) {
        Serial.println("Device detected on 1Wire bus, but address fails CRC check");
        crcError = true;
        break;
      } else if (addressBuffer[0] != DS18B20_FAMILY_CODE) {
        Serial.println("Device detected on 1Wire bus, but it is not a DS18B20");
      } else {
        if (count < maxCount) {
          memcpy(&buffer[count * DS18B20_ADDRESS_SIZE], addressBuffer, DS18B20_ADDRESS_SIZE);
        }
        count++;
        Serial.printf("DS18B20 detected DS18B20 device #%u on 1Wire bus\n", count);
      }
    }
    _oneWire->reset_search();
    Serial.println("End 1Wire device detection attempt");
    Serial.print("\n");
    yield();
  } while (crcError);

  return count;
}

uint8_t DS18B20Bus::getResolution(const byte * address) {
  bool successfulRead = false;
  do {
    successfulRead = readScratchpad(address);
    Serial.printf("DS18B20Bus.getResolution(): %ssuccessful scratchpad read\n", successfulRead ? "" : "un");
    yield();
  } while (!successfulRead);

  return getResolutionValue(_scratchpad[INDEX_CONFIG]);
}

void DS18B20Bus::setResolution(const byte * address, uint8_t resolution) {
  uint8_t desiredResolution = constrain(resolution, 9, 12);
  uint8_t originalResolution = getResolution(address);
  uint8_t currentResolution = originalResolution;
  while (desiredResolution != currentResolution) {
    Serial.printf("DS18B20Bus.setResolution(): attempting to update resolution config from %u bit to %u bit\n", currentResolution, desiredResolution);

    // write the new configuration
    _oneWire->reset();
    _oneWire->select(address);
    _oneWire->write(COMMAND_WRITE_SCRATCHPAD);
    _oneWire->write(ALARM_HIGH_BYTE);
    _oneWire->write(ALARM_LOW_BYTE);
    _oneWire->write(getResolutionConfigValue(resolution));

    // ask the sensor to persist the configuration to EEPROM
    _oneWire->reset();
    _oneWire->select(address);
    _oneWire->write(COMMAND_COPY_SCRATCHPAD);

    currentResolution = getResolution(address);
  }

  Serial.printf("DS18B20Bus.setResolution(): updated resolution config from %u bit to %u bit\n", originalResolution, currentResolution);
}

bool DS18B20Bus::readScratchpad(const byte * address) {
  _oneWire->reset();
  _oneWire->select(address);
  _oneWire->write(COMMAND_READ_SCRATCHPAD);
  for (uint8_t i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++) {
    _scratchpad[i] = _oneWire->read();
  }
  return OneWire::crc8(_scratchpad, INDEX_CRC) == _scratchpad[INDEX_CRC];
}

byte DS18B20Bus::getResolutionConfigValue(uint8_t resolution) {
  switch (constrain(resolution, 9, 12)) {
    case 9: return CONFIG_9_BIT;
    case 10: return CONFIG_10_BIT;
    case 11: return CONFIG_11_BIT;
    case 12:
    default: return CONFIG_12_BIT;
  }
}

uint8_t DS18B20Bus::getResolutionValue(byte configValue) {
  switch (configValue) {
    case CONFIG_9_BIT: return 9;
    case CONFIG_10_BIT: return 10;
    case CONFIG_11_BIT: return 11;
    case CONFIG_12_BIT:
    default: return 12;
  }
}
