#include "basic.hpp"
#include "mfrc.hpp"

void _setup() {
  Serial.begin(9600);
  initMFRC();
}

void _loop() {
  taskMFRC(millis(), micros());
}