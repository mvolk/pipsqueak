#include "TimeProtocol.h"
#include <Errors.h>


// TimeResponse ////////////////////////////////////////////////////////////////////////////////////

TimeResponse::TimeResponse(Hmac * hmac) : Response(hmac, "TimeResponse")
{
}

volatile byte * ICACHE_RAM_ATTR TimeResponse::getBuffer() {
  return _buffer;
}

byte * TimeResponse::getPayload() {
  return _payload;
}

size_t ICACHE_RAM_ATTR TimeResponse::getExpectedSize() {
  return TIME_RESPONSE_SIZE;
}

uint8_t TimeResponse::getExpectedProtocol() {
  return TIME_PROTOCOL_ID;
}


// TimeRequest ////////////////////////////////////////////////////////////////////////////////////

TimeRequest::TimeRequest(uint32_t deviceID, Hmac * hmac)
:
  Request(hmac, "TimeRequest"),
  _response(hmac)
{
  Request::initialize(_buffer, TIME_REQUEST_SIZE, TIME_PROTOCOL_ID, deviceID);
  // There is nothing to populate - this request is "populated" when empty
  setPopulated();
}

void TimeRequest::reset() {
  Request::reset();
  setPopulated();
}

size_t TimeRequest::getSize() {
  return TIME_REQUEST_SIZE;
}

TimeResponse * TimeRequest::getResponse() {
  return &_response;
}

byte * TimeRequest::getBuffer() {
  return _buffer;
}
