#ifndef InitializationParameters_h
#define InitializationParameters_h

#include <Arduino.h>

// Following are the parameters that can be modified to change
// how, and with what values, a Pipsqueak's configuration will
// be written.



// Initialization Program Options //////////////////////////////

// Whether to report the contents only (readOnly = true)
// or update the contents as needed (readOnly = false)
#define DRY_RUN                  false

// The nuclear option - to start fresh as if the virtual EEPROM
// had never before been initialized
#define IGNORE_PREVIOUS_CONFIG   false

// True to overwrite an already initialized buffer if there are
// changes, false to prevent an already-initialized buffer from
// being modified. Irrelevant if DRY_RUN is true.
#define ALLOW_UPDATE             false



// Device Configuration Values /////////////////////////////////

// The device's unique non-zero positive integer ID. Must be
// unique to one device and must be registered with the server.
#define CONFIG_DEVICE_ID 0


// The port number that this device will use when talking to
// the server. Conventionally 9001.
#define CONFIG_HOST_PORT 9001

// The SSID of the WiFi access point this device will connect
// with. "PlaceholderForSSID" is not an allowable value.
#define CONFIG_WIFI_SSID "PlaceholderForSSID"

// The password this device will use to connect to the WiFi
// access point. "PlaceholderForPassword" is not an allowable
// value.
#define CONFIG_WIFI_PASSWORD "PlaceholderForPassword"

// The initial temperature setpoint, to which the device will
// try to drive the remote temperature using the heater and
// chiller. Usually updated via the server over time, making
// this value merely a reasonable starting point before the
// device first makes contact with the server.
#define CONFIG_INITIAL_SETPOINT 15.0

#ifdef __cplusplus
extern "C" {
#endif

// The 32-byte secret key used by the server and this device to
// compute and verify HMACs on messages passed between the two.
// Must be held secret between the server and device.
// Should be unique to each device.
// All zeros is an illegal value, used as a placeholder.
// The value is defined in InitializationParameters.cpp
extern const byte CONFIG_SECRET_KEY[];

// The IP address of the service this device will talk to.
// 127.0.0.1, the loopback address, is not a legal value.
// The value is defined in InitializationParameters.cpp
extern const uint8_t CONFIG_HOST_IP[];

#ifdef __cplusplus
}
#endif

// More Device Configuration ///////////////////////////////////
// These values are unlikely to need modification

// Whether the device has an on-board temperature sensor.
// All v3 Pipsqueaks should have such a sensor installed.
#define CONFIG_HAS_BOARD_SENSOR true

// The GPIO pin number (or constant that resolves to that pin
// number) to which OneWire devices (e.g. DS18B20 temperature
// sensors) are connected.
#define CONFIG_ONE_WIRE_PIN D2

// The GPIO pin number (or constant that resolves to that pin
// number) that must be driven high before heater, chiller and
// indicator signals will have any effect.
#define CONFIG_SIGNAL_ENABLE_PIN D1

// The GPIO pin number (or constant that resolves to that pin
// number) that lights the red LED indicator light when
// driven high.
#define CONFIG_RED_INDICATOR_PIN D7

// The GPIO pin number (or constant that resolves to that pin
// number) that lights the green LED indicator light when
// driven high.
#define CONFIG_GREEN_INDICATOR_PIN D6

// The GPIO pin number (or constant that resolves to that pin
// number) that lights the blue chiller indicator light and
// switches on the flow of power to the chiller peripheral
// when driven high.
#define CONFIG_CHILLER_PIN D5

// The GPIO pin number (or constant that resolves to that pin
// number) that lights the yellow heater  indicator light and
// switches on the flow of power to the heater peripheral
// when driven high.
#define CONFIG_HEATER_PIN D8


#endif // InitializationParameters_h
