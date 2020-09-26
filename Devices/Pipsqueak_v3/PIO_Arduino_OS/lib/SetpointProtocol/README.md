# Setpoint Protocol Library

This library is a subcomponent of the [Pipsqueak Protocol library](../PipsqueakProtocol/README.md).

## Usage

The Pipsqueak Setpoint Protocol is used primarily to receive new setpoint instructions from the
server. Upon updating the setpoint, the new setpoint is reported via the
[Telemetry Protocol](../TelemetryProtocol/README.md).

Responses may additionally be used to aid in keeping the device's clock in sync with the server's
clock.

Refer to the [Pipsqueak Protocol library](../PipsqueakProtocol/README.md) for general Pipsqueak
request/response guidance.

## Request Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | -------------------------------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | [Standard Header Field] The Protocol ID
| 1     | 4   | 4      | uint32 | [Standard Header Field] The unique device ID assigned to each Pipsqueak hardware device
| 5     | 8   | 4      | uint32 | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 9     | 9   | 1      | uint8t | Reboot flag - 0x01 if request is due to reboot, 0x00 otherwise
| 10    | 13  | 4      | uint32 | [Standard Header Field] An arbitrary challenge value that should be different per request
| 14    | 31  | 18     | ------ | Reserved
| 32    | 63  | 32     | byte[] | [Standard Field] HMAC

## Response Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | ---------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | [Standard Header Field] The Protocol ID
| 1     | 4   | 4      | uint32 | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 5     | 8   | 4      | float  | The current temperature control setpoint
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
