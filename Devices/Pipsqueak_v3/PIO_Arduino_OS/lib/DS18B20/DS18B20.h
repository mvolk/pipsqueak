#ifndef DS18B20_h
#define DS18B20_h

#include <Arduino.h>
#include <OneWire.h>

#define DS18B20_HISTORY_SIZE 10
#define DS18B20_ADDRESS_SIZE 8
#define DS18B20_SCRATCHPAD_SIZE 9

// Un-comment to enable detailed debug statements
// #define DEBUG_DS18B20 true

class DS18B20 {
  public:
    /**
     * Constructor.
     *
     * address: 8-byte chip address
     * resolution: 9, 10, 11 or 12, representing the bit depth of the reading
     * readingTTL: millisecond lifespan of a reading, beyond which a reading
     *  is considered unreliable.
     */
    DS18B20(OneWire * oneWire, byte * address, byte resolution, uint32_t readingTTL);

    /**
     * Detected DS18B20 devices on the OneWire bus.
     * Writes up to maxCount detected device addresses to the address buffer.
     * Returns the number of addresses found in total, which may exceed
     * maxCount.
     *
     * Each address requires 8 bytes of space in the buffer.
     */
    static size_t detect(OneWire * oneWire, byte * buffer, size_t maxCount);

    /**
     * Begins the "conversion" process via which temperature is read.
     * This process takes some time - less with lower resolutions, more
     * with higher resolutions. Use isReadyToRead() to decide when to
     * invoke the read() method to finish the process.
     *
     * No-op if already sensing.
     */
    void startSensing();

    /**
     * Indicates whether the sensor is currently in the "conversion"
     * process.
     */
    bool isSensing();

    /**
     * Indicates whether the sensor is ready to divulge the current
     * reading.
     */
    bool isReadyToRead();

    /**
     * Attempts to read the temperature from the sensor.
     * Returns true if further calls to this method will not be
     * productive - such as when sensing is not in progress or
     * a fatal error is encountered or a successful read is
     * performed.
     * Return false if reading failed and the method should be
     * called again.
     * If true is returned, the device is no longer sensing. If
     * false is returned, the device is still sensing.
     */
    bool read();

    /**
     * Returns the most recent valid reading, which may be NAN if
     * there is no sufficiently recent valid reading or if not
     * enough readings have been taken to have confidence in the
     * most recent reading.
     */
    float getTemperature();

  private:
    OneWire * _oneWire;
    byte _address[DS18B20_ADDRESS_SIZE];
    byte _scratchpad[DS18B20_SCRATCHPAD_SIZE];
    byte _resolution;
    uint32_t _readingTTL;
    bool _sensing;
    uint32_t _sensingStartMillis;
    size_t _readAttempts;
    uint32_t _lastSuccessfulRead;
    float _readings[DS18B20_HISTORY_SIZE];
    size_t _countOfReadings;
    size_t _cursor;
    float _currentReading;

    byte getResolutionConfigValue();
    uint32_t getConversionTime();
    void updateReading(float reading);
    float averageReading();
    void doneReading();
    byte getSensorResolution();
    void setSensorResolution();
    bool readScratchpad();
};

#endif // DS18B20_h
