#include <Arduino.h>
#include <PipsqueakState.h>
#include <Hmac.h>
#include <PipsqueakClient.h>
#include <PipsqueakIndicators.h>
#include <PipsqueakSensors.h>
#include <PipsqueakController.h>

Hmac * hmac;
PipsqueakState * state;
PipsqueakClient * client;
PipsqueakIndicators * indicators;
PipsqueakSensors * sensors;
PipsqueakController * controller;

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

  sensors = new PipsqueakSensors(state);
  sensors->setup();

  controller = new PipsqueakController(state);
  controller->setup();

  // Enables powering up the control pins
  pinMode(state->getConfig()->getSignalEnablePin(), OUTPUT);
  digitalWrite(state->getConfig()->getSignalEnablePin(), HIGH);
}

void loop() {
  state->loop();
  client->loop();
  indicators->loop();
  sensors->loop();
  controller->loop();
}
