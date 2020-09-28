#ifndef PipsqueakConfig_h
#define PipsqueakConfig_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

// Un-comment to enable detailed debug statements to Serial
// #define DEBUG_PIPSQUEAK_CONFIG

#define WIFI_SSID_BUFFER_SIZE 32
#define WIFI_PASSWORD_BUFFER_SIZE 64
#define SECRET_KEY_BUFFER_SIZE 32
#define BOARD_SENSOR_ADDRESS_SIZE 8

/**
 * Encapsulates access to persistant memory holding
 * configuration data.
 *
 * Not thread safe or ISR-safe.
 */
class PipsqueakConfig {
  public:
    PipsqueakConfig();

    /**
     * This method must be invoked once before using this
     * class. Until this method is invoked, the values
     * returned by other methods of this class will be
     * meaningless initial values.
     */
    void setup();

    /**
     * Returns the WiFi access point ID to which the
     * Pipsqueak should attempt to connect.
     */
    const char * getWifiSSID();

    /**
     * Returns the WiFi access point password that the
     * Pipsqueak should use when connecting to WiFi.
     */
    const char * getWifiPassword();

    /**
     * Returns the IP address of the server that this
     * Pipsqueak should communicate with.
     */
    IPAddress * getHostIP();

    /**
     * Returns the network port number to use when
     * communicating with the server.
     */
    uint16_t getHostPort();

    /**
     * Returns this device's unique ID.
     *
     * Device IDs must be registered with the server
     * in order for requests to be accepted.
     */
    uint32_t getDeviceID();

    /**
     * Returns this device's secret key.
     *
     * This is a 32-byte value used to compute HMACs on
     * requests and responses. It should be unique to this
     * device.
     *
     * This value must be registered with the server as
     * belonging to this device, and must not otherwise
     * be shared or revealed.
     */
    const byte * getSecretKey();

    /**
     * Returns the pin number used to "turn on" heater
     * and chiller signals.
     *
     * Because many esp8266 pins float on startup, this
     * pin is used to ensure that the heater and chiller
     * are not unintentionally activated during startup.
     * Until this pin is set up in output mode and driven
     * high, the electronic circuitry will prevent signals
     * on the heater and chiller pins from acivating the
     * heater or chiller.
     */
    uint8_t getSignalEnablePin();

    /**
     * Returns the pin number of the OneWire pin used to
     * communicate with DS18B20 temperature sensing ICs.
     *
     * Both the onboard and remote sensors use the same
     * pin.
     */
    uint8_t getOneWirePin();

    /**
     * Returns the pin number used to control the red
     * indicator LED. Active high.
     */
    uint8_t getRedIndicatorPin();

    /**
     * Returns the pin number used to control the
     * green indicator LED. Active high.
     */
    uint8_t getGreenIndicatorPin();

    /**
     * Returns the pin number used to control the
     * heater and heater LED. Active high.
     */
    uint8_t getHeaterPin();

    /**
     * Returns the pin number used to control the
     * chiller and chiller LED. Active high.
     */
    uint8_t getChillerPin();

    /**
     * Updates the temperature setpoint in degrees Celsius.
     */
    void setTemperatureSetpoint(float setpoint);

    /**
     * Returns the current temperature setpoint in degrees
     * Celsius.
     */
    float getTemperatureSetpoint();

    /**
     * Determines whether the provided 8-bit chip address
     * is the address of the onboard temperature sensor.
     */
    bool isBoardSensorAddress(uint8_t * address);

    /**
     * Returns the address of the onboard DS18B20
     * temperature sensor IC.
     */
    uint8_t * getBoardSensorAddress();

    /**
     * Returns the threshold temperature, in degrees
     * Celsius, beyond which the board is consider
     * overheated. Activation of the heater or
     * chiller is counterindicated when the board
     * is overheated.
     */
    float getBoardTemperatureLimit();

  private:
    char _wifiSSID[WIFI_SSID_BUFFER_SIZE];
    char _wifiPassword[WIFI_PASSWORD_BUFFER_SIZE];
    IPAddress * _hostIP;
    uint16_t _hostPort;
    uint32_t _deviceID;
    byte _secretKey[SECRET_KEY_BUFFER_SIZE];
    uint8_t _configurationFlags;
    uint8_t _enablePin;
    uint8_t _oneWirePin;
    uint8_t _redIndicatorPin;
    uint8_t _greenIndicatorPin;
    uint8_t _heaterPin;
    uint8_t _chillerPin;
    float _setpoint;
    byte _boardSensorAddress[BOARD_SENSOR_ADDRESS_SIZE];

    void persist();
};

#endif // PipsqueakConfig_h
