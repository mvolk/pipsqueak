#include <Arduino.h>
#include <unity.h>
#include <Hmac.h>

void test_generate() {
  const char message[] = "An authentic message";
  byte * actual_value = new byte [32];
  Hmac * hmac = new Hmac((const byte *) &("ThisIsATopSecret32ByteValuePad32"));
  hmac->generate((const byte *) &message, 20, actual_value);
  byte expected_value[] = {
    0x79, 0x00, 0x67, 0x61, 0x56, 0xB8, 0x98, 0x85,
    0x3C, 0x85, 0xC0, 0x9F, 0xAC, 0xAC, 0xC1, 0xE1,
    0x1E, 0xF1, 0x9E, 0x80, 0x7C, 0x5A, 0x64, 0xD8,
    0xBA, 0x77, 0x75, 0x46, 0xD0, 0x2A, 0xE1, 0x39
  };

  TEST_ASSERT_EQUAL_MEMORY(&expected_value, actual_value, 32);

  delete hmac;
}

void test_validate() {
  const char message[] = "An authentic message";
  byte expected_value[] = {
    0x79, 0x00, 0x67, 0x61, 0x56, 0xB8, 0x98, 0x85,
    0x3C, 0x85, 0xC0, 0x9F, 0xAC, 0xAC, 0xC1, 0xE1,
    0x1E, 0xF1, 0x9E, 0x80, 0x7C, 0x5A, 0x64, 0xD8,
    0xBA, 0x77, 0x75, 0x46, 0xD0, 0x2A, 0xE1, 0x39
  };
  Hmac * hmac = new Hmac((const byte *) &("ThisIsATopSecret32ByteValuePad32"));

  TEST_ASSERT_TRUE(hmac->validate((const byte *) &message, 20, (const byte *) &expected_value));

  delete hmac;
}

void setup() {
  #ifdef ARDUINO
  delay(2000);
  #endif
  UNITY_BEGIN();
}

void loop() {
  RUN_TEST(test_generate);
  RUN_TEST(test_validate);
  UNITY_END();
}
