#ifndef PipsqueakClient_h
#define PipsqueakClient_h

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#include <Errors.h>
#include <Request.h>
#include <Response.h>
#include <TimeProtocol.h>
#include <SetpointProtocol.h>
#include <TelemetryProtocol.h>
#include <RebootProtocol.h>

#define REQUEST_QUEUE_DEPTH 10

// Un-comment to enable extensive debug statements via Serial
// #define DEBUG_PIPSQUEAK_CLIENT

/**
 * The network client class used to send and receive
 * requests and responses, respectively.
 */
class PipsqueakClient {
  public:
    PipsqueakClient(IPAddress * host, uint16_t _port, uint32_t deviceID, Hmac * hmac);

    /**
     * Invoke once in the Arduino's setup() function.
     */
    void setup();

    /**
     * Invoke one or more times in the Arduino loop() function.
     */
    void loop();

    /**
     * Returns a pointer to the TimeRequest singleton held by
     * the client. Also used to access the response via
     * Request::getResponse().
     *
     * Note that the PipsqueakClient has an internal clock
     * synchronization mechanism that automatically issues
     * time requests as needed. Application code should not
     * need access to this request and need not enqueue these
     * requests.
     */
    TimeRequest * getTimeRequest();

    /**
     * Returns a pointer to the SetpointRequest singleton held by
     * the client. Also used to access the response via
     * Request::getResponse().
     *
     * Optionally invoke setReboot() on this request before enqueueing
     * it for transmission.
     *
     * Note that this request can be replaced with a
     * ReportRebootRequest coupled with a TelemetryRequest. The
     * former provides reboot data to the server, while the latter
     * obtains a setpoint from the server.
     */
    SetpointRequest * getSetpointRequest();

    /**
     * Returns a pointer to the TelemetryRequest singleton held by
     * the client. Also used to access the response via
     * Request::getResponse().
     *
     * Note that this request cannot be sent "empty" - e.g. without
     * any events added to it.
     */
    TelemetryRequest * getTelemetryRequest();

    /**
     * Returns a pointer to the ReportRebootRequest singleton held by
     * the client. Also used to access the response via
     * Request::getResponse().
     *
     * Invoke either reportNormalReboot() or
     * reportExceptionalReboot(const char *) before enqueing this
     * request.
     */
    ReportRebootRequest * getReportRebootRequest();

    /**
     * Enqueues a request to be transmitted. The queue depth
     * is limited.
     *
     * Returns true if enqueued, false if the queue is full.
     */
    bool enqueue(Request * request);

  private:
    TimeRequest _timeRequest;
    SetpointRequest _setpointRequest;
    TelemetryRequest _telemetryRequest;
    ReportRebootRequest _reportRebootRequest;
    Request * _requestQueue[REQUEST_QUEUE_DEPTH];
    size_t _requestQueueDepth;
    size_t _requestQueueCursor;
    IPAddress _host;
    uint16_t _port;
    bool _wiFiConnectionEstablished;
    AsyncClient _client;
    Request * _request;
    Response * _response;
    volatile bool _busy;
    volatile bool _connecting;
    volatile bool _connected;
    volatile bool _transmitting;
    volatile bool _errorDetected;
    volatile int8_t _errorCode;
    volatile bool _timeoutDetected;
    volatile bool _disconnecting;
    volatile bool _disconnected;

    void connect();
    void onConnect();
    void transmit();
    void onData(void * data, size_t len);
    void onError(uint8_t error);
    void onTimeout(uint32_t time);
    void onDisconnect();
    void endSession();
    void synchronizeClock();
    bool clockSyncRequired();
};

#endif // PipsqueakClient_h