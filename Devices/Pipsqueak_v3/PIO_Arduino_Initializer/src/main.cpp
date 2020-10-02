#include <Arduino.h>
#include <InitializationParameters.h>
#include <PipsqueakConfig.h>
#include <DS18B20Bus.h>

// Configure InitializationParameters.h & .cpp before running


bool success = true;
size_t maxAddresses = 2;
byte addressBuffer[16];
PipsqueakConfig * config = new PipsqueakConfig();;
DS18B20Bus * bus = new DS18B20Bus();


void setup() {
  // Set up Serial for output to a monitor
  Serial.begin(57600);
  Serial.print("\n");
  for (uint8_t t = 3; t > 0; t--) {
    Serial.printf("Standby period ends in %d seconds...\n", t);
    Serial.flush();
    delay(1000);
  }
  Serial.print("\n");
  Serial.print("\n");

  // Configure pins
  pinMode(CONFIG_GREEN_INDICATOR_PIN, OUTPUT);
  pinMode(CONFIG_RED_INDICATOR_PIN, OUTPUT);
  pinMode(CONFIG_SIGNAL_ENABLE_PIN, OUTPUT);

  // Green and red illuminated at startup
  digitalWrite(CONFIG_GREEN_INDICATOR_PIN, HIGH);
  digitalWrite(CONFIG_RED_INDICATOR_PIN, HIGH);

  // Enable signals (such as indicator signals above)
  digitalWrite(CONFIG_SIGNAL_ENABLE_PIN, HIGH);

  // Read the virtual EEPROM contents
  config->readContents();
  yield();

  // Apply any updates from InitializationParameters in volatile memory only
  config->applyUpdates();

  // Detect the onboard temperature sensor
  size_t countOfSensors = bus->search(addressBuffer, maxAddresses);
  yield();

  // Order matters - configure the remote sensor before the board sensor
  if (config->detectRemoteSensor(addressBuffer, countOfSensors)) {
    bus->setResolution(config->remoteSensorAddress(), 12);
  }
  yield();

  // Configure the board sensor
  if (config->detectBoardSensor(addressBuffer, countOfSensors)) {
    bus->setResolution(config->boardSensorAddress(), 9);
  } else {
    success = false;
  }
  yield();

  if (DRY_RUN) {
    Serial.println("Dry Run - no modifications made");
    config->printContents();
  } else if (!success) {
    Serial.println("Failed - no modifications made");
  } else if (config->updated()) {
    if (config->previouslyInitialized() && !ALLOW_UPDATE) {
      Serial.println("Updates disallowed by option - no modifications made");
    } else if (config->commitUpdates()) {
      Serial.println("Configuration updates committed to non-volatile memory");
      config->verifyContents();
    } else {
      Serial.println("Failed (invalid configuration?) - no modifications made");
    }
  } else {
    Serial.println("No updates detected - no modifications made");
  }

  if (success) {
    digitalWrite(CONFIG_RED_INDICATOR_PIN, LOW);
  }
}


void loop() {
}
