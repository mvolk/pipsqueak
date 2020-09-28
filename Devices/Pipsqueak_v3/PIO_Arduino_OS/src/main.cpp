#include <Arduino.h>
#include <PipsqueakState.h>
#include <Hmac.h>
#include <PipsqueakClient.h>
#include <PipsqueakIndicators.h>

Hmac * hmac;
PipsqueakState * state;
PipsqueakClient * client;
PipsqueakIndicators * indicators;

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial.println();

  state = new PipsqueakState();
  state->setup();

  hmac = new Hmac(state->getConfig()->getSecretKey());

  client = new PipsqueakClient(state, hmac);
  client->setup();

  indicators = new PipsqueakIndicators(state);
  indicators->setup();

  // Enables powering up the control pins
  pinMode(state->getConfig()->getSignalEnablePin(), OUTPUT);
  digitalWrite(state->getConfig()->getSignalEnablePin(), HIGH);
}

void loop() {
  state->loop();
  client->loop();
  indicators->loop();
}
