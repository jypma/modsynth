#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Waves.hpp"

#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "mozzi_pgmspace.h"

namespace FuncGen {

  // LOOKUP TABLE SINE
  constexpr uint16_t TABLE_SIZE = 256;
  constexpr uint16_t Q_TABLE_SIZE = 64;
  constexpr uint16_t Q3_TABLE_SIZE = 192;

  constexpr uint8_t sinePosScaleBits = 10;
  constexpr uint32_t sinePosScale = 1024;
  constexpr uint32_t sinePosMod = uint32_t(TABLE_SIZE) * sinePosScale;
  constexpr Q16n16 MAX_NOTE = 5242880; // Note 80, ~800Hz
  constexpr Q16n16 MIN_NOTE = 65536; // Note 1

  uint32_t sinePos = 0;
  uint16_t recalc = 0;
  uint32_t increment;
  uint8_t noteIdx = 0;
enum Wave: uint8_t { Sine = 0, Triangle = 1, Saw = 2, Square = 3 };
  Wave wave = Sine;

  const char title[] PROGMEM = "FuncGen   ";
  const char note[] PROGMEM = "Note: ";
  const char wave_t[] PROGMEM = "Wave: ";
  const char sine_t[] PROGMEM = "Sine    ";
  const char triangle_t[] PROGMEM = "Triangle";
  const char square_t[] PROGMEM = "Square  ";
  const char saw_t[] PROGMEM = "Saw     ";

  void draw() {
    drawTextPgm(0, 16, note);
    drawDecimal(40, 16, noteIdx);
    drawTextPgm(16, 24, wave_t);
    switch(currentControlIdx) {
    case 1: drawText(0, 24, ">"); break;
    default: drawText(0, 24, " ");
    }
    switch (wave) {
    case Sine: drawTextPgm(52, 24, sine_t); break;
    case Triangle: drawTextPgm(52, 24, triangle_t); break;
    case Saw: drawTextPgm(52, 24, saw_t); break;
    case Square: drawTextPgm(52, 24, square_t); break;
    }

    drawTextPgm(0, 32, clear);
  }

  void adjust(int8_t d) {
    switch(currentControlIdx) {
    case 1:
      wave = Wave((wave + d + 4) % 4);
      break;
    }
  }

  void start() {
  }

  void stop() {

  }

int16_t getWaveValue() {
  const uint8_t p = sinePos >> sinePosScaleBits;
  switch (wave) {
    case Sine: return Waves::Sine::get(p);
    case Triangle: return Waves::Triangle::get(p);
    case Saw: return Waves::SawDown::get(p);
    case Square: return Waves::Square::get(p);
  }
  return 0;
}

  void fillBuffer(OutputFrame *buf) {
    if (recalc == 0) {
      // TODO if this works, refactor getCV1In to return Q16n16 instead for a bit of extra speed?
      int16_t in = IO::getCV1In();
      if (in < 0) {
        in = 0;
      }
      Q16n16 note = (((uint32_t(in) / 1000) << 16) | ((uint32_t(in % 1000) << 16) / 1000)) * 12;
      if (note > MAX_NOTE) {
        note = MAX_NOTE;
      } else if (note < MIN_NOTE) {
        note = MIN_NOTE;
      }
      noteIdx = (note >> 16);
      Q16n16 freq = Q16n16_mtof(note);
      increment = (((((uint32_t(freq) >> 8) * TABLE_SIZE) >> 8) << sinePosScaleBits) ) / SAMPLERATE;
      recalc = 1;
    } else {
      recalc--;
    }

    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      sinePos = (sinePos + increment);
      if (sinePos > sinePosMod) {
        sinePos -= sinePosMod;
      }
      // We don't use calibration for channels 2, 3 and 4. We might remove them from the VCO anyway
      // (better to have a single more fun voice).
      const int16_t rawValue = IO::calcCV1Out(getWaveValue());
      buf->cv1 = rawValue;
      buf->cv2 = rawValue;
      buf->gate1 = rawValue >> 2;
      buf->gate2 = buf->gate1;
      buf++;
    }
  }

  constexpr Module module = {
    title,
    1,
    &draw,
    &start,
    &stop,
    &adjust,
    &fillBuffer,
    NULL
  };

} // FuncGen
