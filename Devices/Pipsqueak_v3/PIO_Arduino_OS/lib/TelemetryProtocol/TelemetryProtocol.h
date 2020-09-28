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

// Uncomment for detailed debug statements
// #define DEBUG_TELEMETRY_PROTOCOL true

/**
 * Helper class that builds various event types in the
 * proper format for the request and writes the raw
 * representation into the request buffer.
 *
 * Instances are designed to be re-used.
 * Not thread safe. Not ISR safe.
 */
class StatusEvent {
  public:
    StatusEvent();

    /**
     * Configures this event as a temperature observation event.
     *
     * timestamp: the Unix timestamp of the observation
     * temperature: the observed temperature in Celsius
     */
    void temperatureObservation(uint32_t timestamp, float temperature);

    /**
     * Configures this event as a setpoint change event.
     *
     * timestamp: the Unix timestamp of the observation
     * temperature: the new setpoint in Celsius
     */
    void temperatureSetpoint(uint32_t timestamp, float temperature);

    /**
     * Configures this event as an error event.
     *
     * timestamp: the Unix timestamp of the error
     * errorType: the type of error being reported
     * errorCode: the error code, unique among errors of the given type
     */
    void error(
      uint32_t timestamp,
      ErrorType errorType,
      int8_t errorCode
    );

     /**
     * Configures this event as a heater pulse event.
     *
     * timestamp: the Unix timestamp at the beginning of the cycle
     * pulseDuration: how many milliseconds the heater will be on
     * percentPower: the percentage (0-100) of time that the heater
     *               will be powered during the pulse
     * recoveryDuration: how many milliseconds the heater and chiller
     *                   will remain off at the end of the pulse
     */
    void heaterPulse(
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
     */
   void chillerPulse(
      uint32_t timestamp,
      uint32_t pulseDuration,
      uint32_t recoveryDuration
    );

    /**
     * Writes the current event state to a buffer in the appropriate
     * 16-byte layout called out by the telemetry protocol for the
     * current event type.
     */
    void write(byte * buffer);

    /**
     * Reads the current event state from a 16-byte buffer laid out
     * as specified by the telemetry protocol.
     */
    void read(const byte * buffer);

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
     * Adds a status event to the request if possible.
     *
     * statusEvent: the event to add
     *
     * Returns true if the event has been copied to the request's
     * internal buffer; false otherwise.
     */
    bool addStatusEvent(StatusEvent * statusEvent);

    /**
     * Indicates whether the request can accept additional
     * status events.
     *
     * Returns true if additional status events can be accepted.
     */
    bool isReadyForMoreEvents();

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
};

#endif // TelemetryProtocol_h
