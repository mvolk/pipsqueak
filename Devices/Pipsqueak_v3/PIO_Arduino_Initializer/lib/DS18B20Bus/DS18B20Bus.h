#ifndef DS18B20Bus_h
#define DS18B20Bus_h

#include <Arduino.h>
#include <OneWire.h>

#define DS18B20_SCRATCHPAD_SIZE 9
#define DS18B20_ADDRESS_SIZE 8

class DS18B20Bus {
  public:
    DS18B20Bus();
    size_t search(byte * buffer, size_t maxCount);
    void setResolution(const byte * address, uint8_t resolution);

  private:
    OneWire * _oneWire;
    byte _scratchpad[DS18B20_SCRATCHPAD_SIZE];

    uint8_t getResolution(const byte * address);
    bool readScratchpad(const byte * address);
    byte getResolutionConfigValue(uint8_t resolution);
    uint8_t getResolutionValue(byte configValue);
};

#endif // DS18B20Bus_h
