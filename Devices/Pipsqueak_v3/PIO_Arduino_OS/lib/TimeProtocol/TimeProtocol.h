#ifndef TimeProtocol_h
#define TimeProtocol_h

#include <Arduino.h>
#include <Request.h>
#include <Response.h>

#define TIME_PROTOCOL_ID 0x00
#define TIME_REQUEST_SIZE REQUEST_BASE_SIZE
#define TIME_RESPONSE_SIZE RESPONSE_BASE_SIZE

/**
 * Parses and encapsulates the server's TimeRequest
 * response, exposing the server clock setting via
 * Response::getTimestamp().
 */
class TimeResponse: public Response {
  public:
    TimeResponse(Hmac * hmac);

  protected:
    volatile byte * getBuffer();
    byte * getPayload();
    size_t getExpectedSize();
    uint8_t getExpectedProtocol();

  private:
    volatile byte _buffer[TIME_RESPONSE_SIZE];
    byte _payload[TIME_RESPONSE_SIZE];
};


/**
 * Used to synchronize the client device's clock
 * with the server's clock.
 */
class TimeRequest: public Request {
  public:
    TimeRequest(uint32_t deviceID, Hmac * hmac);

    /** See Request.reset() */
    void reset();

    /** See Request.getSize() */
    size_t getSize();

    /** See Request.getResponse() */
    TimeResponse * getResponse();

    /** See Request.getBuffer() */
    byte * getBuffer();

  private:
    byte _buffer[TIME_REQUEST_SIZE];
    TimeResponse _response;
};

#endif // TimeProtocol_h
