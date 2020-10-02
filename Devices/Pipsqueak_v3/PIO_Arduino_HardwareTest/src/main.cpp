#include <Arduino.h>

const uint8_t ENABLE_PIN           PROGMEM = D1;
const uint8_t ONE_WIRE_PIN         PROGMEM = D2;
const uint8_t RED_LED_PIN          PROGMEM = D7;
const uint8_t GREEN_LED_PIN        PROGMEM = D6;
const uint8_t CHILLER_PIN          PROGMEM = D5;
const uint8_t HEATER_PIN           PROGMEM = D8;

uint8_t phase = 1;
uint32_t cycle = 0;

void setup() {
  Serial.begin(57600);

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(CHILLER_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
}

void loop() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, phase < 5 || phase > 8 ? HIGH : LOW);
  digitalWrite(CHILLER_PIN, phase > 2 && cycle % 2 == 0 ? HIGH : LOW);
  digitalWrite(HEATER_PIN, phase > 2 && cycle % 2 == 1 ? HIGH : LOW);

  if (phase == 10) {
    Serial.println("Cycle #" + String(cycle) + "\n");
    Serial.flush();
    cycle += 1;
  }
  phase = (phase % 10) + 1;
  delay(300);
}
