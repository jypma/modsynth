#pragma once

#include <Arduino.h>

namespace IO {

extern int16_t cvIn1_0V;
extern int16_t cvIn1_4V;
extern int16_t cvIn2_0V;
extern int16_t cvIn2_4V;

extern int16_t cvOut1_0V;
extern int16_t cvOut1_8V;
extern int16_t cvOut2_0V;
extern int16_t cvOut2_8V;

  bool getGate1In();
  bool getGate2In();
  void setGate1Out(bool on);
  void setGate2Out(bool on);

int16_t getCV1In();
int16_t getCV2In();
uint16_t calcCV1Out(int16_t mV);
uint16_t calcCV2Out(int16_t mV);
void setup();
void readIfNeeded();

}
