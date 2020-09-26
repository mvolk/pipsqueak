# Time Protocol Library

This library is a subcomponent of the [PipsqueakClient library](../PipsqueakClient/README.md),
which collectively implements the Pipsqueak Protocol.

## Useage

The Pipsqueak Time Protocol is used to acquire the server's current timestamp in order to synchronize
the device's clock with the server.

Devices are expected to utilize this protocol with relative infrequency. In general, this protocol
should be used only upon boot-up and when clock drift is detected.

Refer to the [PipsqueakClient library](../PipsqueakClient/README.md) for general Pipsqueak
request/response guidance.

## Request Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | -------------------------------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | [Standard Header Field] The Protocol ID
| 1     | 4   | 4      | uint32 | [Standard Header Field] The unique device ID assigned to each Pipsqueak hardware device
| 5     | 8   | 4      | uint32 | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 9     | 9   | 1      | ------ | Reserved
| 10    | 13  | 4      | uint32 | [Standard Header Field] An arbitrary challenge value that should be different per request
| 14    | 31  | 18     | ------ | Reserved
| 32    | 63  | 32     | byte[] | [Standard Field] HMAC

## Response Specification

Note that the units are bytes, and both Start and End are inclusive.

| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | ---------------------------------------------------------------------
| 0     | 0   | 1      | uint8  | [Standard Header Field] The Protocol ID
| 1     | 4   | 4      | uint32 | [Standard Header Field] The timestamp (seconds since Jan 1 1970) when the message was sent
| 5     | 8   | 4      | ------ | Reserved
| 9     | 1   | 1      | uint8  | [Standard Header Field] Bitmasked status code
| 10    | 13  | 4      | uint32 | [Standard Header Field] The challenge value from the corresponding request
| 14    | 31  | 18     | ------ | Reserved
| 32    | 63  | 32     | byte[] | [Standard Field] HMAC

## Security

The authenticity (but not the privacy) of requests and responses is protected with HMACs and challenge
values.

The following security practices are assumed:

* Each device is assigned a private HMAC key unique to that device.
* HMAC keys are kept private.
    * Transmission to, storage by, and utilization of the keys by the server is secure
    * The device is physically secured to prevent direct access to flash memory holding key values
* Challenge values are re-generated for each request with a low probability of generating repeat
  values.

### Request Replay

Since this protocol is used when the device's clock setting is unreliable, the server will not reject
requests whose timestamps are grossly out of sync with the server's clock. This behavior means that
previously-issued requests can be replayed. The response, bearing the server's current timestamp,
is unlikely to be useful to an attacker, and the server resources expended to serve the response
are quite minimal. However, there is a DDOS vulnerability here, and the server may employ device-scoped
rate limiting to mitigate that risk. Additionally, the server will reject requests for unregistered
devices and those requests bearing invalid HMACs.

### Response Replay

Response replay is generally thwarted by challenge values and HMACs that prevent manipulation of
challenge responses. However, if a challenge value from a previous request is repeated in a new
request, and the HMAC key has not since changed, the previous response may be successfully replayed.
That would result in a temporary clock sync error that would be detected and corrected upon
the next request, resulting in minimal disruption to the system.

Assuming a good random distribution of challenge values and the low frequency with which this protocol
is exercised, exposure to response replay attacks is expected to be quite low in practice. This,
combined with the low impact of succesful attacks, is likely to render this vulnerability moot
in practice.

Periodic rotation of HMAC keys provides the best available protection against replay attacks based on
long-term observation of responses. Response HMACs generated with old keys will not validate against
those generated with new keys, so changing the HMAC keys invalidates all previous responses that may
have been recorded.

Response replay attacks are expected to be thwarted by challenge values. That said, a persistent
attacked has the potential to hold an outdated response bearing a challenge value repeated in a future
request. This would allow the attacker to roll back the device's clock, which would cause the next
request to fail on clock synchronization, prompting the issuance of a new time request.

Given the infrequency of time requests and the device-specificity of the responses, it is unlikely
that this attack vector will be fruitful. Even if successfully deployed, the impact of this attack
is expected to be minimal. Exposure can be further reduced with periodic rotation of HMAC keys.
Response HMACs generated with old keys will not validate against those generated with new keys.

## Security

The authenticity (but not the privacy) of requests and responses is protected with HMACs, timestamps,
and challenge values.

Request replay attacks are possible within a short period of time (measured in seconds). Rate limiting
may be employed to protect server resources from attacks of this nature. Such an attack will give
the attacker brief insight into current setpoints and the server's clock setting. However, both of these
datapoints are already public in the sense that all requests and responses are transmitted in plaintext.

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
