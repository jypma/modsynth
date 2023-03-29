#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <tables/cos256_int8.h>
#include "Arduino.h"
#include "tables/sin256.h"
#include "tables/triangle256.h"
#include "tables/saw_up256.h"
#include "tables/saw_down256.h"

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

const char sine_t[] PROGMEM = "Sine    ";
const char triangle_t[] PROGMEM = "Triangle";
const char saw_up_t[] PROGMEM = "Saw Up   ";
const char saw_down_t[] PROGMEM = "Saw Down ";
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
const uint16_t *table1 = SIN_DATA;
Wave wave1 = Sine;

uint32_t tablePos2 = 0;
uint32_t increment2;
uint8_t factor2 = FACTOR_ONE;
const uint16_t *table2 = SIN_DATA;
Wave wave2 = Sine;

uint32_t tablePos3 = 0;
uint32_t increment3;
uint8_t factor3 = FACTOR_ONE;
const uint16_t *table3 = SIN_DATA;
Wave wave3 = Sine;

uint32_t tablePos4 = 0;
uint32_t increment4;
uint8_t factor4 = FACTOR_ONE;
const uint16_t *table4 = SIN_DATA;
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
  drawText(16, 24, (currentControlIdx == 2) ? ">" : " ");
  drawText(80, 24, (currentControlIdx == 3) ? ">" : " ");
  drawText(16, 32, (currentControlIdx == 4) ? ">" : " ");
  drawText(80, 32, (currentControlIdx == 5) ? ">" : " ");
  drawText(16, 40, (currentControlIdx == 6) ? ">" : " ");
  drawText(80, 40, (currentControlIdx == 7) ? ">" : " ");
  drawText(16, 48, (currentControlIdx == 8) ? ">" : " ");
  drawText(80, 48, (currentControlIdx == 9) ? ">" : " ");

  drawTextPgm(24, 24, waveTitles[wave1]);
  drawTextPgm(24, 32, waveTitles[wave2]);
  drawTextPgm(24, 40, waveTitles[wave3]);
  drawTextPgm(24, 48, waveTitles[wave4]);
  char str[4];
  formatFactor(str, factor1);
  drawText(88, 24, str);
  formatFactor(str, factor2);
  drawText(88, 32, str);
  formatFactor(str, factor3);
  drawText(88, 40, str);
  formatFactor(str, factor4);
  drawText(88, 48, str);
}

void adjustWave(Wave *wave, const uint16_t **table, int8_t d) {
  *wave = Wave((*wave + 4 + d) % 4);
  switch (*wave) {
    case Sine: *table = SIN_DATA; break;
    case Triangle: *table = TRIANGLE_DATA; break;
    case SawUp: *table = SAW_UP_DATA; break;
    case SawDown: *table = SAW_DOWN_DATA; break;
  }
}

void adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      bpm = constrain(bpm + d, 1, 1000);
      recalc();
      break;
    case 2:
      adjustWave(&wave1, &table1, d);
      break;
    case 3:
      factor1 = constrain(factor1 + d, 0, MAX_FACTOR);
      recalc();
      break;
    case 4:
      adjustWave(&wave2, &table2, d);
      break;
    case 5:
      factor2 = constrain(factor2 + d, 0, MAX_FACTOR);
      recalc();
      break;
    case 6:
      adjustWave(&wave3, &table3, d);
      break;
    case 7:
      factor3 = constrain(factor3 + d, 0, MAX_FACTOR);
      recalc();
      break;
    case 8:
      adjustWave(&wave4, &table4, d);
      break;
    case 9:
      factor4 = constrain(factor4 + d, 0, MAX_FACTOR);
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
      auto time = micros();
      if (lastGateTime != NO_TIME) {
        // Reset the phase on incoming pulse
        mainPos = 0;
        resetPhase();

        auto delayUs = time - lastGateTime;
        uint16_t newbpm = uint32_t(1000000 * 60) / delayUs;
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

uint16_t getTableValue(const uint16_t *table, uint32_t pos) {
  uint8_t idx1 = pos >> sinePosScaleBits;
  // Let idx wrap around since table is 256 entries
  uint8_t idx2 = idx1 + 1;

  // TODO cache v1, v2, delta for more performance, while idx doesn't change.
  uint16_t v1 = FLASH_OR_RAM_READ(table + idx1);
  uint16_t v2 = FLASH_OR_RAM_READ(table + idx2);
  int16_t delta = (v2 > v1) ? v2 - v1 : -(v1 - v2);
  int16_t fraction = (int32_t(delta) * (pos & sinePosFractionMask)) >> sinePosScaleBits;

  return v1 + fraction;
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
  const uint16_t value1 = getTableValue(table1, tablePos1);
  const uint16_t value2 = getTableValue(table2, tablePos2);
  const uint16_t value3 = getTableValue(table3, tablePos3);
  const uint16_t value4 = getTableValue(table4, tablePos4);

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = value1;
    buf->cv2 = value2;
    buf->gate1 = value3 >> 2; // Gate outputs are 10-bit, not 12 bit like CV outputs
    buf->gate2 = value4 >> 2;
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
