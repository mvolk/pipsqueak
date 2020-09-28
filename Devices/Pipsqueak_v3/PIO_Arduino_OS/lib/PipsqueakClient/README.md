# Pipsqueak Client Library

Pipsqueak devices communicate with a server using the Pipsqueak Protocol, a custom
binary TCP/IP protocol designed for compact data exchange. The protocol defines
a generic structure for requests and responses and specific requests and responses
for various purposes.

This library provides a client that leverages and exposes an ecosystem of libaries
that collectively implement the Pipsqueak Protocol. Encapsulated in this library are
the following component libraries:

* [Errors.h](./lib/Errors/README.md) - defines error types and codes
* [Request.h](./lib/Request/README.md) - defines the Request base class
* [Response.h](./lib/Response/README.md) - defines the Response base class
* [TimeProtocol.h](./lib/TimeProtocol/README.md) - defines the TimeRequest and
  TimeResponse classes
* [RebootProtocol.h](./lib/RebootProtocol/README.md) - defines the
  ReportRebootRequest and ReportRebootResponse classes
* [SetpointProtocol.h](./lib/SetpointProtocol/README.md) - defines the
  SetpointRequest and SetpointResponse classes
* [TelemetryProtocol.h](./lib/TelemetryProtocol/README.md) - defines the
  TelemetryRequest and TelemetryResponse classes

The [PipsqueakClient](./PipsqueakClient.h) class abstracts away all the complexity, and in coordination
with [PipsqueakState](../PipsqueakState/README.md), boils the work down to setup() and loop() calls.

## General Specification

Each server will be reachable via a static IPv4 address and port number. Each request-response pair is
transmitted in a single TCP/IP session initiated by a Pipsqueak device. A server cannot send requests
to Pipsqueaks, and Pipsqueaks cannot send responses to a server. Byte order is little endian.

### Requests

Requests consist of a 32-byte header segment followed by an optional arbitary-length data segment and
finally a 32-byte HMAC segment.

The header structure is as follows. Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | -------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | The Protocol ID, described below
| 1     | 4   | 4      | uint32 | The unique device ID assigned to each Pipsqueak hardware device
| 5     | 8   | 4      | uint32 | The timestamp (seconds since Jan 1 1970) when the message was sent
| 9     | 9   | 1      | ------ | Reserved for protocol ID-specific use
| 10    | 13  | 4      | uint32 | An arbitrary challenge value that should be different per request
| 14    | 31  | 18     | ------ | Reserved for protocol ID-specific use

Each specialized request type is assigned a unique protocol ID, each of which defines how the reserved
sections of the header are used, whether a data segment is used, and if it is, how its length is
specified. See each protocol ID's documentation for additional header structure detail.

| Protocol ID | Protocol
| ----------- | ----------------------------------------------------
| 1           | [Time Protocol](../TimeProtocol/README.md)
| 2           | [Setpoint Protocol](../SetpointProtocol/README.md)
| 3           | [Telemetry Protocol](../TelemetryProtocol/README.md)
| 4           | [Reboot Protocol](../RebootProtocol)

### Responses

Responses consist of a 32-byte header followed by a 32-byte HMAC.

The header structure is as follows. Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | ---------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | The Protocol ID, described below, matching the request's Protocol ID
| 1     | 4   | 4      | uint32 | The timestamp (seconds since Jan 1 1970) when the message was sent
| 5     | 8   | 4      | ------ | Reserved for protocol ID-specific use
| 9     | 1   | 1      | uint8  | Bitmasked status code
| 10    | 13  | 4      | uint32 | The challenge value from the corresponding request
| 14    | 31  | 18     | ------ | Reserved for protocol ID-specific use

Each specialized response type is assigned a unique protocol ID, each of which defines how the reserved
sections of the header are used. See each protocol ID's documentation for additional header structure
detail.

| Protocol ID | Protocol
| ----------- | ----------------------------------------------------
| 1           | [Time Protocol](../TimeProtocol/README.md)
| 2           | [Setpoint Protocol](../SetpointProtocol/README.md)
| 3           | [Telemetry Protocol](../TelemetryProtocol/README.md)
| 4           | [Reboot Protocol](../RebootProtocol)

## Usage

Instantiate one instance and call it's setup() and loop() methods as illustrated below. The client
class will take care of connecting to WiFi, synchronizing the system clock, requesting the setpoint,
reporting the reboot reason, sending telemetry requests to the server, and recording error status
events when things go wrong.

``` cpp
#include <Arduino.h>
#include <PipsqueakState.h>
#include <PipsqueakClient.h>
#include <Hmac.h>

PipsqueakState * state;
PipsqueakClient * client;

void setup() {
  state = new PipsqueakState();
  state->setup();

  Hmac * hmac = new Hmac(state.getConfig()->getSecretKey());

  client = new PipsqueakClient(state, hmac);
  client->setup();
}

void loop() {
  state->loop();
  client->loop();
}
```

### Clock Synchronization

It's crucial that the device's clock remain reasonably synchronized (with a second or
two) of the server's clock. This ensures that event timestamps are comparable across
devices and device restarts. Further, anti-replay security features of the protocol
depend on approximate synchronization of the clocks. Drift of as little as three
seconds can produce communication failures. The protocol is particularly unforgiving
when the device's clock is faster than the server's clock.

To ensure that the device's clock remains synchronized with the server, each response
is timestamped by the server and the elapsed request-response time is tracked by this
implementation. Each response, of any derived type, should be inspected and used to
update that clock if the response is valid and the elapsed request-response time is
not too high.

If the device's timestamp is not synchronized with the server, only a TimeRequest
can be used to re-establish synchronization. All other requests will be rejected based
on

Fortunately for you, the PipsqueakClient abstracts all of this away, ensuring that
TimeRequests are issued ahead of all other requests in the queue when necessary
and keeping the system clock in sync.

It's worth understanding that slow network conditions, especially high latency, and
poor server performance can result in complete failure of the protocol. A
request-response cycle that takes more than 1 second will big treated as an
unreliable source of clock sync data, and requests that take more than 3 seconds
to be received and processed by the server will be rejected with errors indicating
clock synchronization problems. If a device is frequently is repeatedly making
TimeRequests, take a close look at errors encoded in the responses and the elapsed
request-response time.
