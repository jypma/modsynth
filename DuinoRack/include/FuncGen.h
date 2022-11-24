#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"

namespace FuncGen {

// LOOKUP TABLE SINE
constexpr uint16_t TABLE_SIZE = 360;
constexpr uint32_t sinePosScale = 65536;
constexpr uint32_t sinePosMod = uint32_t(TABLE_SIZE) * sinePosScale;
int16_t sine[TABLE_SIZE+1];
uint32_t sinePos = 0;
bool tableReady = false;
uint16_t recalc = 0;
uint32_t increment;

const char title[] PROGMEM = "FuncGen   ";

void draw() {
  drawTextPgm(0, 16, clear);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

void adjust(int8_t d) {
  if (tableReady) return;

  // fill table with sinus values for fast lookup
  for (uint16_t i = 0; i < TABLE_SIZE + 1; i++)
  {
    sine[i] = round(4000 * sin(i * PI / 180));
  }
  tableReady = true;
}

void fillBuffer(OutputFrame *buf) {
  if (recalc == 0) {
    // TODO if this works, refactor getCV1In to return Q16n16 instead for a bit of extra speed?
    uint16_t in = IO::getCV1In();
    //Q16n16 note = float_to_Q16n16(float(in) / float(1000.0));
    Serial.println("---");
    Serial.println(in);
    Q16n16 note = (((uint32_t(in) / 1000) << 16) | ((uint32_t(in % 1000) << 16) / 1000)) * 12;
    Serial.println(note >> 16);
    Q16n16 freq = Q16n16_mtof(note);
    Serial.println(freq);
    increment = (((((uint32_t(freq) >> 8) * TABLE_SIZE) >> 8) * sinePosScale) ) / 8000;
    Serial.println(increment);
    //OLD auto freq = pow(10, in / 1000.0);
    //OLD uint32_t increment = uint32_t(freq * TABLE_SIZE) * sinePosScale / 8000; // divide by 8kHz, but x1024
    recalc = 100;
  } else {
    recalc--;
  }

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    PORTC |= (1 << 2);
    sinePos = (sinePos + increment);
    if (sinePos > sinePosMod) {
      sinePos -= sinePosMod;
    }
    PORTC &= ~(1 << 2);
    buf->cvA = IO::calcCV1Out(sine[sinePos >> 16]);
    PORTC |= (1 << 2);
    buf->cvB = buf->cvA;
    PORTC &= ~(1 << 2);
    buf++;
    PORTC |= (1 << 2);
    PORTC &= ~(1 << 2);
  }

}

constexpr Module module = {
  title,
  0,
  &draw,
  &adjust,
  &fillBuffer
};

} // FuncGen
