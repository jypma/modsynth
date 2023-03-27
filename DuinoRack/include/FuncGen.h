#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"

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

  uint16_t sine[TABLE_SIZE+1];
  uint32_t sinePos = 0;
  bool tableReady = false;
  uint16_t recalc = 0;
  uint32_t increment;
  uint8_t noteIdx = 0;
  enum Wave: uint8_t { Sine = 0, Triangle = 1 };
  Wave wave = Sine;

  const char title[] PROGMEM = "FuncGen   ";
  const char note[] PROGMEM = "Note: ";
  const char wave_t[] PROGMEM = "Wave: ";
  const char sine_t[] PROGMEM = "Sine    ";
  const char triangle_t[] PROGMEM = "Triangle";

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
    }

    drawTextPgm(0, 32, clear);
  }

  inline uint16_t getTriangle(int32_t i) {
    if (i < Q_TABLE_SIZE) {
      return (i * 4000 / Q_TABLE_SIZE);
    } else if (i < Q3_TABLE_SIZE) {
      return (int32_t(Q_TABLE_SIZE) - (i - Q_TABLE_SIZE)) * 4000 / Q_TABLE_SIZE;
    } else {
      return ((i - Q3_TABLE_SIZE) - Q_TABLE_SIZE) * 4000 / Q_TABLE_SIZE;
    }
  }

  void prepareWave() {
    if (tableReady) return;

    for (uint16_t i = 0; i < TABLE_SIZE + 1; i++)
    {
      switch(wave) {
      case Sine:
        sine[i] = IO::calcCV1Out(round(4000 * sin(i * PI / (TABLE_SIZE / 2.0))));
        break;
      case Triangle:
        sine[i] = IO::calcCV1Out(getTriangle(i));
        break;
      }
    }
    tableReady = true;
  }
  
  void adjust(int8_t d) {
    switch(currentControlIdx) {
    case 1:
      wave = Wave((wave + 1) % 2);
      tableReady = false;
      break;
    }
    prepareWave();
  }

  void start() {
    prepareWave();
  }

  void stop() {

  }

  /*
  uint16_t bend(uint16_t phase, Q16n16 factor) {
    Q16n16 crossOver16 = Q16n16_to_Q16n0(90 * factor);
    uint16_t crossOver = Q16n16_to_Q16n0(crossOver16);

    if (phase <= crossOver) {
      return Q16n16_to_Q16n0((Q16n0_to_Q16n16(phase) * 90) / crossOver);
    } else if (phase <= (360 - crossOver)) {
      return Q16n16_to_Q16n0(90 + (Q16n0_to_Q16n16(phase) - crossOver16) * 180 / (360 - 2 * crossOver16));
    } else {
      return Q16n16_to_Q16n0(270 + (Q16n0_to_Q16n16(phase) - (360 - 2 * crossOver16)) * 90 / crossOver16);
    }
  }
  */

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
      buf->cv1 = sine[sinePos >> sinePosScaleBits];
      // TODO we need to be able to calculate enough values here
      // Otherwise, reduce table to 8-bit values and 128 entries.
      buf->cv2 = buf->cv1;
      buf->gate1 = buf->cv1 >> 2; // quick hack for 12 to 10 bits
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
    &fillBuffer
  };

} // FuncGen
