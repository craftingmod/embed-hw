#include "basic.hpp"
#include "univ/bool8.hpp"

#pragma once

const PIN OUTPUT_PINS[] = {2, 6};
#define OUTPUT_PINS_LN 2

const PIN BUZZER_PIN = 6;

const PIN SONIC_PINS[] = {3, 4, 7, 8, A0, A1, A2, A3};
// deprecated but just for compile
const bool SONIC_ANALOG[] = {false, false, false, false, false, false, false, false};
#define SONIC_TRIG_PIN 2
#define SONIC_PINS_LN 8