#include "Response.h"
#include <Hmac.h>
#include <Errors.h>

Response::Response(Hmac * hmac, const char * name)
  :
  _challenge { 0 },
  _bytesReceived { 0 },
  _errorCount { 0 },
  _ready { false },
  _inUse { false },
  _elapsedTime { 0 }
{
  _hmac = hmac;
  _name = name;
}

void Response::reset() {
  memset(_errors, 0, ERROR_COUNT_LIMIT * 2);
  memset(getPayload(), 0, getExpectedSize());
  _errorCount = 0;
  _challenge = 0;
  _ready = false;
  _bytesReceived = 0;
  _inUse = false;
  _elapsedTime = 0;
}

void ICACHE_RAM_ATTR Response::receiveBytes(void * data, size_t len) {
  // ignore incoming bytes of the response isn't in use
  if (!isInUse()) return;
  // memcpy is not compatible with volatile memory
  size_t bytesRead = 0;
  for (size_t i = 0; i < len; i++) {
    if (_bytesReceived >= getExpectedSize()) {
      addError(ErrorType::Pipsqueak, RESPONSE_ERROR_EXCESS_DATA);
      break;
    }
    getBuffer()[_bytesReceived] = ((byte *) data)[i];
    bytesRead++;
    _bytesReceived++;
  }
  if (bytesRead < len) {
    _bytesReceived += len - bytesRead;
  }
}

bool Response::isComplete() {
  return _bytesReceived >= getExpectedSize() || _errorCount > 0;
}

void Response::ready(time_t elapsedTime) {
  size_t size = _bytesReceived;
  volatile byte * bufferPtr = getBuffer();
  byte * payloadPtr = getPayload();
  if (_bytesReceived == getExpectedSize()) {
    for (size_t i = 0; i < size; i++) {
      payloadPtr[i] = bufferPtr[i];
    }
    inspectProtocol() && inspectHmac() && inspectChallenge() && inspectStatusCode();
  } else if (size > getExpectedSize()) {
    addError(ErrorType::Pipsqueak, RESPONSE_ERROR_EXCESS_DATA);
  } else if (size < getExpectedSize()) {
    addError(ErrorType::Pipsqueak, RESPONSE_ERROR_TRUNCATED_RESPONSE);
  }
  _elapsedTime = elapsedTime;
  _ready = true;
}

bool Response::isReady() {
  return _ready;
}

bool ICACHE_RAM_ATTR Response::isInUse() {
  return _inUse;
}

void Response::addError(ErrorType errorType, int8_t errorCode) {
  // Ignore excess errors
  if (_errorCount >= ERROR_COUNT_LIMIT) return;
  // Don't add an error that is already present
  for (size_t i = 0; i < _errorCount; i++) {
    if (_errors[i * 2] == errorType && _errors[(i * 2) + 1] == errorCode) return;
  }
  // Add the new error
  _errors[_errorCount * 2] = errorType;
  _errors[(_errorCount * 2) + 1] = errorCode;
  _errorCount += 1;
}

bool Response::hasErrors() {
  return _errorCount > 0;
}

size_t Response::errorCount() {
  return _errorCount;
}

ErrorType Response::getErrorType(size_t index) {
  if (index < 0 || index >= _errorCount) {
    return ErrorType::None;
  }
  return (ErrorType) _errors[index * 2];
}

int8_t Response::getErrorCode(size_t index) {
  if (index < 0 || index >= _errorCount) {
    return ERROR_NONE;
  }
  return _errors[(index * 2) + 1];
}

uint32_t Response::getTimestamp() {
  uint32_t timestamp;
  memcpy(&timestamp, (void *) &getPayload()[RESPONSE_TIMESTAMP_OFFSET], 4);
  return timestamp;
}

time_t Response::getElapsedTime() {
  return _elapsedTime;
}

void Response::setChallenge(uint32_t challenge) {
  _challenge = challenge;
  _inUse = true;
}

const char * Response::getName() {
  return _name;
}

uint8_t Response::getProtocol() {
  return (uint8_t) getPayload()[RESPONSE_PROTOCOL_ID_OFFSET];
}

uint8_t Response::getStatusCode() {
  return (uint8_t) getPayload()[RESPONSE_STATUS_CODE_OFFSET];
}

uint32_t Response::getChallengeResponse() {
  uint32_t challenge;
  memcpy(&challenge, (void *) &getPayload()[RESPONSE_CHALLENGE_OFFSET], 4);
  return challenge;
}

size_t Response::getHmacOffset() {
  return getExpectedSize() - HMAC_SIZE;
}

bool Response::inspectProtocol() {
  bool valid = true;  if (getProtocol() != getExpectedProtocol()) {
    addError(ErrorType::Pipsqueak, RESPONSE_ERROR_INVALID_PROTOCOL);
    valid = false;
  }
  return valid;
}

bool Response::inspectHmac() {
  bool valid = true;
  size_t hmacOffset = getHmacOffset();
  if (!_hmac->validate(getPayload(), hmacOffset, &(getPayload()[hmacOffset]))) {
    addError(ErrorType::Pipsqueak, RESPONSE_ERROR_AUTHENTICATION);
    valid = false;
  }
  return valid;
}

bool Response::inspectChallenge() {
  bool valid = true;
  if (getChallengeResponse() != _challenge) {
    addError(ErrorType::Pipsqueak, RESPONSE_ERROR_CHALLENGE_FAILED);
    valid = false;
  }
  return valid;
}

bool Response::inspectStatusCode() {
  uint8_t statusCode = getStatusCode();
  if (statusCode == 0) return true;
  if ((statusCode & STATUS_MASK_CLOCK_SYNC_BEHIND) == STATUS_MASK_CLOCK_SYNC_BEHIND) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_CLOCK_SYNC_BEHIND);
  }
  if ((statusCode & STATUS_MASK_CLOCK_SYNC_AHEAD) == STATUS_MASK_CLOCK_SYNC_AHEAD) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_CLOCK_SYNC_AHEAD);
  }
  if ((statusCode & STATUS_MASK_AUTHENTICATION) == STATUS_MASK_AUTHENTICATION) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_AUTHENTICATION);
  }
  if ((statusCode & STATUS_MASK_RATE_LIMITED) == STATUS_MASK_RATE_LIMITED) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_RATE_LIMITED);
  }
  if ((statusCode & STATUS_MASK_EXCESS_DATA) == STATUS_MASK_EXCESS_DATA) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_EXCESS_DATA);
  }
  if ((statusCode & STATUS_MASK_DEVICE_NOT_REGISTERED) == STATUS_MASK_DEVICE_NOT_REGISTERED) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_DEVICE_NOT_REGISTERED);
  }
  if ((statusCode & STATUS_MASK_CONCURRENT_REQUESTS) == STATUS_MASK_CONCURRENT_REQUESTS) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_CONCURRENT_REQUESTS);
  }
  if ((statusCode & STATUS_MASK_UNKNOWN_EVENT_TYPE) == STATUS_MASK_UNKNOWN_EVENT_TYPE) {
    addError(ErrorType::Pipsqueak, REQUEST_ERROR_UNKNOWN_EVENT_TYPE);
  }
  return false;
}
