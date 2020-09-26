#ifndef TelemetryProtocol_h
#define TelemetryProtocol_h

#include <Arduino.h>
#include <Request.h>
#include <Response.h>
#include <Errors.h>

#define TELEMETRY_PROTOCOL_ID 0x02

#define STATUS_EVENT_SIZE 16
#define STATUS_EVENT_TYPE_OFFSET 0
#define STATUS_EVENT_TYPE_TEMPERATURE 1
#define STATUS_EVENT_TYPE_SETPOINT 2
#define STATUS_EVENT_TYPE_ERROR 6
#define STATUS_EVENT_TYPE_HEATER 4
#define STATUS_EVENT_TYPE_CHILLER 7
#define STATUS_EVENT_TIMESTAMP_OFFSET 1
#define STATUS_EVENT_TEMPERATURE_OFFSET 5
#define STATUS_EVENT_SETPOINT_OFFSET 5
#define STATUS_EVENT_PULSE_DURATION_OFFSET 5
#define STATUS_EVENT_PERCENT_POWER_OFFSET 9
#define STATUS_EVENT_RECOVERY_DURATION_OFFSET 10
#define STATUS_EVENT_ERROR_TYPE_OFFSET 5
#define STATUS_EVENT_ERROR_CODE_OFFSET 6

/**
 * Helper class that builds various event types in the
 * proper format for the request and writes the raw
 * representation into the request buffer.
 *
 * Instances are designed to be re-used.
 * Not thread safe. Not ISR safe.
 *
 * Built for use within the Request class.
 */
class StatusEvent {
  public:
    StatusEvent();

    /** See Request.addTemperatureEvent(uint32_t, float) */
    void temperatureObservation(uint32_t timestamp, float temperature);

    /** See Request.addSetpointEvent(uint32_t, float) */
    void temperatureSetpoint(uint32_t timestamp, float temperature);

    /** See Request.addErrorEvent(uint32_t, ErrorType, int8_t) */
    void error(
      uint32_t timestamp,
      ErrorType errorType,
      int8_t errorCode
    );

    /** See Request.addHeaterEvent(uint32_t, uint32_t, uint8_t, uint32_t) */
    void heaterCycle(
      uint32_t timestamp,
      uint32_t pulseDuration,
      uint8_t percentPower,
      uint32_t recoveryDuration
    );

    /** See Request.addChillerEvent(uint32_t, uint32_t, uint32_t) */
    void chillerCycle(
      uint32_t timestamp,
      uint32_t pulseDuration,
      uint32_t recoveryDuration
    );

    /**
     * Writes the current event state to a buffer in the appropriate
     * 16-byte layout called out by the telemetry protocol for the
     * current event type. Intended for use by the Request class.
     */
    void write(byte * buffer);

  private:
    byte _payload[STATUS_EVENT_SIZE];

    void reset();
};


#define TELEMETRY_RESPONSE_SIZE RESPONSE_BASE_SIZE
#define TELEMETRY_RESPONSE_SETPOINT_OFFSET 5

/**
 * Parses and encapsulates the server's TelementryRequest
 * response, exposing the server clock setting and
 * the setpoint that the server intends for the
 * client device to use.
 *
 * Used to detect request errors, confirm receipt of events,
 * synchronize the device's clock to the server's clock, and
 * update the device's temperature setpoint based on the
 * desired setpoint from the server.
 */
class TelemetryResponse: public Response {
  public:
    TelemetryResponse(Hmac * hmac);

    /**
     * The setpoint that the server intends for the
     * client device to use.
     *
     * The behavior of this method is undefined if
     * there are response errors (i.e. hasErrors()
     * returns true).
     */
    float getSetpoint();

  protected:
    volatile byte * getBuffer();
    byte * getPayload();
    size_t getExpectedSize();
    uint8_t getExpectedProtocol();

  private:
    volatile byte _buffer[TELEMETRY_RESPONSE_SIZE];
    byte _payload[64];
};


#define TELEMETRY_REQUEST_EVENT_SIZE 16
#define TELEMETRY_REQUEST_EVENT_COUNT_LIMIT 32
#define TELEMETRY_REQUEST_BASE_SIZE REQUEST_BASE_SIZE
#define TELEMETRY_REQUEST_MAX_SIZE REQUEST_BASE_SIZE + (TELEMETRY_REQUEST_EVENT_SIZE * TELEMETRY_REQUEST_EVENT_COUNT_LIMIT)
#define TELEMETRY_REQUEST_COUNT_OFFSET 9
#define TELEMETRY_REQUEST_EVENTS_BUFFER_OFFSET REQUEST_HEADER_SIZE

/**
 * Used to send observations and system state to the server, to
 * receive changes in desired setpoint, and to keep the device's
 * clock in sync with the server.
 */
class TelemetryRequest: public Request {
  public:
    TelemetryRequest(uint32_t deviceID, Hmac * hmac);

    /** See Request::reset() */
    void reset();

    /**
     * Adds a temperature observation event to the request if possible.
     *
     * timestamp: the Unix timestamp of the observation
     * temperature: the observed temperature in Celsius
     *
     * Returns true if the event could be added, false otherwise.
     */
    bool addTemperatureEvent(uint32_t timestamp, float temperature);

    /**
     * Sets up this event as a setpoint report event. Such an
     * event might be created upon bootup to report the current
     * setpoint, or upon change of setpoint.
     *
     * timestamp: the Unix timestamp of the observation
     * temperature: the setpoint as of the timestamp
     *
     * Returns true if the event could be added, false otherwise.
     */
    bool addSetpointEvent(uint32_t timestamp, float temperature);

    /**
     * Sets up this event as an error reporting event.
     *
     * timestamp: the Unix timestamp of the error
     * errorType: the type of error being reported
     * errorCode: the error code, unique among errors of the given type
     *
     * Returns true if the event could be added, false otherwise.
     */
    bool addErrorEvent(
      uint32_t timestamp,
      ErrorType errorType,
      int8_t errorCode
    );

    /**
     * Sets up this event as a heater cycle reporting event.
     *
     * timestamp: the Unix timestamp at the beginning of the cycle
     * pulseDuration: how many milliseconds the heater will be on
     * percentPower: the percentage (0-100) of time that the heater
     *               will be powered during the pulse
     * recoveryDuration: how many milliseconds the heater and chiller
     *                   will remain off at the end of the pulse
     *
     * Returns true if the event could be added, false otherwise.
     */
    bool addHeaterEvent(
      uint32_t timestamp,
      uint32_t pulseDuration,
      uint8_t percentPower,
      uint32_t recoveryDuration
    );

    /**
     * Sets up this event as a chiller cycle reporting event.
     *
     * timestamp: the Unix timestamp at the beginning of the cycle
     * pulseDuration: how many milliseconds the chiller will be on
     * recoveryDuration: how many milliseconds the heater and chiller
     *                   will remain off at the end of the pulse
     *
     * Returns true if the event could be added, false otherwise.
     */
    bool addChillerEvent(
      uint32_t timestamp,
      uint32_t pulseDuration,
      uint32_t recoveryDuration
    );

    /** See Request.getSize() */
    size_t getSize();

    /** See Request.getResponse() */
    TelemetryResponse * getResponse();

    /** See Request.getBuffer() */
    byte * getBuffer();

  private:
    byte _buffer[TELEMETRY_REQUEST_MAX_SIZE];
    StatusEvent _statusEvent;
    uint8_t _eventCount;
    TelemetryResponse _response;

    bool addEvent();
};

#endif // TelemetryProtocol_h
