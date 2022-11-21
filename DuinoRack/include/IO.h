#pragma once

#include <Arduino.h>

namespace IO {
  extern uint16_t cvIn1;
  extern uint16_t cvIn2;

  int16_t getCV1();
  int16_t getCV2();
  void setup();
  void readIfNeeded();
}
