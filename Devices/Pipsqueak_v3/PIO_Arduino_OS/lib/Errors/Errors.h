#ifndef Errors_h
#define Errors_h

enum ErrorType { None = 0, Pipsqueak = 1, TcpStack = 2 };

// Network Error Codes ////////////////////////////////////////////////////////////////////////////
// Network errors are reported by the Pipsqueak device
// Use values between -1 and -32

#define NETWORK_ERROR_WIFI_CONNECTION -1
#define NETWORK_ERROR_CONNECTION_FAILED -2
#define NETWORK_ERROR_CLIENT_STATE -3
#define NETWORK_ERROR_BUFFER_FULL -4
#define NETWORK_ERROR_BROKEN_PIPE -5
#define NETWORK_ERROR_CONNECTION_LOST -6
#define NETWORK_ERROR_TIMEOUT -7

// Error code placeholder that means "no error"
#define ERROR_NONE 0x00

// Response Status Flags //////////////////////////////////////////////////////////////////////////
// Status flags are set by the server in responses to indicate problems with requests

// The device's clock is too far behind the server's clock
// This can also indicate extreme network latency
#define STATUS_MASK_CLOCK_SYNC_BEHIND 0x01
// The device's clock is too far ahead of the server's clock (generally, "in the future")
#define STATUS_MASK_CLOCK_SYNC_AHEAD 0x02
// The request's HMAC could not be authenticated by the server
#define STATUS_MASK_AUTHENTICATION 0x04
// The server is rate limiting requests from this device
// In practice, the server is more likely to drop the connection than report
// that rate limiting was employed.
#define STATUS_MASK_RATE_LIMITED 0x08
// Too much data was sent to the server
#define STATUS_MASK_EXCESS_DATA 0x10
// The server reports that the device ID is not registered with the server
#define STATUS_MASK_DEVICE_NOT_REGISTERED 0x20
// The server is reporting that it received this request while still
// in the process of handling another request from the same device.
// Requests from each device are expected to be strictly serial.
// This can happen if multiple devices are assigned the same device ID in error.
#define STATUS_MASK_CONCURRENT_REQUESTS 0x40
// The server could encountered a status event in a StoreStatusEventRequest whose type is not known
#define STATUS_MASK_UNKNOWN_EVENT_TYPE 0x80


// Request Error Codes ////////////////////////////////////////////////////////////////////////////
// Request errors are detected based on status codes returned by the server
// Use values between 1 and 31

// Corresponds to STATUS_MASK_CLOCK_SYNC_BEHIND
#define REQUEST_ERROR_CLOCK_SYNC_BEHIND 1
// Corresponds to STATUS_MASK_CLOCK_SYNC_AHEAD
#define REQUEST_ERROR_CLOCK_SYNC_AHEAD 2
// Corresponds to STATUS_MASK__AUTHENTICATION
#define REQUEST_ERROR_AUTHENTICATION 3
// Corresponds to STATUS_MASK_RATE_LIMITED
#define REQUEST_ERROR_RATE_LIMITED 4
// Corresponds to STATUS_MASK_EXCESS_DATA
#define REQUEST_ERROR_EXCESS_DATA 5
// Corresponds to STATUS_MASK_DEVICE_NOT_REGISTERED
#define REQUEST_ERROR_DEVICE_NOT_REGISTERED 6
// The server reports that it is still busy processing the previous request from this device
// Corresponds to STATUS_MASK__CONCURRENT_REQUESTS
#define REQUEST_ERROR_CONCURRENT_REQUESTS 7
// The server doesn't recognize the type of an event reported via a StoreStatusEventsRequest
// Corresponds to STATUS_MASK__UNKNOWN_EVENT_TYPE
#define REQUEST_ERROR_UNKNOWN_EVENT_TYPE 8
// Request was enqueued without a meaningful (isPopulated()) payload
#define REQUEST_NOT_POPULATED 9


// Response Error Codes ///////////////////////////////////////////////////////////////////////////
// Response errors are reported by the Pipsqueak device to indicate problems with server responses
// Use values between 32 and 64

// More than the expected number of bytes were received
#define RESPONSE_ERROR_EXCESS_DATA 32
// The connection ended before the expected number of bytes were received
#define RESPONSE_ERROR_TRUNCATED_RESPONSE 33
// The response HMAC could not be verified
#define RESPONSE_ERROR_AUTHENTICATION 34
// The protocol identified in the server response did not match the expected protocol identifier value
#define RESPONSE_ERROR_INVALID_PROTOCOL 35
// The challenge response from the server did not match the challenge in the request
#define RESPONSE_ERROR_CHALLENGE_FAILED 36

// Device Error Codes  ////////////////////////////////////////////////////////////////////////////
// Indicate hardware, software or state errors
// Use values between 64 and 96

// Clock isn't synchronized
#define CLOCK_SYNC_ERROR 65
// Board sensor not detected
#define BOARD_SENSOR_DETECTION_ERROR 66
// Remote sensor not detected
#define REMOTE_SENSOR_DETECTION_ERROR 67
// Board temperature exceeds limit
#define DEVICE_OVERHEATED_ERROR 68
// Unable to acquire board temperature
#define BOARD_TEMPERATURE_NAN_ERROR 69
// Unable to acquire remote temperature
#define REMOTE_TEMPERATURE_NAN_ERROR 70
// WiFi connection failure
#define WIFI_CONNECTION_ERROR 71

#endif // Errors_h
