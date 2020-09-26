# Pipsqueak Protocol Library

Pipsqueak devices communicate with a server using the Pipsqueak Protocol, a custom
binary TCP/IP protocol designed for compact data exchange. The protocol defines
a generic structure for requests and responses and specific requests and responses
for various purposes.

## General Specification

Each server will be reachable via a static IPv4 address and port number. Each request-response pair is
transmitted in a single TCP/IP session initiated by a Pipsqueak device. A server cannot send requests
to Pipsqueaks, and Pipsqueaks cannot send responses to a server. Byte order is little endian.

### Requests

Requests consist of a 32-byte header, an optional arbitary-length data segment, and a 32-byte HMAC at the end.

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

Classes supplied in this implementation are designed for static memory allocation and entirely avoid
dynamic allocation. Each instance is expected to be re-used, and request-response cycles are expected
to be sequential rather than parallel.

TODO: document PipsqueakClient
