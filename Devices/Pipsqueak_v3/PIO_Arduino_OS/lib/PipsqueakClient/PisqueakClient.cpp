#include "PipsqueakClient.h"
#include <ESP8266WiFi.h>
#include <TimeLib.h>

extern "C" {
  #include <user_interface.h>
}

PipsqueakClient::PipsqueakClient(PipsqueakState * pipsqueakState, Hmac * hmac)
:
  _timeRequest(pipsqueakState->getConfig()->getDeviceID(), hmac),
  _setpointRequest(pipsqueakState->getConfig()->getDeviceID(), hmac),
  _telemetryRequest(pipsqueakState->getConfig()->getDeviceID(), hmac),
  _reportRebootRequest(pipsqueakState->getConfig()->getDeviceID(), hmac),
  _requestQueueDepth { 0 },
  _requestQueueCursor { 0 },
  _wiFiConnectionEstablished { false },
  _wiFiReconnecting { false },
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
  _disconnected { false },
  _lastRequestAttemptTimestamp { 0 }
{
  _state = pipsqueakState;
}

void PipsqueakClient::setup() {
  // Register ISR callbacks
  _client.onConnect([](void * networkClient, AsyncClient * asyncClient) { ((PipsqueakClient *) networkClient)->onConnect(); }, this);
  _client.onData([](void * networkClient, AsyncClient * asyncClient, void * data, size_t len) { ((PipsqueakClient *) networkClient)->onData(data, len); }, this);
  _client.onError([](void * networkClient, AsyncClient * asyncClient, uint8_t error) { ((PipsqueakClient *) networkClient)->onError(error); }, this);
  _client.onTimeout([](void * networkClient, AsyncClient * asyncClient, uint32_t time) { ((PipsqueakClient *) networkClient)->onTimeout(time); }, this);
  _client.onDisconnect([](void * networkClient, AsyncClient * asyncClient) { ((PipsqueakClient *) networkClient)->onDisconnect(); }, this);

  // Always issue a TimeRequest first to establish clock sync
  enqueue(&_timeRequest);

  // Then issue a report reboot request and setpoint request
  prepareReportRebootRequest();
  enqueue(&_reportRebootRequest);
  _setpointRequest.setReboot();
  enqueue(&_setpointRequest);

  // Initiate WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(_state->getConfig()->getWifiSSID(), _state->getConfig()->getWifiPassword());
}

void PipsqueakClient::loop() {
  // Don't do anything until the WiFi connection is initially established
  if (!_wiFiConnectionEstablished || _wiFiReconnecting) {
    if (!WiFi.isConnected()) return;
    _wiFiReconnecting = false;
    _wiFiConnectionEstablished = true;
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): WiFi connected");
    #endif
  } else if (!WiFi.isConnected()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): WiFi connection lost; reconnecting");
    #endif
    // setAutoReconnect(true) has been unreliable - may be trying to use same channel?
    _wiFiReconnecting = true;
    _state->recordError(ErrorType::Pipsqueak, WIFI_CONNECTION_ERROR);
    WiFi.disconnect();
    delay(500);
    WiFi.begin(_state->getConfig()->getWifiSSID(), _state->getConfig()->getWifiPassword());
  }

  if (_transmitting && _response->isComplete()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.printf("PipsqueakClient.loop(): %s complete/ready\n", _response->getName());
    #endif
    endSession();
  }

  bool clockSyncIsRequired = false;

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
      Serial.printf("PipsqueakClient.loop(): transmission error: %d: %s\n", _errorCode, _client.errorToString(_errorCode));
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
      // invoke whether errors are present or not
      _state->recordErrors(_response);
      synchronizeClock();
      clockSyncIsRequired = clockSyncRequired();
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      if (_response->hasErrors()) {
        Serial.printf("PipsqueakClient.loop(): %s failed with %u errors\n", _request->getName(), _response->errorCount());
      } else {
        Serial.printf("PipsqueakClient.loop(): %s succeeded in %lu seconds\n", _request->getName(), _response->getElapsedTime());
      }
      #endif
      if (_response->hasErrors() && _request != &_timeRequest) {
        _request->failed();
        enqueue(_request);
      } else {
        _request->reset();
      }
      _response = NULL;
    }
    if (_request != NULL) {
      _request = NULL;
    }
    _busy = false;
  }

  if (_connected && !_transmitting) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.loop(): client connection established");
    #endif
    transmit();
  }

  yield();

  if (!_telemetryRequest.isInFlight()) {
    size_t count = 0;
    while (_telemetryRequest.isReadyForMoreEvents() && _state->hasStatusEvents()) {
      _telemetryRequest.addStatusEvent(_state->dequeueStatusEvent());
      count += 1;
    }
    if (count > 0) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.printf("PipsqueakClient.loop(): added %u status events to TelemetryRequest\n", count);
      Serial.println("PipsqueakClient.loop(): auto-enqueuing a TelemetryRequest for transmission");
      #endif
      enqueue(&_telemetryRequest);
      yield();
    }
  }

  if (_request == NULL) {
    if (clockSyncIsRequired) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.loop(): auto-staging a TimeRequest for transmission");
      #endif
      _timeRequest.reset();
      _request = &_timeRequest;
      _response = _request->getResponse();
    } else if (_requestQueueDepth > 0) {
      _request = _requestQueue[_requestQueueCursor];
      _response = _request->getResponse();
      _requestQueueCursor = (_requestQueueCursor + 1) % REQUEST_QUEUE_DEPTH;
      _requestQueueDepth -= 1;
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.printf("PipsqueakClient.loop(): staging a %s for transmission\n", _request->getName());
      #endif
    }
  }

  if (!_busy && _request != NULL && !isRateLimited()) {
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
  for (size_t i = _requestQueueCursor; i < _requestQueueDepth; i++) {
    if (_requestQueue[i] == request) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.printf("PipsqueakClient.enqueue(%s): rejected (already enqueued)\n", request->getName());
      #endif
      return false;
    }
  }
  _requestQueue[(_requestQueueCursor + _requestQueueDepth) % REQUEST_QUEUE_DEPTH] = request;
  _requestQueueDepth += 1;
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("PipsqueakClient.enqueue(%s): accepted\n", request->getName());
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
  _lastRequestAttemptTimestamp = millis();

  if (!WiFi.isConnected()) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.println("PipsqueakClient.connect(): WiFi is not connected.");
    #endif
    _response->addError(ErrorType::Pipsqueak, NETWORK_ERROR_WIFI_CONNECTION);
    _disconnected = true;
    return;
  }

  PipsqueakConfig * config = _state->getConfig();
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("PipsqueakClient.connect(): Initiating client connection attempt to %s:%u\n", config->getHostIP()->toString().c_str(), config->getHostPort());
  #endif
  _client.connect(*(config->getHostIP()), config->getHostPort());
}

void ICACHE_RAM_ATTR PipsqueakClient::onConnect() {
  _connecting = false;
  _connected = true;
}

void PipsqueakClient::transmit() {
  if (!_request->ready(now(), RANDOM_REG32)) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.printf("PipsqueakClient.transmit(): %s not populated or otherwise unready to transmit\n", _request->getName());
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

void PipsqueakClient::synchronizeClock() {
  if (_response == NULL) return;
  if (_response->hasErrors()) return;
  if (_response->getElapsedTime() > 1) {
    #ifdef DEBUG_PIPSQUEAK_CLIENT
    Serial.printf("PipsqueakClient.synchronizeClock(): %lu seconds elapsed\n", _response->getElapsedTime());
    #endif
    return;
  }
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("PipsqueakClient.synchronizeClock(): success @ %u\n", _response->getTimestamp());
  #endif
  setTime(_response->getTimestamp());
  _state->setClockSynchronized(true);
}

bool PipsqueakClient::clockSyncRequired() {
  if (_response == NULL) return false;
  if (_request == &_timeRequest && _response->hasErrors()) return true;

  for (size_t i = 0; i < _response->errorCount(); i++) {
    if (_response->getErrorType(i) != ErrorType::Pipsqueak) {
      continue;
    }

    // Either of these two errors indicate that the clocks are out of sync
    int8_t code = _response->getErrorCode(i);
    if (
      code == REQUEST_ERROR_CLOCK_SYNC_BEHIND ||
      code == REQUEST_ERROR_CLOCK_SYNC_AHEAD
    ) {
      #ifdef DEBUG_PIPSQUEAK_CLIENT
      Serial.println("PipsqueakClient.clockSyncRequired(): out of sync");
      #endif
      _state->setClockSynchronized(false);
      return true;
    }
  }

  // If neither clock sync errors are present, clocks are either sufficiently
  // in sync, or a problem bigger than clock sync (for example, network
  // connectivity) is preventing determination of synchronicity.
  return false;
}

void PipsqueakClient::prepareReportRebootRequest() {
  if (ESP.getResetInfoPtr()->reason == REASON_DEFAULT_RST) {
    _reportRebootRequest.reportNormalReboot();
    return;
  }
  memset(_rebootMessage, 0, REPORT_REBOOT_REQUEST_MESSAGE_SIZE_LIMIT);
  sprintf(
    _rebootMessage,
    "Fatal exception:%d flag:%d (%s) epc1:0x%08x epc2:0x%08x epc3:0x%08x excvaddr:0x%08x depc:0x%08x",
    ESP.getResetInfoPtr()->exccause,
    ESP.getResetInfoPtr()->reason,
    (
      ESP.getResetInfoPtr()->reason == 1 ? "WDT" :
      ESP.getResetInfoPtr()->reason == 2 ? "EXCEPTION" :
      ESP.getResetInfoPtr()->reason == 3 ? "SOFT_WDT" :
      ESP.getResetInfoPtr()->reason == 4 ? "SOFT_RESTART" :
      ESP.getResetInfoPtr()->reason == 5 ? "DEEP_SLEEP_AWAKE" :
      ESP.getResetInfoPtr()->reason == 6 ? "EXT_SYS_RST" :
      "UNKNOWN"
    ),
    ESP.getResetInfoPtr()->epc1,
    ESP.getResetInfoPtr()->epc2,
    ESP.getResetInfoPtr()->epc3,
    ESP.getResetInfoPtr()->excvaddr,
    ESP.getResetInfoPtr()->depc
  );
  #ifdef DEBUG_PIPSQUEAK_CLIENT
  Serial.printf("ReportExceptionRequest(): reset diagnostic = %s\n", _rebootMessage);
  #endif
  _reportRebootRequest.reportExceptionalReboot(_rebootMessage);
}

bool PipsqueakClient::isRateLimited() {
  // Rate limit imposed here one request per second
  return millis() - _lastRequestAttemptTimestamp < 1000;
}
