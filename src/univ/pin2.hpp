#pragma once
#include "../basic.hpp"

class Pin2 {
private:
  uint8_t data;
public:
  Pin2() {
    data = 0;
  }
  Pin2(PIN one, PIN two) {
    setPin1(one);
    setPin2(two);
  }
  PIN getPin1() const {
    return data >> 4;
  }
  PIN getPin2() const {
    return data & 0xF;
  }
  void setPin1(PIN pin) {
    data = (data & 0xF) | (pin << 4);
  }
  void setPin2(PIN pin) {
    data = (data & 0xF0) | pin;
  }
  PIN operator[](byte index) {
    return (index == 0) ? getPin1() : getPin2();
  }
  void reset() {
    data = 0;
  }
};