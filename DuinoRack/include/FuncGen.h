#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"

namespace FuncGen {

// LOOKUP TABLE SINE
constexpr uint16_t TABLE_SIZE = 360;
constexpr uint8_t sinePosScaleBits = 10;
constexpr uint32_t sinePosScale = 1024;
constexpr uint32_t sinePosMod = uint32_t(TABLE_SIZE) * sinePosScale;
constexpr Q16n16 MAX_NOTE = 5242880; // Note 80, ~800Hz

int16_t sine[TABLE_SIZE+1];
uint32_t sinePos = 0;
bool tableReady = false;
uint16_t recalc = 0;
uint32_t increment;
uint8_t noteIdx = 0;

const char title[] PROGMEM = "FuncGen   ";
const char note[] PROGMEM = "Note: ";

void draw() {
  drawTextPgm(0, 16, note);
  drawDecimal(40, 16, noteIdx);
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
  PORTC |= (1 << 2);

  if (recalc == 0) {
    // TODO if this works, refactor getCV1In to return Q16n16 instead for a bit of extra speed?
    uint16_t in = IO::getCV1In();
    Q16n16 note = (((uint32_t(in) / 1000) << 16) | ((uint32_t(in % 1000) << 16) / 1000)) * 12;
    if (note > MAX_NOTE) {
      note = MAX_NOTE;
    }
    noteIdx = (note >> 16);
    Q16n16 freq = Q16n16_mtof(note);
    increment = (((((uint32_t(freq) >> 8) * TABLE_SIZE) >> 8) << sinePosScaleBits) ) / 8000;
    recalc = 1;
  } else {
    recalc--;
  }

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    sinePos = (sinePos + increment);
    if (sinePos > sinePosMod) {
      sinePos -= sinePosMod;
    }
    buf->cvA = IO::calcCV1Out(sine[sinePos >> sinePosScaleBits]);
    buf->cvB = buf->cvA;
    buf++;
  }

  PORTC &= ~(1 << 2);
}

constexpr Module module = {
  title,
  0,
  &draw,
  &adjust,
  &fillBuffer
};

} // FuncGen
