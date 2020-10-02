#include "InitializationParameters.h"

// The 32-byte secret key used by the server and this device to
// compute and verify HMACs on messages passed between the two.
// Must be held secret between the server and device.
// Should be unique to each device.
// All zeros is an illegal value, used as a placeholder.
extern "C" const byte CONFIG_SECRET_KEY[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// The IP address of the service this device will talk to.
// 127.0.0.1, the loopback address, is not a legal value.
extern "C" const uint8_t CONFIG_HOST_IP[] = {127, 0, 0, 1};
