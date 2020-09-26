#include "PipsqueakClient.h"
#include <ESP8266WiFi.h>
#include <TimeLib.h>

PipsqueakClient::PipsqueakClient(IPAddress * host, uint16_t port, uint32_t deviceID, Hmac * hmac)
:
  _timeRequest(deviceID, hmac),
  _setpointRequest(deviceID, hmac),
  _telemetryRequest(deviceID, hmac),
  _reportRebootRequest(deviceID, hmac),
  _requestQueueDepth { 0 },
  _requestQueueCursor { 0 },
  _client(),
  _request { NULL },
  _response { NULL },
  _busy { false },
  _connecting { false },
  _connected { false },
  _transmitting { false },
  _errorDetected { false },
  _errorCode { 0 },
  _timeoutDetected { false },
  _disconnecting { false },
  _disconnected { false }
{
  _host = *host;
  _port = port;
}

void PipsqueakClient::setup() {
  _client.onConnect([](void * networkClient, AsyncClient * asyncClient) { ((PipsqueakClient *) networkClient)->onConnect(); }, this);
  _client.onData([](void * networkClient, AsyncClient * asyncClient, void * data, size_t len) { ((PipsqueakClient *) networkClient)->onData(data, len); }, this);
  _client.onError([](void * networkClient, AsyncClient * asyncClient, uint8_t error) { ((PipsqueakClient *) networkClient)->onError(error); }, this);
  _client.onTimeout([](void * networkClient, AsyncClient * asyncClient, uint32_t time) { ((PipsqueakClient *) networkClient)->onTimeout(time); }, this);
  _client.onDisconnect([](void * networkClient, AsyncClient * asyncClient) { ((PipsqueakClient *) networkClient)->onDisconnect(); }, this);
}

void PipsqueakClient::loop() {
  if (_transmitting && _response->isComplete()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): response complete/ready");
    #endif
    endSession();
  }

  if (_disconnected) {
    if (_disconnecting) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): client connection terminated normally");
      #endif
      _connected = false;
      _disconnecting = false;
    }

    if (_timeoutDetected) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): timeout");
      #endif
      _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_TIMEOUT);
    }

    if (_errorDetected) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.printf("PipsqueakClient.loop(): transmission error: %d: %s\n", _errorCode, _client->errorToString(_errorCode));
      #endif
      _response->addError(ErrorType::TcpStack, _errorCode);
    }

    if (_transmitting) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): disconnected while transmitting.");
      #endif
      _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_BROKEN_PIPE);
      _transmitting = false;
      _connected = false;
    }

    if (_connected) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): disconnected after connecting but before transmitting.");
      #endif
      _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_CONNECTION_LOST);
      _connected = false;
    }

    if (_connecting) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): disconnected while connecting.");
      #endif
      _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_CONNECTION_FAILED);
      _connecting = false;
      _client.close(true);
    }

    _disconnected = false;
    if (_response != NULL) {
      _response->ready(now() - _request->getTimestamp());
      _response = NULL;
    }
    if (_request != NULL) {
      _request = NULL;
    }
    _busy = false;
  }

  yield();

  if (_connected && !_transmitting) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): client connection established");
    #endif
    transmit();
  }

  if (_request == NULL && _requestQueueDepth > 0) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): staging the next request for transmission");
    #endif
    _request = _requestQueue[_requestQueueCursor];
    _response = _request->getResponse();
    _requestQueueCursor = (_requestQueueCursor + 1) % REQUEST_QUEUE_DEPTH;
    _requestQueueDepth -= 1;
  }

  if (!_busy && _request != NULL) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): connect()");
    #endif
    connect();
  }
}

TimeRequest * PipsqueakClient::getTimeRequest() {
  return &_timeRequest;
}

SetpointRequest * PipsqueakClient::getSetpointRequest() {
  return &_setpointRequest;
}

TelemetryRequest * PipsqueakClient::getTelemetryRequest() {
  return &_telemetryRequest;
}

ReportRebootRequest * PipsqueakClient::getReportRebootRequest() {
  return &_reportRebootRequest;
}

bool PipsqueakClient::enqueue(Request * request) {
  if (_requestQueueDepth == REQUEST_QUEUE_DEPTH) return false;
  _requestQueue[(_requestQueueCursor + _requestQueueDepth) % REQUEST_QUEUE_DEPTH] = request;
  _requestQueueDepth += 1;
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.println("PipsqueakClient.enqueue(Request *): accepted");
  #endif
  return true;
}

void PipsqueakClient::connect() {
  _busy = true;
  _connecting = true;
  _connected = false;
  _transmitting = false;
  _errorDetected = false;
  _errorCode = 0;
  _timeoutDetected = false;
  _disconnecting = false;
  _disconnected = false;

  if (!WiFi.isConnected()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.connect(): WiFi is not connected.");
    #endif
    _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_WIFI_CONNECTION);
    _disconnected = true;
    return;
  }

  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("PipsqueakClient.connect(): Initiating client connection attempt to %s:%u\n", _host.toString().c_str(), _port);
  #endif
  _client.connect(_host, _port);
}

void ICACHE_RAM_ATTR PipsqueakClient::onConnect() {
  _connecting = false;
  _connected = true;
}

void PipsqueakClient::transmit() {
  if (!_request->ready(now(), RANDOM_REG32)) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.transmit(): request not populated or otherwise unready to transmit");
    #endif
    _response->reset();
    _response->addError(ErrorType::Pipsqueak, REQUEST_NOT_POPULATED);
    endSession();
    return;
  }

  if (!_client.canSend()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.transmit(): async client cannot send");
    #endif
    _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_CLIENT_STATE);
    endSession();
    return;
  }

  if (_client.space() < _request->getSize()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.transmit(): async client does not have space for the entire message");
    #endif
    _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_BUFFER_FULL);
    endSession();
    return;
  }

  _transmitting = true;
  _client.write((const char *) _request->getBuffer(), _request->getSize());

  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("PipsqueakClient.transmit(): Wrote %d bytes to the async client\n", _request->getSize());
  #endif
}

void ICACHE_RAM_ATTR PipsqueakClient::onData(void * data, size_t len) {
  _response->receiveBytes(data, len);
  _client.ack(len);
};

void ICACHE_RAM_ATTR PipsqueakClient::onError(uint8_t error) {
  _errorDetected = true;
  _errorCode = error;
}

void ICACHE_RAM_ATTR PipsqueakClient::onTimeout(uint32_t time) {
  _timeoutDetected = true;
}

void ICACHE_RAM_ATTR PipsqueakClient::onDisconnect() {
  _disconnected = true;
}

void PipsqueakClient::endSession() {
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.println("PipsqueakClient.endSession()");
  #endif
 _transmitting = false;
  _disconnecting = true;
  _client.close(true);
}
