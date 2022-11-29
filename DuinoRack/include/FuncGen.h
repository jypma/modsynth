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
enum Wave: uint8_t { Sine = 0, Triangle = 1 };
Wave wave = Sine;

const char title[] PROGMEM = "FuncGen   ";
const char note[] PROGMEM = "Note: ";
const char wave_t[] PROGMEM = "  Wave: ";
const char sine_t[] PROGMEM = "Sine    ";
const char triangle_t[] PROGMEM = "Triangle";

void draw() {
  drawTextPgm(0, 16, note);
  drawDecimal(40, 16, noteIdx);
  drawTextPgm(0, 24, wave_t);
  switch(currentControlIdx) {
  case 1: drawText(0, 24, ">"); break;
  }
  switch (wave) {
  case Sine: drawTextPgm(52, 24, sine_t); break;
  case Triangle: drawTextPgm(52, 24, triangle_t); break;
  }

  drawTextPgm(0, 32, clear);
}

  inline uint16_t getTriangle(int32_t i) {
    if (i < 90) {
      return (i * 4000 / 90);
    } else if (i < 270) {
      return (int32_t(90) - (i - 90)) * 4000 / 90;
    } else {
      return ((i - 270) - 90) * 4000 / 90;
    }
  }

void adjust(int8_t d) {
  switch(currentControlIdx) {
  case 1:
    wave = Wave((wave + 1) % 2);
    tableReady = false;
    break;
  }

  if (tableReady) return;

  for (uint16_t i = 0; i < TABLE_SIZE + 1; i++)
  {
    switch(wave) {
    case Sine: sine[i] = round(4000 * sin(i * PI / 180)); break;
    case Triangle: sine[i] = getTriangle(i); break;
    }
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
  1,
  &draw,
  &adjust,
  &fillBuffer
};

} // FuncGen
