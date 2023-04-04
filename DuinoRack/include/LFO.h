#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Arduino.h"
#include "tables/sin256.h"
#include "Waves.hpp"

#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "mozzi_pgmspace.h"

namespace LFO {

constexpr uint16_t TABLE_SIZE = 256;
constexpr uint16_t Q_TABLE_SIZE = 64;
constexpr uint16_t Q3_TABLE_SIZE = 192;

constexpr uint8_t sinePosScaleBits = 14;

constexpr uint32_t sinePosScale = (uint32_t(1) << sinePosScaleBits);
constexpr uint32_t sinePosFractionMask = (uint32_t(1) << sinePosScaleBits) - 1;
constexpr uint32_t sinePosMod = uint32_t(TABLE_SIZE) * sinePosScale;

constexpr Q16n16 MAX_NOTE = 5242880; // Note 80, ~800Hz
constexpr Q16n16 MIN_NOTE = 65536;   // Note 1

enum Wave: uint8_t { Sine = 0, Triangle = 1, SawUp = 2, SawDown = 3 };

const char sine_t[] PROGMEM = "Sin";
const char triangle_t[] PROGMEM = "Tri";
const char saw_up_t[] PROGMEM = "Saw";
const char saw_down_t[] PROGMEM = "iSw";
const char * waveTitles[] = { sine_t, triangle_t, saw_up_t, saw_down_t };

const uint8_t factorNom[] = { 4, 3, 2, 1, 1, 1, 1 };
const uint8_t factorDen[] = { 1, 1, 1, 1, 2, 3, 4 };
constexpr uint8_t FACTOR_ONE = 3; // The factor of 1/1, i.e. normal BPM
constexpr uint8_t MAX_FACTOR = 6;

uint32_t mainPos;
uint32_t increment;

uint32_t tablePos1 = 0;
uint32_t increment1;
uint8_t factor1 = FACTOR_ONE;
Wave wave1 = Sine;
uint8_t bend1 = 0;

uint32_t tablePos2 = 0;
uint32_t increment2;
uint8_t factor2 = FACTOR_ONE;
Wave wave2 = Sine;

uint32_t tablePos3 = 0;
uint32_t increment3;
uint8_t factor3 = FACTOR_ONE;
Wave wave3 = Sine;

uint32_t tablePos4 = 0;
uint32_t increment4;
uint8_t factor4 = FACTOR_ONE;
Wave wave4 = Sine;

uint16_t bpm = 120;

const char title[] PROGMEM = "LFO   ";
const char bpm_t[] PROGMEM = "BPM:  ";
const char wave1_t[] PROGMEM = "A:";
const char wave2_t[] PROGMEM = "B:";
const char wave3_t[] PROGMEM = "1:";
const char wave4_t[] PROGMEM = "2:";

bool lastGate = false;
constexpr uint32_t NO_TIME = 0xFFFFFFFF;
uint32_t lastGateTime = NO_TIME;

void recalc() {
  // Multiply by OUTBUFSIZE, since we increment only once per buffer
  increment = (((uint32_t(bpm) * TABLE_SIZE) << sinePosScaleBits)) / 60 * OUTBUFSIZE / SAMPLERATE;
  increment1 = increment / factorNom[factor1] * factorDen[factor1];
  increment2 = increment / factorNom[factor2] * factorDen[factor2];
  increment3 = increment / factorNom[factor3] * factorDen[factor3];
  increment4 = increment / factorNom[factor4] * factorDen[factor4];
}

void formatFactor(char *str, uint8_t factor) {
  str[3] = 0;
  if (factorDen[factor] == 1) {
    str[0] = '0' + factorNom[factor];
    str[1] = 'x';
    str[2] = ' ';
  } else {
    str[0] = '0' + factorNom[factor];
    str[1] = '/';
    str[2] = '0' + factorDen[factor];
  }
}

void draw() {
  drawTextPgm(7, 16, bpm_t);
  drawDecimal(40, 16, bpm);

  drawTextPgm(0, 24, wave1_t);
  drawTextPgm(0, 32, wave2_t);
  drawTextPgm(0, 40, wave3_t);
  drawTextPgm(0, 48, wave4_t);
  drawText(0, 16, (currentControlIdx == 1) ? ">" : " ");
  drawText(12, 24, (currentControlIdx == 2) ? ">" : " ");
  drawText(36, 24, (currentControlIdx == 3) ? ">" : " ");
  drawText(72, 24, (currentControlIdx == 4) ? ">" : " ");
  drawDecimal(78, 24, bend1);
  drawText(96, 24, (currentControlIdx == 5) ? ">" : " ");

  drawText(12, 32, (currentControlIdx == 6) ? ">" : " ");
  drawText(36, 32, (currentControlIdx == 7) ? ">" : " ");
  drawText(12, 40, (currentControlIdx == 8) ? ">" : " ");
  drawText(36, 40, (currentControlIdx == 9) ? ">" : " ");
  drawText(12, 48, (currentControlIdx == 10) ? ">" : " ");
  drawText(36, 48, (currentControlIdx == 11) ? ">" : " ");

  drawTextPgm(18, 24, waveTitles[wave1]);
  drawTextPgm(18, 32, waveTitles[wave2]);
  drawTextPgm(18, 40, waveTitles[wave3]);
  drawTextPgm(18, 48, waveTitles[wave4]);
  char str[4];
  formatFactor(str, factor1);
  drawText(42, 24, str);
  formatFactor(str, factor2);
  drawText(42, 32, str);
  formatFactor(str, factor3);
  drawText(42, 40, str);
  formatFactor(str, factor4);
  drawText(42, 48, str);
}

void adjustWave(Wave *wave, int8_t d) {
  *wave = Wave((*wave + 4 + d) % 4);
}

void adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      bpm = applyDelta<uint16_t>(bpm, d, 1, 1000);
      recalc();
      break;
    case 2:
      adjustWave(&wave1, d);
      break;
    case 3:
      factor1 = applyDelta<uint32_t>(factor1, d, 0, MAX_FACTOR);
      recalc();
      break;
    case 4:
      bend1 = applyDelta<uint8_t>(bend1, d, 0, 255);
      break;
    case 5:
      break;
    case 6:
      adjustWave(&wave2, d);
      break;
    case 7:
      factor2 = applyDelta<uint32_t>(factor2, d, 0, MAX_FACTOR);
      recalc();
      break;
    case 8:
      adjustWave(&wave3, d);
      break;
    case 9:
      factor3 = applyDelta<uint32_t>(factor3, d, 0, MAX_FACTOR);
      recalc();
      break;
    case 10:
      adjustWave(&wave4, d);
      break;
    case 11:
      factor4 = applyDelta<uint32_t>(factor4, d, 0, MAX_FACTOR);
      recalc();
      break;
  }
}

void resetPhase() {
  constexpr auto OK_TO_RESET_MIN = uint32_t(10) << sinePosScaleBits;
  constexpr auto OK_TO_RESET_MAX = uint32_t(240) << sinePosScaleBits;

  if (factor1 >= FACTOR_ONE) {
    tablePos1 = 0;
  } else if (tablePos1 < OK_TO_RESET_MIN || tablePos1 > OK_TO_RESET_MAX) {
    tablePos1 = 0;
  }
  if (factor2 >= FACTOR_ONE) {
    tablePos2 = 0;
  } else if (tablePos2 < OK_TO_RESET_MIN || tablePos2 > OK_TO_RESET_MAX) {
    tablePos2 = 0;
  }
  if (factor3 >= FACTOR_ONE) {
    tablePos3 = 0;
  } else if (tablePos3 < OK_TO_RESET_MIN || tablePos3 > OK_TO_RESET_MAX) {
    tablePos3 = 0;
  }
  if (factor4 >= FACTOR_ONE) {
    tablePos4 = 0;
  } else if (tablePos4 < OK_TO_RESET_MIN || tablePos4 > OK_TO_RESET_MAX) {
    tablePos4 = 0;
  }
}

void checkGate() {
  bool gate = IO::getGate1In();
  if (gate != lastGate) {
    lastGate = gate;
    if (gate) {
      auto time = millis(); // millis is significantly faster than micros(), which has a long division.
      if (lastGateTime != NO_TIME) {
        // Reset the phase on incoming pulse
        mainPos = 0;
        resetPhase();

        auto delayMs = time - lastGateTime;
        uint16_t newbpm = uint32_t(1000) * 60 / delayMs;
        if (newbpm != bpm && newbpm > 30) {
          bpm = newbpm;
          recalc();
        }
      }
      lastGateTime = time;
    }
  }
}

void start() {
  recalc();
  lastGate = IO::getGate1In();
  lastGateTime = NO_TIME;
}

void stop() {}

int16_t getTableValue(Wave wave, uint32_t pos){
  switch (wave) {
    case Sine: return Waves::Sine::get(pos);
    case Triangle: return Waves::Triangle::get(pos);
    case SawUp: return Waves::SawUp::get(pos);
    default: return Waves::SawDown::get(pos);
  }
}

void fillBuffer(OutputFrame *buf) {
  checkGate();

  // only use one value for the entire buffer. 750Hz is perfectly acceptable for an LFO.
  mainPos += increment;
  if (mainPos > sinePosMod) {
    mainPos -= sinePosMod;
    resetPhase();
  } else {
    tablePos1 += increment1;
    tablePos2 += increment2;
    tablePos3 += increment3;
    tablePos4 += increment4;
  }
  const uint16_t value1 = IO::calcCV1Out(getTableValue(wave1, tablePos1));
  const uint16_t value2 = IO::calcCV2Out(getTableValue(wave2, tablePos2));
  const uint16_t value3 = IO::calcGate1Out(getTableValue(wave3, tablePos3));
  const uint16_t value4 = IO::calcGate2Out(getTableValue(wave4, tablePos4));

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = value1;
    buf->cv2 = value2;
    buf->gate1 = value3;
    buf->gate2 = value4;
    buf++;
  }
}

constexpr Module module = {
  title,
  9,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer
};

} // FuncGen
