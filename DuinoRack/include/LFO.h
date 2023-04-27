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

  constexpr uint16_t TABLE_SIZE = Waves::TABLE_SIZE;
  constexpr uint16_t Q_TABLE_SIZE = TABLE_SIZE / 4;
  constexpr uint16_t Q3_TABLE_SIZE = Q_TABLE_SIZE * 3;

  constexpr uint8_t sinePosScaleBits = Waves::posScaleBits;

constexpr uint32_t sinePosScale = (uint32_t(1) << sinePosScaleBits);
constexpr uint32_t sinePosFractionMask = (uint32_t(1) << sinePosScaleBits) - 1;
constexpr uint32_t sinePosMod = uint32_t(TABLE_SIZE) * sinePosScale;

constexpr Q16n16 MAX_NOTE = 5242880; // Note 80, ~800H
constexpr Q16n16 MIN_NOTE = 65536;   // Note 1

enum Wave: uint8_t { Sine = 0, Triangle = 1, SawUp = 2, SawDown = 3, Square = 4 };

const char sine_t[] PROGMEM = "Sin";
const char triangle_t[] PROGMEM = "Tri";
const char saw_up_t[] PROGMEM = "Saw";
const char saw_down_t[] PROGMEM = "iSw";
const char square_t[] PROGMEM = "Sqr";
const char *waveTitles[] = {sine_t, triangle_t, saw_up_t, saw_down_t, square_t};
constexpr uint8_t N_WAVES = 5;

const uint8_t factorNom[] = { 4, 3, 2, 1, 1, 1, 1 };
const uint8_t factorDen[] = { 1, 1, 1, 1, 2, 3, 4 };
constexpr uint8_t FACTOR_ONE = 3; // The factor of 1/1, i.e. normal BPM
constexpr uint8_t MAX_FACTOR = 6;

uint32_t mainPos;
uint32_t increment;

struct Shape {
  uint32_t tablePos = 0;
  uint32_t increment;
  uint8_t factor = FACTOR_ONE;
  Wave wave = Sine;

  void recalc(uint32_t mainIncrement) {
    increment = mainIncrement / factorNom[factor] * factorDen[factor];
  }

  void resetPhase() {
    constexpr auto OK_TO_RESET_MIN = uint32_t(10) << sinePosScaleBits;
    constexpr auto OK_TO_RESET_MAX = uint32_t(240) << sinePosScaleBits;

    if (factor >= FACTOR_ONE) {
      tablePos = 0;
    } else if (tablePos < OK_TO_RESET_MIN || tablePos > OK_TO_RESET_MAX) {
      tablePos = 0;
    }
  }

  void performStep() {
    tablePos += increment;
    if (tablePos > sinePosMod) {
      tablePos -= sinePosMod;
    }
  }

  int16_t getTableValue(){
    switch (wave) {
      case Sine: return Waves::Sine::get(tablePos);
      case Triangle: return Waves::Triangle::get(tablePos);
      case SawUp: return Waves::SawUp::get(tablePos);
      case SawDown: return Waves::SawDown::get(tablePos);
      case Square: return Waves::Square::get(tablePos);
    }
    return 0;
  }
};

constexpr uint8_t N_SHAPES = 4;
constexpr uint8_t CONTROLS_PER_SHAPE = 4;
Shape shapes[N_SHAPES];

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
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].recalc(increment);
  }
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
  drawSelected(0, 16, 1);
  drawTextPgm(7, 16, bpm_t);
  drawDecimal(40, 16, bpm);

  drawTextPgm(0, 24, wave1_t);
  drawTextPgm(0, 32, wave2_t);
  drawTextPgm(0, 40, wave3_t);
  drawTextPgm(0, 48, wave4_t);
  for (uint8_t s = 0; s < N_SHAPES; s++) {
    uint8_t controlIdx = s * 4 + 2;
    uint8_t y = s * 8 + 24;

    drawSelected(12, y, controlIdx);
    drawTextPgm(18, y, waveTitles[shapes[s].wave]);
    drawSelected(36, y, controlIdx + 1);
    char str[4];
    formatFactor(str, shapes[s].factor);
    drawText(42, y, str);
    drawSelected(72, y, controlIdx + 2);
    drawSelected(96, y, controlIdx + 3);
  }
}

void adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      bpm = applyDelta<uint16_t>(bpm, d, 1, 1000);
      recalc();
      break;
    default:
      uint8_t shapeIdx = (currentControlIdx - 2) / CONTROLS_PER_SHAPE;
      uint8_t controlIdx = (currentControlIdx - 2) % CONTROLS_PER_SHAPE;

      switch(controlIdx) {
        case 0:
          shapes[shapeIdx].wave = Wave(applyDelta<uint8_t>(shapes[shapeIdx].wave, d, 0, N_WAVES - 1));
          break;
        case 1:
          shapes[shapeIdx].factor = applyDelta<uint32_t>(shapes[shapeIdx].factor, d, 0, MAX_FACTOR);
          recalc();
          break;
      }
  }
}

void resetPhase() {
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].resetPhase();
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

void fillBuffer(OutputFrame *buf) {
  checkGate();

  // only use one value for the entire buffer. 750Hz is perfectly acceptable for an LFO.
  mainPos += increment;
  if (mainPos > sinePosMod) {
    mainPos -= sinePosMod;
    resetPhase();
  } else {
    for (uint8_t i = 0; i < N_SHAPES; i++) {
      shapes[i].performStep();
    }
  }
  const uint16_t value1 = IO::calcCV1Out(shapes[0].getTableValue());
  const uint16_t value2 = IO::calcCV2Out(shapes[1].getTableValue());
  const uint16_t value3 = IO::calcGate1Out(shapes[2].getTableValue());
  const uint16_t value4 = IO::calcGate2Out(shapes[3].getTableValue());

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
  1 + N_SHAPES * CONTROLS_PER_SHAPE,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  NULL
};

} // FuncGen
