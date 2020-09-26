#ifndef SetpointProtocol_h
#define SetpointProtocol_h

#include <Arduino.h>
#include <Request.h>
#include <Response.h>

#define SETPOINT_PROTOCOL_ID 0x01

#define SETPOINT_REQUEST_SIZE REQUEST_BASE_SIZE
#define SETPOINT_REQUEST_REBOOT_FLAG_OFFSET 9

#define SETPOINT_RESPONSE_SIZE RESPONSE_BASE_SIZE
#define SETPOINT_RESPONSE_SETPOINT_OFFSET 5

/**
 * Parses and encapsulates the server's SetpointRequest
 * response, exposing the server clock setting and
 * the setpoint that the server intends for the
 * client device to use.
 */
class SetpointResponse: public Response {
  public:
    SetpointResponse(Hmac * hmac);

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
    volatile byte _buffer[SETPOINT_RESPONSE_SIZE];
    byte _payload[SETPOINT_RESPONSE_SIZE];
};

/**
 * Used to request the temperature setpoint that the
 * client device should be using.
 */
class SetpointRequest: public Request {
  public:
    SetpointRequest(uint32_t deviceID, Hmac * hmac);

    /** See Request.reset() */
    void reset();

    /**
     * Indicates that the setpoint is being
     * requested as part of the reboot process.
     * Provides reboot frequency and timing
     * observability to the server.
     */
    void setReboot();

    /** See Request.getSize() */
    size_t getSize();

    /** See Request.getResponse() */
    SetpointResponse * getResponse();

    /** See Request.getBuffer() */
    byte * getBuffer();

  private:
    byte _buffer[SETPOINT_REQUEST_SIZE];
    SetpointResponse _response;
};

#endif // SetpointProtocol.h
