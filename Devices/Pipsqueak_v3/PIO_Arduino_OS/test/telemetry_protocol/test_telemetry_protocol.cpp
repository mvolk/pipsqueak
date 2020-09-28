#include <Arduino.h>
#include <unity.h>
#include <Hmac.h>
#include <Errors.h>
#include <TelemetryProtocol.h>

#define SECRET_KEY "ThisIsATopSecret32ByteValuePad32"
#define DEVICE_ID 127
#define MOCK_NOW 1234567898
#define CHALLENGE 3876543210
#define MOCK_LATER 1234567900

void test_constructor() {
  TelemetryRequest * subject = new TelemetryRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  TEST_ASSERT_FALSE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
  const byte expectedRequest[64] = {
    0x02, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
  TelemetryRequest * subject = new TelemetryRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  StatusEvent * statusEvent = new StatusEvent();
  statusEvent->temperatureSetpoint(MOCK_NOW - 5, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 5, 15.625);
  subject->addStatusEvent(statusEvent);
  statusEvent->chillerPulse(MOCK_NOW - 4, 1, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 3, 14.5);
  subject->addStatusEvent(statusEvent);
  statusEvent->heaterPulse(MOCK_NOW - 3, 1, 30, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 2, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->error(MOCK_NOW - 1, ErrorType::TcpStack, NETWORK_ERROR_TIMEOUT);
  subject->addStatusEvent(statusEvent);
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_TRUE(subject->isInFlight());
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(176, subject->getSize());
  const byte expectedRequest[176] = {
    0x02, 0x7F, 0x00, 0x00, 0x00, 0xDA, 0x02, 0x96,
    0x49, 0x07, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0xD5, 0x02, 0x96, 0x49, 0x00, 0x00, 0x70,
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xD5, 0x02, 0x96, 0x49, 0x00, 0x00, 0x7A,
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0xD6, 0x02, 0x96, 0x49, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xD7, 0x02, 0x96, 0x49, 0x00, 0x00, 0x68,
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0xD7, 0x02, 0x96, 0x49, 0x01, 0x00, 0x00,
    0x00, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xD8, 0x02, 0x96, 0x49, 0x00, 0x00, 0x70,
    0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0xD9, 0x02, 0x96, 0x49, 0x02, 0xF9, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x51, 0xEB, 0xD3, 0x55, 0x60, 0x62, 0x3C, 0xFE,
    0xDF, 0x17, 0x0D, 0x0A, 0xA5, 0x78, 0xF9, 0xA5,
    0x30, 0x90, 0x70, 0x8C, 0xFF, 0x9F, 0xA6, 0xAB,
    0x8A, 0xC3, 0x8A, 0xBA, 0xD8, 0x72, 0xEB, 0xB0
  };
  TEST_ASSERT_EQUAL_MEMORY(expectedRequest, subject->getBuffer(), 176);
}

void test_reset() {
  TelemetryRequest * subject = new TelemetryRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  StatusEvent * statusEvent = new StatusEvent();
  statusEvent->temperatureSetpoint(MOCK_NOW - 5, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 5, 15.625);
  subject->addStatusEvent(statusEvent);
  statusEvent->chillerPulse(MOCK_NOW - 4, 1, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 3, 14.5);
  subject->addStatusEvent(statusEvent);
  statusEvent->heaterPulse(MOCK_NOW - 3, 1, 30, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 2, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->error(MOCK_NOW - 1, ErrorType::TcpStack, NETWORK_ERROR_TIMEOUT);
  subject->addStatusEvent(statusEvent);
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->reset();
  TEST_ASSERT_FALSE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(64, subject->getSize());
}

void test_failed() {
  TelemetryRequest * subject = new TelemetryRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  StatusEvent * statusEvent = new StatusEvent();
  statusEvent->temperatureSetpoint(MOCK_NOW - 5, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 5, 15.625);
  subject->addStatusEvent(statusEvent);
  statusEvent->chillerPulse(MOCK_NOW - 4, 1, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 3, 14.5);
  subject->addStatusEvent(statusEvent);
  statusEvent->heaterPulse(MOCK_NOW - 3, 1, 30, 1);
  subject->addStatusEvent(statusEvent);
  statusEvent->temperatureObservation(MOCK_NOW - 2, 15.000);
  subject->addStatusEvent(statusEvent);
  statusEvent->error(MOCK_NOW - 1, ErrorType::TcpStack, NETWORK_ERROR_TIMEOUT);
  subject->addStatusEvent(statusEvent);
  subject->ready(MOCK_NOW, CHALLENGE);
  subject->failed();
  TEST_ASSERT_TRUE(subject->isPopulated());
  TEST_ASSERT_FALSE(subject->isInFlight());
  TEST_ASSERT_FALSE(subject->getResponse()->isInUse());
  TEST_ASSERT_FALSE(subject->getResponse()->isComplete());
  TEST_ASSERT_FALSE(subject->getResponse()->isReady());
  TEST_ASSERT_EQUAL(176, subject->getSize());
}

void test_response() {
  TelemetryRequest * subject = new TelemetryRequest(DEVICE_ID, new Hmac((const byte *) &SECRET_KEY));
  StatusEvent * statusEvent = new StatusEvent();
  statusEvent->temperatureSetpoint(MOCK_NOW - 5, 15.000);
  subject->addStatusEvent(statusEvent);
  subject->ready(MOCK_NOW, CHALLENGE);
  TEST_ASSERT_TRUE(subject->getResponse()->isInUse());
  byte response[64] = {
    0x02, 0xDC, 0x02, 0x96, 0x49, 0x00, 0x00, 0xC0,
    0x7F, 0x00, 0xEA, 0x5A, 0x0F, 0xE7, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x51, 0x11, 0x65, 0xC7, 0x99, 0x37, 0xBF, 0x4A,
    0x60, 0x77, 0x42, 0x69, 0x00, 0x40, 0x4A, 0xF9,
    0x6C, 0xA8, 0x52, 0xB0, 0x98, 0xB7, 0xEB, 0xD6,
    0x78, 0x2A, 0x7F, 0x19, 0x21, 0xD8, 0x33, 0x74
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
