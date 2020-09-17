#include <ArduinoAdapter.h>
#include <unity.h>
#include <Hmac.h>

void test_hmac() {
  const char password[] = "ThisIsATopSecret32ByteValuePad32";
  const char message[] = "An authentic message";
  Hmac * hmac = new Hmac((const byte *) &password, (const byte *) &message, 20);
  byte * actual_value = new byte [32];
  hmac->write(actual_value);
  byte expected_value[] = {
    0x79, 0x00, 0x67, 0x61, 0x56, 0xB8, 0x98, 0x85,
    0x3C, 0x85, 0xC0, 0x9F, 0xAC, 0xAC, 0xC1, 0xE1,
    0x1E, 0xF1, 0x9E, 0x80, 0x7C, 0x5A, 0x64, 0xD8,
    0xBA, 0x77, 0x75, 0x46, 0xD0, 0x2A, 0xE1, 0x39
  };

  TEST_ASSERT_EQUAL_MEMORY(&expected_value, actual_value, 32);

  delete hmac;
}

void setup() {
  #ifdef ARDUINO
  delay(2000);
  #endif
  UNITY_BEGIN();
}

void loop() {
  RUN_TEST(test_hmac);
  UNITY_END();
}
