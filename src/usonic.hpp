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

// 타임아웃 (microsecond)
#define SONIC_TIMEOUT 24000
// 갱신 주기..
#define DETECT_INTERVAL 200
// 있다는걸 감지하는 최대 거리
#define SONIC_DISTANCE 20
// 최대 초음파센서 길이
#define MAX_DISTANCE 400
// 인덱스
#define AUTH_SONIC 0
// 모두 확인완료 비트
const byte DONE_BIT = (byte)((unsigned short)(1 << SONIC_PINS_LN) - 1);

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
    this->timeout = (unsigned int)timeout * 1000; // ms to ys
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
  bool detect() {
    const uint microTime = micros();
    if (status[DETERMINED]) {
      return true;
    }
    if (abs(microTime - startTime) >= timeout) {
      if (status[LAST_MISSED]) {
        distance = -1;
      } else {
        status.setBool(LAST_MISSED, true);
        status.setBool(OUTOFRANGE, true);
      }
      status.setBool(DETERMINED, true);
      return true;
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
        status.setBool(LAST_STATE, state);
        return true;
      }
      status.setBool(LAST_STATE, state);
    }
    return false;
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
    this->detected.reset();
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
    detected.reset();
    digitalWrite(triggerPIN, LOW);
    delayMicroseconds(2);
    uint microTime = micros();
    digitalWrite(triggerPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPIN, LOW);
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      sonicSensors[i].request(microTime);
    }
  }
  byte detect() {
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      bool determined = sonicSensors[i].detect();
      detected.setBool(i, determined);
    }
    return detected.getRaw();
  }
};

class SonicGroupV2 {
private:
  PIN echoPins[SONIC_PINS_LN];
  Bool8 lastIsError;
  Bool8 detected;
  Bool8 lastStates;
  uint8_t triggerPIN;
  uint16_t timeout;
  CarEvent currentEvents[SONIC_PINS_LN];
  uint risingTimes[SONIC_PINS_LN];
  uint fallingTimes[SONIC_PINS_LN];
public:
  float distances[SONIC_PINS_LN];
  SonicGroupV2(uint8_t trigger, uint16_t timeout) {
    triggerPIN = trigger;
    this->timeout = timeout;
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      echoPins[i] = SONIC_PINS[i];
      currentEvents[i] = CarEvent::NOTHING;
      distances[i] = -1;
    }
  }
  void reset() {
    detected.reset();
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      risingTimes[i] = 0;
      fallingTimes[i] = 0;
    }
  }
  void readStatus() {
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      uint cardUUID = getCardUUID();
      float distance = distances[i];
      bool last = lastStates[i];
      bool current = distance >= 0 && distance <= SONIC_DISTANCE;
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
            setCardUUID(0);
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
  void printDistance() {
    Serial.print("[Distance] ");
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      Serial.print(distances[i]);
      Serial.print(' ');
    }
    Serial.println();
  }
  void printStatus() {
    Serial.print("[Status] ");
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      CarDetect::printEvent(currentEvents[i]);
      Serial.print(' ');
    }
    Serial.println();
  }
  void triggerSync() {
    // 1. reset
    reset();
    // 2. trigger pin
    digitalWrite(triggerPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPIN, HIGH);
    // 3. define variables
    bool turnedOff = false;
    Bool8 completed;
    Bool8 lastStat;
    uint startMicro = micros();
    // 4. async detection
    while (true) {
      uint micro = micros();
      if (!turnedOff && micro - startMicro >= 10) {
        // 1. turn off trigger
        digitalWrite(triggerPIN, LOW);
        turnedOff = true;
      }
      if (micro - startMicro >= timeout) {
        break;
      }
      byte doneN = 0;
      for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
        if (completed[i]) {
          doneN += 1;
          continue;
        }
        bool last = lastStat[i];
        bool state;
        state = digitalRead(echoPins[i]);
        if (!last && state) {
          // rising
          risingTimes[i] = micros();
          lastStat.setBool(i, true);
        } else if (last && !state && risingTimes[i] > 0) {
          // falling
          fallingTimes[i] = micros();
          completed.setBool(i, true);
        }
      }
      if (doneN >= SONIC_PINS_LN) {
        break;
      }
    }
    // 5. store data
    for (byte i = 0; i < SONIC_PINS_LN; i += 1) {
      if (fallingTimes[i] == 0) {
        // OutOfBounds
        if (lastIsError[i]) {
          distances[i] = -1;
        } else {
          lastIsError.setBool(i, true);
        }
      } else {
        lastIsError.setBool(i, false);
        distances[i] = (float)(fallingTimes[i] - risingTimes[i]) / 58.82;
      }
    }
    // 6. read status for use
    readStatus();
  }
};

namespace Timestamp {
  uint sonicCheck = 0;
}

SonicGroupV2 sonicManager(SONIC_TRIG_PIN, SONIC_TIMEOUT);
byte printCount = 0;

void taskScan() {
  const uint time = millis();
  if (time - Timestamp::sonicCheck >= DETECT_INTERVAL) {
    Timestamp::sonicCheck = time;
    sonicManager.triggerSync();
    if (++printCount >= 5) {
      printCount = 0;
      sonicManager.printDistance();
      sonicManager.printStatus();
    }
  }
}