#pragma once
#include "../basic.hpp"

class Bool8 {
private:
  uint8_t data;
public:
  Bool8() {
    data = 0;
  }
  Bool8(uint8_t binary) {
    data = binary;
  }
  bool operator[](byte offset) {
    return getBool(offset);
  }

  /**
   * offset의 bool값 리턴
   */
  bool getBool(uint8_t offset) {
    if (offset >= 8 || offset < 0) {
      return false;
    }
    return (data & (B1 << offset)) != B0;
  }
  void setBool(uint8_t offset, const bool value) {
    if (offset >= 8 || offset < 0) {
      return;
    }
    if (value) {
      data |= (B1 << offset);
    } else {
      data &= ~(B1 << offset);
    }
  }
  void reset() {
    data = 0;
  }
  bool operator<<(uint8_t offset) {
    return this->getBool(offset);
  }
};