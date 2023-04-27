#pragma once

#include <Arduino.h>
#include "Inline.h"

template <typename T>
inline T applyDelta(T value, int8_t delta, T min, T max) {
  if (delta > 0) {
    if (value < max - delta) {
      return value + delta;
    } else {
      return max;
    }
  } else {
    if (value > min - delta) {
      return value + delta;
    } else {
      return min;
    }
  }
}

template <typename T>
inline T applyDelta16(T value, int16_t delta, T min, T max) {
  if (delta > 0) {
    if (value < max - delta) {
      return value + delta;
    } else {
      return max;
    }
  } else {
    if (value > min - delta) {
      return value + delta;
    } else {
      return min;
    }
  }
}

namespace IO {

  void loadCalibration();
  void saveCalibration();

extern int16_t cvIn1_0V;
extern int16_t cvIn1_4V;
extern int16_t cvIn2_0V;
extern int16_t cvIn2_4V;

int16_t getcvOut1_0V();
int16_t getcvOut1_4V();
int16_t getcvOut2_0V();
int16_t getcvOut2_4V();
int16_t getGate1_0V();
int16_t getGate1_4V();
int16_t getGate2_0V();
int16_t getGate2_4V();

void calibrateCVOut1_0V(int16_t delta);
void calibrateCVOut1_4V(int16_t delta);
void calibrateCVOut2_0V(int16_t delta);
void calibrateCVOut2_4V(int16_t delta);
void calibrateGate1_0V(int16_t delta);
void calibrateGate1_4V(int16_t delta);
void calibrateGate2_0V(int16_t delta);
void calibrateGate2_4V(int16_t delta);

void calibratePWMPeriod(int16_t delta);
uint16_t getPWMPeriod();

bool getGate1In();
bool getGate2In();
bool getGate3In();

INLINE void setGate1Out(uint16_t value) {
  OCR1A = value;
}
INLINE void setGate2Out(uint16_t value) {
  OCR1B = value;
}

int16_t getCV1In();
int16_t getCV2In();
uint16_t calcCV1Out(int16_t mV);
uint16_t calcCV2Out(int16_t mV);
uint16_t calcGate1Out(int16_t mV);
uint16_t calcGate2Out(int16_t mV);
void setup();
void readIfNeeded();

}
