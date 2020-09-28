#include <Arduino.h>
#include <PipsqueakState.h>
#include <Hmac.h>
#include <PipsqueakClient.h>

Hmac * hmac;
PipsqueakState * state;
PipsqueakClient * client;

void setup() {
  Serial.begin(57600);
  delay(1000);
  Serial.println();

  state = new PipsqueakState();

  state->setup();

  hmac = new Hmac(state->getConfig()->getSecretKey());

  client = new PipsqueakClient(state, hmac);
  client->setup();
}

void loop() {
  state->loop();
  client->loop();
}
