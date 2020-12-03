#pragma once

enum CarEvent {
  NOTHING,
  NORMAL_0T1,
  NORMAL_1T0,
  UNAUTH_0T1,
  UNAUTH_1T0,
  AUTH_0T1,
  AUTH_1T0,
};
namespace CarDetect {
  /**
   * `event`을 문자열로 출력한다.
   */
  void printType(CarEvent event) {
    const char* names[] = {"Nothing", "[Normal] OFF->ON",
      "[Normal] ON->OFF", "[Unauth] OFF->ON", "[Unauth] ON->OFF",
      "[Auth] OFF->ON", "[Auth] ON->OFF"};
    const CarEvent events[] = {NOTHING, NORMAL_0T1,
      NORMAL_1T0, UNAUTH_0T1, UNAUTH_1T0, AUTH_0T1, AUTH_1T0};
    for (byte i = 0; i < 7; i += 1) {
      if (event == events[i]) { 
        Serial.print(names[i]);
        break;
      }
    }
  }
}