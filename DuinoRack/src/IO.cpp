#include <Arduino.h>
#include <avr/sfr_defs.h>
#include <mozzi_fixmath.h>
#include "IO.h"
#include "Module.h"

namespace IO {
uint16_t cvIn1 = 0;
uint16_t cvIn2 = 0;
bool readingCv1 = true;

int16_t cvIn1_0V = 571;
int16_t cvIn1_4V = 269;
int16_t cvIn2_0V = 573;
int16_t cvIn2_4V = 270;

int16_t cvOut1_0V = 1335;
int16_t cvOut1_4V = 2659;
int16_t cvOut2_0V = 1328;
int16_t cvOut2_4V = 2653;

Q15n16 cvOut1Factor = 0;
Q15n16 cvOut2Factor = 0;

int16_t getcvOut1_0V() { return cvOut1_0V; }
int16_t getcvOut1_4V() { return cvOut1_4V; }
int16_t getcvOut2_0V() { return cvOut2_0V; }
int16_t getcvOut2_4V() { return cvOut2_4V; }

void recalibrate() {
  cvOut1Factor = Q15n0_to_Q15n16(cvOut1_4V - cvOut1_0V) / 1000 / 4;
  cvOut2Factor = Q15n0_to_Q15n16(cvOut2_4V - cvOut2_0V) / 1000 / 4;
}

void calibratePWMPeriod(int16_t delta) {
  ICR1 = constrain(ICR1 + delta, 800, uint32_t(F_CPU) / SAMPLERATE);
}

uint16_t getPWMPeriod() {
  return ICR1;
}

void calibrateCVOut1_0V(int16_t delta) {
  cvOut1_0V = constrain(cvOut1_0V + delta, 0, cvOut1_4V);
  recalibrate();
}

void calibrateCVOut1_4V(int16_t delta) {
  cvOut1_4V = constrain(cvOut1_4V + delta, cvOut1_0V, 4095);
  recalibrate();
}

void calibrateCVOut2_0V(int16_t delta) {
  cvOut2_0V = constrain(cvOut2_0V + delta, 0, cvOut2_4V);
  recalibrate();
}

void calibrateCVOut2_4V(int16_t delta) {
  cvOut2_4V = constrain(cvOut2_4V + delta, cvOut2_0V, 4095);
  recalibrate();
}

int16_t getCV1In() {
  return (int32_t(cvIn1_0V) - cvIn1) * 4000 / (cvIn1_0V - cvIn1_4V);
}

int16_t getCV2In() {
  return (int32_t(cvIn2_0V) - cvIn2) * 4000 / (cvIn2_0V - cvIn2_4V);
}

uint16_t calcCV1Out(int16_t mV) {
  // TODO compare generated assembly to https://github.com/rekka/avrmultiplication/blob/master/mult32x16.h
  return Q15n16_to_Q15n0(cvOut1Factor * mV) + cvOut1_0V;
}

uint16_t calcCV2Out(int16_t mV) {
  return Q15n16_to_Q15n0(cvOut2Factor * mV) + cvOut2_0V;
}

bool getGate1In() {
  return (PINB & (1 << 0)) != 0;
}

bool getGate2In() {
  return (PINC & (1 << 2)) != 0;
}

bool getGate3In() {
  // This is behind the MIDI in optocoupler, hence inverted.
  return (PIND & (1 << 0)) == 0;
}

void setGate1Out(bool on) {
  // PB1, D9
  if (on) {
    PORTB |= (1 << 1);
  } else {
    PORTB &= ~(1 << 1);
  }
}

void setGate2Out(uint8_t on) {
  // PB2, D10
  if (on) {
    PORTB |= (1 << 2);
  } else {
    PORTB &= ~(1 << 2);
  }
}

// FIXME no configure, just use PWM for everything if the filter holds up.
void configureGate1Direct() {
  TCCR1A &= ~_BV(COM1A1) & ~_BV(COM1A0);
  setGate1Out(false);
}

void configureGate2Direct() {
  TCCR1A &= ~_BV(COM1B1) & ~_BV(COM1B0);
  setGate2Out(false);
}

void configureGate1PWM() {
  // Non-inverting fast PWM mode
  TCCR1A |= _BV(COM1A1);
  setGate1PWM(0);
}

void configureGate2PWM() {
  // Non-inverting fast PWM mode
  TCCR1A |= _BV(COM1B1);
  setGate2PWM(0);
}

void setup() {
  recalibrate();

  // Select Vref=AVcc
  ADMUX = _BV(REFS0);
  //set prescaller to 128 (at 16MHz, is 125kHz)and enable ADC
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);
  // Start with channel 0 (CV in 1)
  readingCv1 = true;
  ADMUX = (ADMUX & 0xF0) | (0 & 0x0F);
  //single conversion mode
  ADCSRA |= _BV(ADSC);

  pinMode(8, INPUT); // PB0, Gate IN 1
  pinMode(16, INPUT); // PC2, Gate IN 2

  pinMode(9, OUTPUT); // PB1, Gate OUT 1
  pinMode(10, OUTPUT); // PB2, Gate OUT 2

  // Set up timer 1 for maximum speed
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;//initialize counter value to 0
  constexpr uint16_t ovfValue = 1024;
  ICR1 = ovfValue;
  // Waveform generation mode 14: Fast PWM, TOP is ICR1,
  // and Timer1 clock source is CPU / 1 (16MHz).
  TCCR1A |= _BV(WGM11);
  TCCR1B |= _BV(WGM12) | _BV(WGM13) | _BV(CS10);

  configureGate1Direct();
}

void readIfNeeded() {
  if ((ADCSRA & (1<<ADSC)) == 0) {
    if (readingCv1) {
      cvIn1 = ADC;
      readingCv1 = false;
      ADMUX = (ADMUX & 0xF0) | (1 & 0x0F);
    } else {
      cvIn2 = ADC;
      readingCv1 = true;
      ADMUX = (ADMUX & 0xF0) | (0 & 0x0F);
    }
    //single conversion mode
    ADCSRA |= (1<<ADSC);
  }
}

}
