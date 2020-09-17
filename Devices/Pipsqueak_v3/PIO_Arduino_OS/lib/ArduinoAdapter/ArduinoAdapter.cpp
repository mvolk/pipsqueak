#include "ArduinoAdapter.h"

#ifndef ARDUINO

void yield() {
  int i = 0;
  i += 1;
}

int main() {
  setup();
  int loopNumber = 0;
  while (loopNumber < MAX_LOOP_COUNT) {
    loopNumber += 1;
    loop();
  }
  return 0;
}

#endif // ARDUINO
