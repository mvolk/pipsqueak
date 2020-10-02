#ifndef PipsqueakConfig_h
#define PipsqueakConfig_h

#include <Arduino.h>
#include <DS18B20Bus.h>

class PipsqueakConfig {
  public:
    PipsqueakConfig();

    /**
     * Reads the contents of non-volatile memory into the
     * volatile memory of this object, unless
     * IGNORE_PREVIOUS_CONFIG in InitializationParameters.h
     * is true of the virtual EEPROM has not been initialized.
     * In either of those two cases, the values in this object
     * will be set to those specified in InitializationParameters.h.
     */
    void readContents();

    /**
     * Indicates whether the non-volatile memory has previously
     * been initialized.
     */
    bool previouslyInitialized();

    /**
     * Invoke after readContents() to apply updates, if any,
     * from InitializationParameters.h to the volatile memory
     * representation of the configuration.
     */
    void applyUpdates();

    /**
     * Finds the board sensor address among those detected, if
     * possible. Returns true if found, false otherwise.
     */
    bool detectBoardSensor(byte * addresses, size_t count);

    /**
     * Returns the board sensor address if known, or a pointer
     * to a null byte.
     */
    byte * boardSensorAddress();

    /**
     * Finds the remote sensor address among those detected, if
     * possible. Returns true if found, false otherwise.
     */
    bool detectRemoteSensor(byte * addresses, size_t count);

    /**
     * Returns the remote sensor address if known, or a pointer
     * to a null byte.
     */
    byte * remoteSensorAddress();

    /**
     * Indicates whether the configuration in volatile memory
     * has been updated or is otherwise different from that
     * in non-volatile memory.
     *
     * Always true if the previous config is being ignored
     * via IGNORE_PREVIOUS_CONFIG in InitializationParameters.h
     */
    bool updated();

    /**
     * Prints the configuration in volatile memory to Serial.
     */
    void printContents();

    /**
     * Commits the configuration in volatile memory to non-volatile
     * memory, unless DRY_RUN is enabled, or unless ALLOW_UPDATES
     * is false and the non-volatile memory has already been
     * initialized.
     * Returns true if the updates are valid and have been committed.
     */
    bool commitUpdates();

    /**
     * Verifies that the configuration in volatile memory matches
     * the configuration in non-volatile memory. Logs comparisons
     * to Serial and returns true if equivalent, false otherwise.
     * Useful for verifying that commits were successful.
     */
    bool verifyContents();

  private:
    uint8_t _previouslyWrittenFlag;
    uint32_t _writeCount;
    uint8_t _schemaVersion;
    byte _configurationFlags;
    uint32_t _deviceID;
    float _setpoint;
    uint8_t _hostIP[4];
    uint16_t _hostPort;
    char _wifiSSID[32];
    char _wifiPassword[64];
    byte _secretKey[32];
    uint8_t _enablePin;
    uint8_t _oneWirePin;
    uint8_t _redIndicatorPin;
    uint8_t _greenIndicatorPin;
    uint8_t _heaterPinCount;
    uint8_t * _heaterPins;
    uint8_t _chillerPinCount;
    uint8_t * _chillerPins;
    byte _boardSensorAddress[DS18B20_ADDRESS_SIZE];
    byte _remoteSensorAddress[DS18B20_ADDRESS_SIZE];

    bool updatesAreValid();
    size_t nullCount(const byte * subject, size_t limit);
};

#endif // PipsqueakConfig
