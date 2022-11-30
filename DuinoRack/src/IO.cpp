#include <Arduino.h>
#include "IO.h"

namespace IO {
uint16_t cvIn1 = 0;
uint16_t cvIn2 = 0;
bool readingCv1 = true;

int16_t cvIn1_0V = 545;
int16_t cvIn1_4V = 276;
int16_t cvIn2_0V = 700;
int16_t cvIn2_4V = 300;

int16_t cvOut1_0V = 1369;
int16_t cvOut1_8V = 3868;
int16_t cvOut2_0V = 1369;
int16_t cvOut2_8V = 3868;

int16_t getCV1In() {
  return (int32_t(cvIn1_0V) - cvIn1) * 4000 / (cvIn1_0V - cvIn1_4V);
}

int16_t getCV2In() {
  return (int32_t(cvIn2_0V) - cvIn2) * 4000 / (cvIn2_0V - cvIn2_4V);
}

  // TODO rewrite into Q16N16 and optimize.
uint16_t calcCV1Out(int16_t mV) {
  return (int32_t(mV) * (cvOut1_8V - cvOut1_0V) / 1000 / 8) + cvOut1_0V;
}

uint16_t calcCV2Out(int16_t mV) {
  return (int32_t(mV) * (cvOut2_8V - cvOut2_0V) / 1000 / 8) + cvOut2_0V;
}

  bool getGate1In() {
    return (PINB & (1 << 0)) != 0;
  }

  bool getGate2In() {
    return (PINB & (1 << 1)) != 0;
  }

  void setGate1Out(bool on) {
    if (on) {
      PORTC |= (1 << 2);
    } else {
      PORTC &= ~(1 << 2);
    }
  }

  void setGate2Out(uint8_t on) {
    if (on) {
      PORTC |= (1 << 3);
    } else {
      PORTC &= ~(1 << 3);
    }
  }

void setup() {
  // Select Vref=AVcc
  ADMUX |= (1<<REFS0);
  //set prescaller to 128 (at 16MHz, is 125kHz)and enable ADC
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
  // Start with channel 0 (CV in 1)
  ADMUX = (ADMUX & 0xF0) | (0 & 0x0F);
  //single conversion mode
  ADCSRA |= (1<<ADSC);

  pinMode(8, INPUT); // PB0, Gate IN 1
  pinMode(9, INPUT); // PB1, Gate IN 2

  pinMode(16, OUTPUT); // PC2, Gate OUT 1
  pinMode(17, OUTPUT); // PC3, Gate OUT 2
}

void readIfNeeded() {
  if ((ADCSRA & (1<<ADSC)) == 0) {
    if (readingCv1) {
      cvIn1 = ADC;
      //Serial.println("A0");
      //Serial.println(cvIn1);
      readingCv1 = false;
      ADMUX = (ADMUX & 0xF0) | (1 & 0x0F);
    } else {
      cvIn2 = ADC;
      //Serial.println("A1");
      //Serial.println(cvIn1);
      readingCv1 = true;
      ADMUX = (ADMUX & 0xF0) | (0 & 0x0F);
    }
    //single conversion mode
    ADCSRA |= (1<<ADSC);
  }
}

}
