#include <Arduino.h>
#include <unity.h>
#include <Hmac.h>
#include <Errors.h>
#include <RebootProtocol.h>

#define SECRET_KEY "ThisIsATopSecret32ByteValuePad32"
#define DEVICE_ID 127
#define MOCK_NOW 1234567898
#define CHALLENGE 3876543210
#define MOCK_LATER 1234567900

void test_constructor() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  TEST_ASSERT_FALSE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x03, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

void test_reportNormalReboot() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->reportNormalReboot();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(68, subject->getSize());
  const uint32_t expectedMessageSize = 4;
  TEST_ASSERT_EQUAL_MEMORY(&expectedMessageSize, &subject->getBuffer()[14], 4);
  const byte expectedMessage[4] = { 0x6E, 0x2F, 0x61, 0x00 };
  TEST_ASSERT_EQUAL_MEMORY(expectedMessage, &subject->getBuffer()[32], 4);
}

void test_reportExceptionalReboot() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  const char * message = "Fatal exception 28 (flag: 2 - EXCEPTION) epc1=0x40208110, epc2=0x00000000, epc3=0x00000000, excvaddr=0x00000000, depc=0x00000000";
  subject->reportExceptionalReboot(message);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(193, subject->getSize());
  const uint32_t expectedMessageSize = 129;
  TEST_ASSERT_EQUAL_MEMORY(&expectedMessageSize, &subject->getBuffer()[14], 4);
  const byte expectedMessage[129] = {
    0x46, 0x61, 0x74, 0x61, 0x6C, 0x20, 0x65, 0x78,
    0x63, 0x65, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20,
    0x32, 0x38, 0x20, 0x28, 0x66, 0x6C, 0x61, 0x67,
    0x3A, 0x20, 0x32, 0x20, 0x2D, 0x20, 0x45, 0x58,
    0x43, 0x45, 0x50, 0x54, 0x49, 0x4F, 0x4E, 0x29,
    0x20, 0x65, 0x70, 0x63, 0x31, 0x3D, 0x30, 0x78,
    0x34, 0x30, 0x32, 0x30, 0x38, 0x31, 0x31, 0x30,
    0x2C, 0x20, 0x65, 0x70, 0x63, 0x32, 0x3D, 0x30,
    0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x2C, 0x20, 0x65, 0x70, 0x63, 0x33, 0x3D,
    0x30, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x2C, 0x20, 0x65, 0x78, 0x63, 0x76,
    0x61, 0x64, 0x64, 0x72, 0x3D, 0x30, 0x78, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2C,
    0x20, 0x64, 0x65, 0x70, 0x63, 0x3D, 0x30, 0x78,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x00
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedMessage, &subject->getBuffer()[32], 129);
}

void test_ready_normal() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->reportNormalReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_TRUE(subject->isInFlight());
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(68, subject->getSize());
  const byte expectedRequest[68] = {
    0x03, 0x7F, 0x00, 0x00, 0x00, 0xDA, 0x02, 0x96,
    0x49, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6E, 0x2F, 0x61, 0x00,
    0x45, 0x8D, 0x4A, 0x61, 0x74, 0xC9, 0x6B, 0x62,
    0xF1, 0x39, 0xB4, 0x12, 0x3D, 0xAC, 0x63, 0x73,
    0x99, 0x48, 0x30, 0xDF, 0xA3, 0x98, 0x4E, 0x63,
    0x32, 0x7B, 0x62, 0x44, 0x30, 0x45, 0xBB, 0x28
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 68);
}

void test_ready_exceptional() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  const char * message = "Fatal exception 28 (flag: 2 - EXCEPTION) epc1=0x40208110, epc2=0x00000000, epc3=0x00000000, excvaddr=0x00000000, depc=0x00000000";
  subject->reportExceptionalReboot(message);
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_TRUE(subject->isInFlight());
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(193, subject->getSize());
  const byte expectedRequest[193] = {
    0x03, 0x7F, 0x00, 0x00, 0x00, 0xDA, 0x02, 0x96,
    0x49, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x81, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x46, 0x61, 0x74, 0x61, 0x6C, 0x20, 0x65, 0x78,
    0x63, 0x65, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20,
    0x32, 0x38, 0x20, 0x28, 0x66, 0x6C, 0x61, 0x67,
    0x3A, 0x20, 0x32, 0x20, 0x2D, 0x20, 0x45, 0x58,
    0x43, 0x45, 0x50, 0x54, 0x49, 0x4F, 0x4E, 0x29,
    0x20, 0x65, 0x70, 0x63, 0x31, 0x3D, 0x30, 0x78,
    0x34, 0x30, 0x32, 0x30, 0x38, 0x31, 0x31, 0x30,
    0x2C, 0x20, 0x65, 0x70, 0x63, 0x32, 0x3D, 0x30,
    0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x2C, 0x20, 0x65, 0x70, 0x63, 0x33, 0x3D,
    0x30, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x2C, 0x20, 0x65, 0x78, 0x63, 0x76,
    0x61, 0x64, 0x64, 0x72, 0x3D, 0x30, 0x78, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2C,
    0x20, 0x64, 0x65, 0x70, 0x63, 0x3D, 0x30, 0x78,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x00,
    0xE8, 0xE0, 0xE5, 0xA7, 0x8C, 0x82, 0x6E, 0x3C,
    0x35, 0x8C, 0xFF, 0x1E, 0xA7, 0xC4, 0x29, 0x7F,
    0xF2, 0x51, 0xD5, 0x30, 0x59, 0xC2, 0x81, 0xC7,
    0xD8, 0x43, 0x87, 0xE9, 0x71, 0x62, 0x4C, 0xC6
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 68);
}

void test_failed() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->reportNormalReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->failed();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(68, subject->getSize());
  const uint32_t expectedMessageSize = 4;
  TEST_ASSERT_EQUAL_MEMORY(&expectedMessageSize, &subject->getBuffer()[14], 4);
  const byte expectedMessage[4] = { 0x6E, 0x2F, 0x61, 0x00 };
  TEST_ASSERT_EQUAL_MEMORY(expectedMessage, &subject->getBuffer()[32], 4);
}

void test_reset() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->reportNormalReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->reset();
  TEST_ASSERT_FALSE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
}

void test_response() {
  ReportRebootRequest * subject = new ReportRebootRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  subject->reportNormalReboot();
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  byte response[64] = {
    0x03, 0xDC, 0x02, 0x96, 0x49, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0B, 0x19, 0x69, 0x02, 0x5C, 0x97, 0x4B, 0xD5,
    0x06, 0x59, 0xCB, 0xEF, 0xE7, 0xC9, 0x3E, 0xA9,
    0x52, 0x3A, 0x24, 0x44, 0xC7, 0xBE, 0x90, 0x6C,
    0x8B, 0xFF, 0x32, 0x4D, 0xBE, 0xE3, 0x3D, 0xAC
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
  RUN_TEST(test_reportNormalReboot);
  RUN_TEST(test_reportExceptionalReboot);
  RUN_TEST(test_ready_normal);
  RUN_TEST(test_ready_exceptional);
  RUN_TEST(test_failed);
  RUN_TEST(test_reset);
  RUN_TEST(test_response);
  UNITY_END();
}

void loop() {
}
