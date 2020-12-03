#pragma once

#include "univ/pin2.hpp"
#include "pinmap.h"
#include "univ/bool8.hpp"
#include "mfrc.hpp"
#include "cardetect.hpp"

#define LAST_STATE 0
#define DETERMINED 1
#define OUTOFRANGE 2
#define RISED 3
#define LAST_MISSED 4
#define ECHO_ANALOG 5

#define TRIGGER 0
#define ECHO 1

#define SONIC_TIMEOUT 24
// 갱신 주기..
#define DETECT_INTERVAL 200
// 있다는걸 감지하는 최대 거리
#define SONIC_DISTANCE 20
// 최대 초음파센서 길이
#define MAX_DISTANCE 400

#define AUTH_SONIC 0

class UltraSonic {
private:
  float distance;
  uint timeout;
public:
  uint startTime; // micro time
  uint risingTime; // micro time
  Pin2 pins; // trigger / echo
  Bool8 status;
  UltraSonic() {
    // array init, later maybe
  }
  UltraSonic(PIN trig, PIN echo, bool echoAnalog, uint16_t timeout) {
    this->construct(trig, echo, echoAnalog, timeout);
  }
  void construct(PIN trig, PIN echo, bool echoAnalog, uint16_t timeout) {
    this->startTime = 0;
    this->risingTime = 0;
    this->distance = -1;
    this->timeout = timeout * 1000; // ms to ys
    this->pins.setPin1(trig);
    this->pins.setPin2(echo);
    this->status.reset();
    this->status.setBool(ECHO_ANALOG, echoAnalog);
  }
  /**
   * 거리 측정을 싱글 센서에게 요청
   */
  void request(uint microTime) {
    startTime = microTime;
    status.setBool(DETERMINED, false);
    status.setBool(RISED, false);
  }
  /**
   * 루프안에서 계속 감지
   */
  void detect() {
    const uint microTime = micros();
    if (status[DETERMINED]) {
      return;
    }
    if (microTime - startTime >= timeout) {
      if (status[LAST_MISSED]) {
        distance = -1;
      } else {
        status.setBool(LAST_MISSED, true);
        status.setBool(OUTOFRANGE, true);
      }
      status.setBool(DETERMINED, true);
      return;
    }
    // set state
    bool state;
    if (status[ECHO_ANALOG]) {
      // analogPin
      state = analogRead(pins[ECHO]) >= 128;
    } else {
      // digitalPin
      state = digitalRead(pins[ECHO]) == HIGH;
    }
    bool lastState = status[LAST_STATE];
    if (!lastState && state) {
      // rising
      risingTime = micros();
      status.setBool(RISED, true);
      status.setBool(LAST_STATE, state);
    } else if (lastState && !state) {
      if (status[RISED]) {
        // falling
        uint microTime = micros();
        distance = (float)(microTime - risingTime) / 58.82; // 유효범위: 0~200
        if (distance >= MAX_DISTANCE || distance < 2) {
          distance = -1;
        }
        status.setBool(OUTOFRANGE, false);
        status.setBool(LAST_MISSED, false);
        status.setBool(DETERMINED, true);
      }
      status.setBool(LAST_STATE, state);
    }
  }
  bool isDetermined() {
    return status[DETERMINED];
  }
  bool isOutOfRange() {
    return status[OUTOFRANGE];
  }
  float getDistance() {
    return distance;
  }
  void triggerSensor() {
    digitalWrite(pins[TRIGGER], LOW);
    delayMicroseconds(2);
    digitalWrite(pins[TRIGGER], HIGH);
    delayMicroseconds(10);
    digitalWrite(pins[TRIGGER], LOW);
  }
};

class SonicGroup {
private:
  unsigned char printCount;
  UltraSonic sonicSensors[SONIC_PINS_LN];
  Bool8 detected;
  Bool8 lastStates;
  uint8_t triggerPIN;
  CarEvent currentEvents[SONIC_PINS_LN];
public:
  SonicGroup(uint8_t triggerPIN, uint16_t timeout) {
    this->triggerPIN = triggerPIN;
    this->printCount = 0;
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      sonicSensors[i].construct(triggerPIN, SONIC_PINS[i], SONIC_ANALOG[i], timeout);
      currentEvents[i] = CarEvent::NOTHING;
    }
  }
  void printStatus() {
    // Serial.println("| 1 2 3 4 5 6 7 8 |");
    if (++printCount <= 5) {
      return;
    }
    printCount = 0;
    Serial.print("Distance: ");
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      Serial.print(sonicSensors[i].getDistance());
      Serial.print(' ');
    }
    Serial.println();
  }
  void readStatus() {
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      UltraSonic& sonic = sonicSensors[i];
      if (!sonic.isDetermined()) {
        continue;
      }
      uint cardUUID = getCardUUID();
      bool last = lastStates[i];
      bool current = sonic.getDistance() >= 0 && sonic.getDistance() <= SONIC_DISTANCE;
      if (!last && current) {
        if (i == AUTH_SONIC) {
          // read auth
          if (cardUUID == CARD1 || cardUUID == CARD2) {
            // accepted o_<
            currentEvents[i] = CarEvent::AUTH_0T1;
          } else {
            currentEvents[i] = CarEvent::UNAUTH_0T1;
          }
        } else {
          // simple
          currentEvents[i] = CarEvent::NORMAL_0T1;
        }
        // come in
      } else if (last && !current) {
        if (i == AUTH_SONIC) {
          if (cardUUID == CARD1 || cardUUID == CARD2) {
            // accepted
            cardUUID = 0;
            currentEvents[i] = CarEvent::AUTH_1T0;
          } else {
            currentEvents[i] = CarEvent::UNAUTH_1T0;
          }
        } else {
          currentEvents[i] = CarEvent::NORMAL_1T0;
        }
      }
      lastStates.setBool(i, current);
    }
  }
  void trigger() {
    digitalWrite(triggerPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPIN, LOW);
    uint microTime = micros();
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      sonicSensors[i].request(microTime);
    }
  }
  void detect() {
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      sonicSensors[i].detect();
    }
  }
};

namespace Timestamp {
  uint sonicCheck = 0;
}

SonicGroup sonicManager(SONIC_TRIG_PIN, SONIC_TIMEOUT);

void taskScan() {
  const uint time = millis();
  if (time - Timestamp::sonicCheck >= DETECT_INTERVAL) {
    Timestamp::sonicCheck = time;
    sonicManager.readStatus();
    sonicManager.printStatus();
    sonicManager.trigger();
  }
  sonicManager.detect();
}