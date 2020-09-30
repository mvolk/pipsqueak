#include "SetpointProtocol.h"
#include <Errors.h>

// SetpointResponse //////////////////////////////////////////////////////////////////////////////

SetpointResponse::SetpointResponse(Hmac * hmac) : Response(hmac, "SetpointResponse")
{
}

float SetpointResponse::getSetpoint() {
  float setpoint;
  memcpy(&setpoint, &_payload[SETPOINT_RESPONSE_SETPOINT_OFFSET], 4);
  return setpoint;
}

volatile byte * ICACHE_RAM_ATTR SetpointResponse::getBuffer() {
  return _buffer;
}

byte * SetpointResponse::getPayload() {
  return _payload;
}

size_t ICACHE_RAM_ATTR SetpointResponse::getExpectedSize() {
  return SETPOINT_REQUEST_SIZE;
}

uint8_t SetpointResponse::getExpectedProtocol() {
  return SETPOINT_PROTOCOL_ID;
}


// SetpointRequest ///////////////////////////////////////////////////////////////////////////////

SetpointRequest::SetpointRequest(uint32_t deviceID, Hmac * hmac)
:
  Request(hmac, "SetpointRequest"),
  _response(hmac)
{
  Request::initialize(_buffer, SETPOINT_REQUEST_SIZE, SETPOINT_PROTOCOL_ID, deviceID);
  // This request has only an optional payload
  setPopulated();
}

void SetpointRequest::reset() {
  Request::reset();
  _buffer[SETPOINT_REQUEST_REBOOT_FLAG_OFFSET] = 0x00;
  setPopulated();
}

void SetpointRequest::setReboot() {
  Request::setPopulated();
  _buffer[SETPOINT_REQUEST_REBOOT_FLAG_OFFSET] = 0x01;
}

size_t SetpointRequest::getSize() {
  return SETPOINT_REQUEST_SIZE;
}

SetpointResponse * SetpointRequest::getResponse() {
  return &_response;
}

byte * SetpointRequest::getBuffer() {
  return _buffer;
}
