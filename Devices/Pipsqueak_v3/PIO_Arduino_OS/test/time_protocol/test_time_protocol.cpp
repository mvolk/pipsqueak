#include <Arduino.h>
#include <unity.h>
#include <Hmac.h>
#include <Errors.h>
#include <TimeProtocol.h>

#define SECRET_KEY "ThisIsATopSecret32ByteValuePad32"
#define DEVICE_ID 127
#define MOCK_NOW 1234567890
#define CHALLENGE 3876543210
#define MOCK_LATER 1234567900

void test_constructor() {
  TimeRequest * subject = new TimeRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

void test_ready() {
  TimeRequest * subject = new TimeRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_TRUE(subject->isInFlight());
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x00, 0x7F, 0x00, 0x00, 0x00, 0xD2, 0x02, 0x96,
    0x49, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0xAD, 0x83, 0x4F, 0xF3, 0xF4, 0x4C, 0x17,
    0x0E, 0x46, 0xE7, 0xD0, 0xB7, 0x79, 0x54, 0x64,
    0x90, 0x8C, 0x12, 0x3F, 0xC4, 0xB6, 0xAE, 0x40,
    0x77, 0xC4, 0x4A, 0xAB, 0xF5, 0xD8, 0x5C, 0x28
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 64);
}

void test_failed() {
  TimeRequest * subject = new TimeRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->failed();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
}

void test_reset() {
  TimeRequest * subject = new TimeRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->reset();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
}

void test_response() {
  TimeRequest * subject = new TimeRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  byte response[64] = {
    0x00, 0xDC, 0x02, 0x96, 0x49, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x98, 0x2A, 0x6D, 0xC3, 0x30, 0xA8, 0x3B, 0x99,
    0xFF, 0xE5, 0x71, 0x95, 0x14, 0xDA, 0xBD, 0xD0,
    0x2C, 0xFE, 0x19, 0x17, 0x7B, 0x1E, 0x9C, 0xBC,
    0xD9, 0xE4, 0x32, 0x3F, 0x95, 0xC2, 0x0A, 0xD7
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
  RUN_TEST(test_ready);
  RUN_TEST(test_failed);
  RUN_TEST(test_reset);
  RUN_TEST(test_response);
  UNITY_END();
}

void loop() {
}
