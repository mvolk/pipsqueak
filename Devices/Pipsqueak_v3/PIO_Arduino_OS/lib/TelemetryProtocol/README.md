# Telemetry Protocol Library

This library is a subcomponent of the [PipsqueakClient library](../PipsqueakClient/README.md),
which collectively implements the Pipsqueak Protocol.

## Useage

The Pipsqueak Telemetry Protocol is used for the following purposes:

1. Transmission of temperature observations to the server on a regular cadence.
2. Transmission of heater and chiller cycle events to the server.
3. Transmission of error codes to the server.
4. Transmission of setpoint change events.
5. Retreiving updated temperature setpoints from the server.
5. Keeping the device's clock in sync with the server's clock.

In general, observations should not be recorded until a successful [Time](../TimeProtocol/README.md)
response is received and the device's clock is synced.

Refer to the [PipsqueakClient library](../PipsqueakClient/README.md) for general Pipsqueak
request/response guidance.

## Request Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 0          | 0          | 1      | uint8       | [Standard Header Field] The Protocol ID
| 1          | 4          | 4      | uint32      | [Standard Header Field] The unique device ID assigned to each Pipsqueak hardware device
| 5          | 8          | 4      | uint32      | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 9          | 9          | 1      | uint8t      | The number of events, "n"
| 10         | 13         | 4      | uint32      | [Standard Header Field] An arbitrary challenge value that should be different per request
| 14         | 31         | 18     | ------      | Reserved
| 32+16(x-1) | 32+(16x-1) | 16     | StatusEvent | Status events, where x ranges from 1 to n inclusive and n is the number of events
| 32+16n     | 63+16n     | 32     | byte[]      | [Standard Field] HMAC

StatusEvent specifications vary by type, but each starts with:

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 0          | 0          | 1      | uint8       | Event type
| 1          | 4          | 4      | uint32      | Event timestamp

Event types are:

| Event Type | Description
| ---------- | ----------------------------
| 1          | Temperature observation
| 2          | New Setpoint
| 3          | deprecated
| 4          | Heater cycle event
| 5          | deprecated
| 6          | Error event
| 7          | Chiller cycle event

### Temperature Observation

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 5          | 8          | 4      | float       | Observed temperature, in Celsius
| 9          | 15         | 7      | -------     | Reserved

### Setpoint Change

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 5          | 8          | 4      | float       | New setpoint, in Celsius
| 9          | 15         | 7      | -------     | Reserved

### Heater Cycle Event

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 5          | 8          | 4      | float       | How long the heater was/will be on, in seconds
| 9          | 9          | 1      | uint8       | Heater power (0-100%)
| 10         | 13         | 4      | uint8       | Minimum time the heater will remain off after the cycle, in second (e.g. recovery duration)
| 14         | 15         | 2      | -------     | Reserved

### Error Event

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 5          | 5          | 1      | uint8       | Event type (0 = None, 1 = Pipsqueak, 2 = TCP/IP Stack)
| 6          | 6          | 1      | int8        | Error code
| 7          | 15         | 9      | -------     | Reserved

Error codes are defined in [Errors.h](../Errors/Errors.h).

### Chiller Cycle Event

| Start      | End        | Length | Type        | Content
| ---------- | ---------- | ------ | ----------- | -------------------------------------------------------------------------------------------
| 5          | 8          | 4      | float       | How long the chiller was/will be on, in seconds
| 9          | 9          | 1      | -------     | Reserved
| 10         | 13         | 4      | uint8       | Minimum time the chiller will remain off after the cycle, in second (e.g. recovery duration)
| 14         | 15         | 2      | -------     | Reserved

## Response Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | ---------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | [Standard Header Field] The Protocol ID
| 1     | 4   | 4      | uint32 | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 5     | 8   | 4      | float  | The desired temperature control setpoint
| 9     | 1   | 1      | uint8  | [Standard Header Field] Bitmasked status code
| 10    | 13  | 4      | uint32 | [Standard Header Field] The challenge value from the corresponding request
| 14    | 31  | 18     | ------ | Reserved
| 32    | 63  | 32     | byte[] | [Standard Field] HMAC

## Security

The authenticity (but not the privacy) of requests and responses is protected with HMACs, timestamps,
and challenge values.

The following security practices are assumed:

* Each device is assigned a private HMAC key unique to that device.
* HMAC keys are kept private.
    * Transmission to, storage by, and utilization of the keys by the server is secure
    * The device is physically secured to prevent direct access to flash memory holding key values
* Challenge values are re-generated for each request with a low probability of generating repeat
  values.

### Request Replay

Request replay attacks are possible within a short period of time (measured in seconds). Rate limiting
may be employed to protect server resources from attacks of this nature. Such an attack will give
the attacker brief insight into current setpoints and the server's clock setting. However, both of these
datapoints are already public in the sense that all requests and responses are transmitted in plaintext.

### Response Replay

An attacker with long-term access to the request/response flow may conceivably record responses
bearing challenges that get repeated in future requests. This would allow such an attacker to replay
a response with the correct challenge, corrupting the device's clock setting and setting the
temperature setpoint to an earlier (but not arbitary) value. The effect would be short-lived, as the
next request will fail on clock sync and the device will self-correct within a few seconds. That
period of time is not long enough to give the attacker the ability to meaingfully impact the
function of the device.

Periodic rotation of HMAC keys provides the best available protection against replay attacks based on
long-term observation of responses. Response HMACs generated with old keys will not validate against
those generated with new keys, so changing the HMAC keys invalidates all previous responses that may
have been recorded.
