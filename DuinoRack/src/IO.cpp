#include <Arduino.h>
#include "IO.h"

namespace IO {
uint16_t cvIn1 = 0;
uint16_t cvIn2 = 0;
bool readingCv1 = true;

int16_t cvIn1_0V = 700;
int16_t cvIn1_4V = 300;
int16_t cvIn2_0V = 700;
int16_t cvIn2_4V = 300;

int16_t getCV1() {                                // 2.7V (-8V)      0V (0V)
  Serial.println(cvIn1);                          // 0               543
  Serial.println(int32_t(cvIn1_0V - cvIn1));      // 700             157
  return ((int32_t)(cvIn1_0V - cvIn1)) * 4000 / (cvIn1_0V - cvIn1_4V);
}

int16_t getCV2() {
  return ((int32_t)(cvIn2_0V - cvIn2)) * 4000 / (cvIn2_0V - cvIn2_4V);
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
