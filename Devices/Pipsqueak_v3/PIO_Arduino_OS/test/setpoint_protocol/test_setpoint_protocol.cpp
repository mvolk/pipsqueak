#include <Arduino.h>
#include <unity.h>
#include <Hmac.h>
#include <Errors.h>
#include <SetpointProtocol.h>

#define SECRET_KEY "ThisIsATopSecret32ByteValuePad32"
#define DEVICE_ID 127
#define MOCK_NOW 1234567898
#define CHALLENGE 3876543210
#define MOCK_LATER 1234567900

void test_constructor() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x01, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 64);
}

void test_not_reboot() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  uint8_t expectedValue = 0x00;
  TEST_ASSERT_EQUAL_MEMORY(&expectedValue, &subject->getBuffer()[9], 1);
}

void test_setReboot() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->setReboot();
  uint8_t expectedValue = 0x01;
  TEST_ASSERT_EQUAL_MEMORY(&expectedValue, &subject->getBuffer()[9], 1);
}

void test_ready() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->setReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_TRUE(subject->isInFlight());
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x01, 0x7F, 0x00, 0x00, 0x00, 0xDA, 0x02, 0x96,
    0x49, 0x01, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA8, 0x29, 0x46, 0x23, 0xE9, 0xC5, 0xD7, 0x1B,
    0x6C, 0x08, 0x3A, 0x7A, 0x60, 0x23, 0xA7, 0x26,
    0x1D, 0x65, 0x3C, 0x00, 0x36, 0x8E, 0xDC, 0x34,
    0x62, 0x37, 0x25, 0x76, 0xC0, 0x83, 0x80, 0xB4
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 64);
}

void test_failed() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->setReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->failed();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  uint8_t expectedValue = 0x01;
  TEST_ASSERT_EQUAL_MEMORY(&expectedValue, &subject->getBuffer()[9], 1);
}

void test_reset() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->setReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->reset();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  uint8_t expectedValue = 0x00;
  TEST_ASSERT_EQUAL_MEMORY(&expectedValue, &subject->getBuffer()[9], 1);
}

void test_response() {
  SetpointRequest * subject = new SetpointRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  byte response[64] = {
    0x01, 0xDC, 0x02, 0x96, 0x49, 0x00, 0x00, 0xC0,
    0x7F, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x13, 0x4F, 0x42, 0x7F, 0xF7, 0xCC, 0xA2, 0x43,
    0x97, 0xB9, 0x7F, 0xF2, 0x88, 0xE7, 0x70, 0xEB,
    0xEF, 0x42, 0xB0, 0x04, 0x8E, 0x4B, 0x98, 0x91,
    0x22, 0xD8, 0x8C, 0xC6, 0x7F, 0xC8, 0x0C, 0x61
  };
  subject->getResponse()->receiveBytes(&response[0], 32);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_FALSE(subject->getResponse()->hasErrors());
  TEST_ASSERT_EQUAL(0, subject->getResponse()->errorCount());
  subject->getResponse()->receiveBytes(&response[32], 32);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_TRUE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_FALSE(subject->getResponse()->hasErrors());
  TEST_ASSERT_EQUAL(0, subject->getResponse()->errorCount());
  subject->getResponse()->ready(0);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_TRUE(subject->getResponse()->isComplete());
  TEST_ASSERT_TRUE(subject->getResponse()->isReady());
  TEST_ASSERT_FALSE(subject->getResponse()->hasErrors());
  TEST_ASSERT_EQUAL(0, subject->getResponse()->errorCount());
  TEST_ASSERT_EQUAL(ErrorType::None, subject->getResponse()->getErrorType(0));
  TEST_ASSERT_EQUAL(ERROR_NONE, subject->getResponse()->getErrorCode(0));
  TEST_ASSERT_EQUAL(MOCK_LATER, subject->getResponse()->getTimestamp());
}

void setup() {
  #ifdef ARDUINO
  delay(2000);
  #endif
  UNITY_BEGIN();
  RUN_TEST(test_constructor);
  RUN_TEST(test_not_reboot);
  RUN_TEST(test_setReboot);
  RUN_TEST(test_ready);
  RUN_TEST(test_failed);
  RUN_TEST(test_reset);
  RUN_TEST(test_response);
  UNITY_END();
}

void loop() {
}
