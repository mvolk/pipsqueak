#include "TelemetryProtocol.h"
#include <Errors.h>

// StatusEvent ///////////////////////////////////////////////////////////////////////////////////

StatusEvent::StatusEvent() {
  memset(_payload, 0, STATUS_EVENT_SIZE);
}

void StatusEvent::temperatureObservation(uint32_t timestamp, float temperature) {
  reset();
  _payload[STATUS_EVENT_TYPE_OFFSET] = STATUS_EVENT_TYPE_TEMPERATURE;
  memcpy(&_payload[STATUS_EVENT_TIMESTAMP_OFFSET], &timestamp, 4);
  memcpy(&_payload[STATUS_EVENT_TEMPERATURE_OFFSET], &temperature, 4);
}

void StatusEvent::temperatureSetpoint(uint32_t timestamp, float temperature) {
  reset();
  _payload[STATUS_EVENT_TYPE_OFFSET] = STATUS_EVENT_TYPE_SETPOINT;
  memcpy(&_payload[STATUS_EVENT_TIMESTAMP_OFFSET], &timestamp, 4);
  memcpy(&_payload[STATUS_EVENT_SETPOINT_OFFSET], &temperature, 4);
}

void StatusEvent::error(
  uint32_t timestamp,
  ErrorType errorType,
  int8_t errorCode
) {
  reset();
  _payload[STATUS_EVENT_TYPE_OFFSET] = STATUS_EVENT_TYPE_ERROR;
  memcpy(&_payload[STATUS_EVENT_TIMESTAMP_OFFSET], &timestamp, 4);
  _payload[STATUS_EVENT_ERROR_TYPE_OFFSET] = errorType;
  _payload[STATUS_EVENT_ERROR_CODE_OFFSET] = errorCode;\
}

void StatusEvent::heaterPulse(
  uint32_t timestamp,
  uint32_t pulseDuration,
  uint8_t percentPower,
  uint32_t recoveryDuration
) {
  reset();
  _payload[STATUS_EVENT_TYPE_OFFSET] = STATUS_EVENT_TYPE_HEATER;
  memcpy(&_payload[STATUS_EVENT_TIMESTAMP_OFFSET], &timestamp, 4);
  memcpy(&_payload[STATUS_EVENT_PULSE_DURATION_OFFSET], &pulseDuration, 4);
  _payload[STATUS_EVENT_PERCENT_POWER_OFFSET] = percentPower;
  memcpy(&_payload[STATUS_EVENT_RECOVERY_DURATION_OFFSET], &recoveryDuration, 4);\
}

void StatusEvent::chillerPulse(
  uint32_t timestamp,
  uint32_t pulseDuration,
  uint32_t recoveryDuration
) {
  reset();
  _payload[STATUS_EVENT_TYPE_OFFSET] = STATUS_EVENT_TYPE_CHILLER;
  memcpy(&_payload[STATUS_EVENT_TIMESTAMP_OFFSET], &timestamp, 4);
  memcpy(&_payload[STATUS_EVENT_PULSE_DURATION_OFFSET], &pulseDuration, 4);
  memcpy(&_payload[STATUS_EVENT_RECOVERY_DURATION_OFFSET], &recoveryDuration, 4);
}

void StatusEvent::write(byte * buffer) {
  memcpy(buffer, _payload, STATUS_EVENT_SIZE);
  reset();
}

void StatusEvent::read(const byte * buffer) {
  reset();
  memcpy(_payload, buffer, STATUS_EVENT_SIZE);
}

void StatusEvent::reset() {
  memset(_payload, 0, STATUS_EVENT_SIZE);
}


// TelemetryResponse /////////////////////////////////////////////////////////////////////

TelemetryResponse::TelemetryResponse(Hmac * hmac) : Response(hmac, "TelemetryResponse")
{
}

float TelemetryResponse::getSetpoint() {
  float setpoint;
  memcpy(&setpoint, &_payload[TELEMETRY_RESPONSE_SETPOINT_OFFSET], 4);
  return setpoint;
}

volatile byte * ICACHE_RAM_ATTR TelemetryResponse::getBuffer() {
  return _buffer;
}

byte * TelemetryResponse::getPayload() {
  return _payload;
}

size_t ICACHE_RAM_ATTR TelemetryResponse::getExpectedSize() {
  return TELEMETRY_RESPONSE_SIZE;
}

uint8_t TelemetryResponse::getExpectedProtocol() {
  return TELEMETRY_PROTOCOL_ID;
}


// TelemetryRequest //////////////////////////////////////////////////////////////////////

TelemetryRequest::TelemetryRequest(uint32_t deviceID, Hmac * hmac)
  :
  Request(hmac, "TelemetryRequest"),
  _eventCount { 0 },
  _response(hmac)
{
  Request::initialize(_buffer, TELEMETRY_REQUEST_MAX_SIZE, TELEMETRY_PROTOCOL_ID, deviceID);
}

void TelemetryRequest::reset() {
  Request::reset();
  memset(&_buffer[TELEMETRY_REQUEST_EVENTS_BUFFER_OFFSET], 0, _eventCount * TELEMETRY_REQUEST_EVENT_SIZE);
  // don't reset event count until superclass reset() is invoked to ensure correct size reporting for hmac reset
  _eventCount = 0;
  _buffer[TELEMETRY_REQUEST_COUNT_OFFSET] = _eventCount;
}

bool TelemetryRequest::addStatusEvent(StatusEvent * statusEvent) {
  if (!isReadyForMoreEvents()) return false;
  if (_eventCount == 0) Request::setPopulated();
  size_t offset = TELEMETRY_REQUEST_EVENTS_BUFFER_OFFSET + (_eventCount * TELEMETRY_REQUEST_EVENT_SIZE);
  #ifdef DEBUG_TELEMETRY_PROTOCOL
  Serial.printf("TelemetryRequest.addStatusEvent(...) at offset %u\n", offset);
  #endif
  statusEvent->write(&_buffer[offset]);
  _eventCount += 1;
  _buffer[TELEMETRY_REQUEST_COUNT_OFFSET] = _eventCount;
  return true;
}

bool TelemetryRequest::isReadyForMoreEvents() {
  if (_eventCount == TELEMETRY_REQUEST_EVENT_COUNT_LIMIT) return false;
  if (isInFlight()) return false;
  return true;
}

size_t TelemetryRequest::getSize() {
  return TELEMETRY_REQUEST_BASE_SIZE + (_eventCount * TELEMETRY_REQUEST_EVENT_SIZE);
}

TelemetryResponse * TelemetryRequest::getResponse() {
  return &_response;
}

byte * TelemetryRequest::getBuffer() {
  return _buffer;
}
