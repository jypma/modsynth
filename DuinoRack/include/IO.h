#pragma once

#include <Arduino.h>

#define INLINE inline __attribute__((always_inline))

namespace IO {

extern int16_t cvIn1_0V;
extern int16_t cvIn1_4V;
extern int16_t cvIn2_0V;
extern int16_t cvIn2_4V;

int16_t getcvOut1_0V();
int16_t getcvOut1_8V();
int16_t getcvOut2_0V();
int16_t getcvOut2_8V();

void calibrateCVOut1_0V(int16_t delta);
void calibrateCVOut1_8V(int16_t delta);
void calibrateCVOut2_0V(int16_t delta);
void calibrateCVOut2_8V(int16_t delta);

void calibratePWMPeriod(int16_t delta);
uint16_t getPWMPeriod();

bool getGate1In();
bool getGate2In();
bool getGate3In();

void setGate1Out(bool on);
void setGate2Out(bool on);
INLINE void setGate1PWM(uint16_t value) {
  OCR1A = value;
}
INLINE void setGate2PWM(uint16_t value) {
  OCR1B = value;
}
void configureGate1Direct();
void configureGate2Direct();
void configureGate1PWM();
void configureGate2PWM();

int16_t getCV1In();
int16_t getCV2In();
uint16_t calcCV1Out(int16_t mV);
uint16_t calcCV2Out(int16_t mV);
void setup();
void readIfNeeded();

}
