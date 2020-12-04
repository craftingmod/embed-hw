#include "basic.hpp"
#include "univ/bool8.hpp"

#pragma once

const PIN OUTPUT_PINS[] = {2, 5, 6};
#define OUTPUT_PINS_LN 3

// 3, 4, 7, 8, A0, A1, A2, A3
// 1, 2, 3, 4, 5, 6, 7, 8
// 1, 2, 4, 6, 8, 3, 5, 7
// 3, 4, 8, A1, A3, 7, A0, A2
const PIN SONIC_PINS[] = {A1, A0, 4, 8, 7, A3, A2, 3};
// deprecated but just for compile
const bool SONIC_ANALOG[] = {false, false, false, false, false, false, false, false};
#define SONIC_TRIG_PIN 2
#define SONIC_PINS_LN 8

#define LED_RED 5
#define LED_GREEN 6


// 7 - [8]
// 5 - [6]
// 3 - [2]
// 1 - [4]

// 3pin: 8자리
// A3: 6자리
// 8: 4자리
// A0: 2자리
// A1: 1자리
// 4: 3자리
// 7: 5자리
// A2: 7자리
// A1, A0, 4, 8, 7, A3, A2, 3