#include <Arduino.h>
#include <EEPROM.h>

#define RED_INDICATOR_PIN   D7
#define GREEN_INDICATOR_PIN D6
#define SIGNAL_ENABLE_PIN   D1

bool isMemoryCleared() {
  EEPROM.begin(256);
  byte buffer;
  for (size_t i = 0; i < 256; i++) {
    EEPROM.get(i, buffer);
    if (buffer != 0x00) {
      Serial.println("Found data in the non-volatile memory");
      return false;
    }
  }
  return true;
}

void clearMemory() {
  Serial.println("Attempting to wipe the non-volatile memory...");
  EEPROM.begin(256);
  for (size_t i = 0; i < 256; i++) EEPROM.put(i, (byte) 0x00);
  EEPROM.commit();
}

void setup() {
  pinMode(RED_INDICATOR_PIN, OUTPUT);
  pinMode(GREEN_INDICATOR_PIN, OUTPUT);
  pinMode(SIGNAL_ENABLE_PIN, OUTPUT);

  digitalWrite(SIGNAL_ENABLE_PIN, HIGH);
  digitalWrite(RED_INDICATOR_PIN, HIGH);
  digitalWrite(GREEN_INDICATOR_PIN, LOW);

  Serial.begin(57600);
  delay(1000);
  Serial.print("\n");

  Serial.println("Inspecting non-volatile memory...");
  while (!isMemoryCleared()) {
    clearMemory();
    Serial.println("Verifying that the non-volatile memory has been wiped...");
    for (size_t i = 0; i < 4; i++) {
      digitalWrite(RED_INDICATOR_PIN, LOW);
      delay(250);
      digitalWrite(RED_INDICATOR_PIN, HIGH);
      delay(250);
    }
  }

  Serial.println("Non-volatile memory has been cleared");
  digitalWrite(RED_INDICATOR_PIN, LOW);
  digitalWrite(GREEN_INDICATOR_PIN, HIGH);
}

void loop() {
}
