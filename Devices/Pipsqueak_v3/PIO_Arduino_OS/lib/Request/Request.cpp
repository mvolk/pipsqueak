#include "Request.h"

Request::Request(Hmac * hmac, const char * name) : _populated { false }, _inFlight { false }
{
  _name = name;
  _hmac = hmac;
}

bool Request::isPopulated() {
  return _populated;
}

bool Request::ready(time_t now, uint32_t challenge) {
  if (!_populated || _inFlight) return false;
  _inFlight = true;
  getResponse()->reset();
  getResponse()->setChallenge(challenge);
  memcpy((void *) &getBuffer()[REQUEST_CHALLENGE_OFFSET], &challenge, 4);
  memcpy((void *) &getBuffer()[REQUEST_TIMESTAMP_OFFSET], &now, 4);
  setHmac();
  return true;
}

bool Request::isInFlight() {
  return _inFlight;
}

void Request::failed() {
  reset(_populated);
}

void Request::reset() {
  reset(false);
}

time_t Request::getTimestamp() {
  time_t timestamp;
  memcpy(&timestamp, (void *) &getBuffer()[REQUEST_TIMESTAMP_OFFSET], 4);
  return timestamp;
}

const char * Request::getName() {
  return _name;
}

void Request::initialize(byte * buffer, size_t bufferSize, uint8_t protocolID, uint32_t deviceID) {
  memset(buffer, 0, bufferSize);
  buffer[REQUEST_PROTOCOL_ID_OFFSET] = protocolID;
  memcpy(&buffer[REQUEST_DEVICE_ID_OFFSET], &deviceID, 4);
}

void Request::setPopulated() {
  _populated = true;
}

size_t Request::getHmacOffset() {
  return getSize() - HMAC_SIZE;
}

void Request::setHmac() {
  byte * buffer = getBuffer();
  size_t hmacOffset = getHmacOffset();
  _hmac->generate(buffer, hmacOffset, &buffer[hmacOffset]);
}

void Request::reset(bool populated) {
  byte * buffer = getBuffer();
  memset((void *) &buffer[REQUEST_TIMESTAMP_OFFSET], 0, 4);
  memset((void *) &buffer[REQUEST_CHALLENGE_OFFSET], 0, 4);
  memset((void *) &buffer[getHmacOffset()], 0, HMAC_SIZE);
  getResponse()->reset();
  _inFlight = false;
  _populated = populated;
}
