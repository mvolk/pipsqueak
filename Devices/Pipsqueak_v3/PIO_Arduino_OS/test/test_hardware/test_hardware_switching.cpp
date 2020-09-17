/**
 * This test exercises the switching hardware and indicators.
 *
 * Successful operation is visually determined:
 * - Green indicator on steady
 * - Repeating cycle of indications:
 *   - Red + Yellow
 *   - Yellow
 *   - Red + Yellow
 *   - Red
 *   - Red + Blue
 *   - Blue
 *   - Red + Blue
 *   - Red
 * - [Optional] Power module switching corresponding to
 *   yellow and blue indicator light illumination
 */

#include <Arduino.h>
#include <unity.h>

const uint8_t ENABLE_PIN           PROGMEM = D1;
const uint8_t ONE_WIRE_PIN         PROGMEM = D2;
const uint8_t RED_LED_PIN          PROGMEM = D7;
const uint8_t GREEN_LED_PIN        PROGMEM = D6;
const uint8_t CHILLER_PIN          PROGMEM = D5;
const uint8_t HEATER_PIN           PROGMEM = D8;

uint8_t phase = 1;
uint32_t cycle = 0;

void setup() {
  UNITY_BEGIN();

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(CHILLER_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);

  UNITY_END();
}

void loop() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, phase < 5 || phase > 8 ? HIGH : LOW);
  digitalWrite(CHILLER_PIN, phase > 2 && cycle % 2 == 0 ? HIGH : LOW);
  digitalWrite(HEATER_PIN, phase > 2 && cycle % 2 == 1 ? HIGH : LOW);

  if (phase == 10) {
    cycle += 1;
  }

  phase = (phase % 10) + 1;

  delay(300);
}
