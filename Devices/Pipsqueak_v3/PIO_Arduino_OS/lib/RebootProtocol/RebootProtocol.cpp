#include "RebootProtocol.h"

// ReportRebootResponse //////////////////////////////////////////////////////////////////////////////

ReportRebootResponse::ReportRebootResponse(Hmac * hmac) : Response(hmac, "ReportRebootResponse")
{
}

volatile byte * ICACHE_RAM_ATTR ReportRebootResponse::getBuffer() {
  return _buffer;
}

byte * ReportRebootResponse::getPayload() {
  return _payload;
}

size_t ICACHE_RAM_ATTR ReportRebootResponse::getExpectedSize() {
  return REPORT_REBOOT_RESPONSE_SIZE;
}

uint8_t ReportRebootResponse::getExpectedProtocol() {
  return REPORT_REBOOT_PROTOCOL_ID;
}


// ReportRebootRequest ///////////////////////////////////////////////////////////////////////////////

ReportRebootRequest::ReportRebootRequest(uint32_t deviceID, Hmac * hmac)
  :
  Request(hmac, "ReportRebootRequest"),
  _size { REPORT_REBOOT_REQUEST_BASE_SIZE },
  _messageSize { 0 },
  _response(hmac)
{
  Request::initialize(_buffer, REPORT_REBOOT_REQUEST_MAX_SIZE, REPORT_REBOOT_PROTOCOL_ID, deviceID);
}

void ReportRebootRequest::reset() {
  Request::reset();
  localReset();
}

void ReportRebootRequest::reportNormalReboot() {
  const char * message = "n/a";
  setPayload(message, min(strlen(message) + 1, (size_t) REPORT_REBOOT_REQUEST_MESSAGE_SIZE_LIMIT));
}

void ReportRebootRequest::reportExceptionalReboot(const char * reason) {
  setPayload(reason, min(strlen(reason) + 1, (size_t) REPORT_REBOOT_REQUEST_MESSAGE_SIZE_LIMIT));
}

size_t ReportRebootRequest::getSize() {
  return _size;
}

ReportRebootResponse * ReportRebootRequest::getResponse() {
  return &_response;
}

byte * ReportRebootRequest::getBuffer() {
  return _buffer;
}

void ReportRebootRequest::localReset() {
  if (_messageSize > 0) {
    memset(&_buffer[REPORT_REBOOT_REQUEST_MESSAGE_SIZE_OFFSET], 0, 4);
    memset(&_buffer[REPORT_REBOOT_REQUEST_MESSAGE_OFFSET], 0, _messageSize);
  }
  _size = REPORT_REBOOT_REQUEST_BASE_SIZE;
  _messageSize = 0;
}

void ReportRebootRequest::setPayload(const char * message, size_t messageSize) {
  if (_messageSize > 0) localReset();
  Request::setPopulated();
  _messageSize = messageSize;
  _size = REPORT_REBOOT_REQUEST_BASE_SIZE + _messageSize;
  memcpy(&_buffer[REPORT_REBOOT_REQUEST_MESSAGE_SIZE_OFFSET], &_messageSize, 4);
  memcpy(&_buffer[REPORT_REBOOT_REQUEST_MESSAGE_OFFSET], message, _messageSize - 1);
  // Ensure null termination of the message
  _buffer[REPORT_REBOOT_REQUEST_MESSAGE_OFFSET + _messageSize - 1] = 0;
}
