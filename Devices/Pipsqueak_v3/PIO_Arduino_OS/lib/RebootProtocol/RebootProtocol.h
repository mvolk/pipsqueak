#ifndef RebootProtocol_h
#define RebootProtocol_h

#include <Arduino.h>
#include <Request.h>
#include <Response.h>

#define REPORT_REBOOT_PROTOCOL_ID 0x03

#define REPORT_REBOOT_REQUEST_MESSAGE_SIZE_LIMIT 256
#define REPORT_REBOOT_REQUEST_BASE_SIZE REQUEST_BASE_SIZE
#define REPORT_REBOOT_REQUEST_MAX_SIZE REQUEST_BASE_SIZE + REPORT_REBOOT_REQUEST_MESSAGE_SIZE_LIMIT
#define REPORT_REBOOT_REQUEST_MESSAGE_SIZE_OFFSET 14
#define REPORT_REBOOT_REQUEST_MESSAGE_OFFSET REQUEST_HEADER_SIZE

#define REPORT_REBOOT_RESPONSE_SIZE RESPONSE_BASE_SIZE

/**
 * Parses and encapsulates the server's TimeRequest
 * response, exposing the server clock setting via
 * Request.getTimestamp().
 */
class ReportRebootResponse: public Response {
  public:
    ReportRebootResponse(Hmac * hmac);

  protected:
    volatile byte * getBuffer();
    byte * getPayload();
    size_t getExpectedSize();
    uint8_t getExpectedProtocol();

  private:
    volatile byte _buffer[REPORT_REBOOT_RESPONSE_SIZE];
    byte _payload[REPORT_REBOOT_RESPONSE_SIZE];
};


/**
 * Used to report device reboots and their causes to the
 * server for observability and diagnostic work.
 */
class ReportRebootRequest: public Request {
  public:
    ReportRebootRequest(uint32_t deviceID, Hmac * hmac);

    /** See Request.reset() */
    void reset();

    /**
     * Populates the request to indicate a normal reboot
     * (rst_reason::REASON_DEFAULT_RST), not prompted by
     * a crash & subsequent automatic restart.
     */
    void reportNormalReboot();

    /**
     * Populates the request to indicate an reboot due to
     * a crash. The reason string should be formatted for
     * consumption by the Arduino esp8266 exception
     * decoder.
     */
    void reportExceptionalReboot(const char * reason);

    /** See Request.getSize() */
    size_t getSize();

    /** See Request.getResponse() */
    ReportRebootResponse * getResponse();

    /** See Request.getBuffer() */
    byte * getBuffer();

  private:
    byte _buffer[REPORT_REBOOT_REQUEST_MAX_SIZE];
    size_t _size;
    size_t _messageSize;
    ReportRebootResponse _response;

    void localReset();
    void setPayload(const char * message, size_t messageSize);
};

#endif // RebootProtocol_h
