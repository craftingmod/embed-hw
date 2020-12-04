#include "main.h"
#include "pinmap.h"
#include "usonic.hpp"
#include "mfrc.hpp"
#include <Wire.h>

void _setup() {
  Serial.begin(115200);
  Wire.begin(9);
  Wire.onRequest(requestI2C);
  initMFRC();
  for (byte i = 0; i < OUTPUT_PINS_LN; i += 1) {
    pinMode(OUTPUT_PINS[i], OUTPUT);
    digitalWrite(OUTPUT_PINS[i], LOW);
  }
  for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
    pinMode(SONIC_PINS[i], INPUT);
  }
}

void _loop() {
  taskMFRC();
  taskScan();
}