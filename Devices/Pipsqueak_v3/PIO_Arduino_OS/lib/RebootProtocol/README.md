# Reboot Protocol Library

This library is a subcomponent of the [PipsqueakClient library](../PipsqueakClient/README.md),
which collectively implements the Pipsqueak Protocol.

## Usage

The Pipsqueak Reboot Protocol is used primarily to report reboots and their causes for system
monitoring purposes.

Responses may additionally be used to aid in keeping the device's clock in sync with the server's
clock.

This request's payload is set via the `setNormalReboot` and `setExceptionalReboot` methods, only
one of which is ordinarily invoked per request-response cycle.

A reboot is "normal" if the reset reason code
([`rst_info.reason`](https://github.com/esp8266/Arduino/blob/32470fbfabeb326132f4bb1f79933c7cd0285e17/tools/sdk/include/user_interface.h#L62))
is zero ([`rst_reason::REASON_DEFAULT_RST`](https://github.com/esp8266/Arduino/blob/32470fbfabeb326132f4bb1f79933c7cd0285e17/tools/sdk/include/user_interface.h#L52)).

For exceptional reboots, the `reason` string supplied should be formatted in a manner compatible
with the esp8266 exception decoder.

Refer to the [PipsqueakClient library](../PipsqueakClient/README.md) for general Pipsqueak
request/response guidance.

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
Reboot report recording is idempotent, so replay of requests will not impact data quality.

### Response Replay

An attacker with long-term access to the request/response flow may conceivably record responses
bearing challenges that get repeated in future requests. This would allow such an attacker to replay
a response with the correct challenge, with the potential effect of corrupting the device's clock
setting. The effect would be short-lived, as the next request will fail on clock sync and the device
will self-correct within a few seconds. That period of time is not long enough to meaingfully impact
the function of the device.

The infrequent nature of this protocol's exercise significantly reduces exposure to this replay
vulnerability.

Periodic rotation of HMAC keys provides the best available protection against replay attacks based on
long-term observation of responses. Response HMACs generated with old keys will not validate against
those generated with new keys, so changing the HMAC keys invalidates all previous responses that may
have been recorded.
