#include "main.h"
#include "pinmap.h"
#include "usonic.hpp"
#include "mfrc.hpp"

void _setup() {
  Serial.begin(115200);
  initMFRC();
  for (byte i = 0; i < OUTPUT_PINS_LN; i += 1) {
    pinMode(OUTPUT_PINS[i], OUTPUT);
    digitalWrite(OUTPUT_PINS[i], LOW);
  }
  for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
    if (!SONIC_ANALOG[i]) {
      pinMode(SONIC_PINS[i], INPUT);
    }
  }
}

void _loop() {
  taskMFRC();
  taskScan();
}