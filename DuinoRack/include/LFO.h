#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Arduino.h"
#include "Debug.hpp"
#include "tables/sin256.h"
#include "Waves.hpp"
#include "Storage.hpp"

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

enum Range: uint8_t { Negative4, Bipolar4, Positive4, Positive8 };
constexpr uint8_t N_RANGE = 4;
const char neg4_t[] PROGMEM = "-4 0V";
const char bip4_t[] PROGMEM = "+/-4V";
const char pos4_t[] PROGMEM = "0 +4V";
const char pos8_t[] PROGMEM = "0 +8V";
const char *rangeTitles[] = {neg4_t, bip4_t, pos4_t, pos8_t};

uint32_t mainPos;
uint32_t increment;

struct Shape {
  uint32_t tablePos = 0;
  //uint32_t incrementA; // For period one
  //uint32_t incrementB; // For period two
  uint32_t increment;  // For further periods
  int8_t swing = 0;
  int8_t period = 0; // TODO: Have an actual reset input reset this
  int8_t swingPeriods = 2;
  // slop: 7 random values around +/- 49%(?), add them up, 8th is to bring it back in sync.
  // Always resync after 8 periods.
  // Directly affects tablePos (unlike swing)
  uint8_t factor = FACTOR_ONE;
  Wave wave = Sine;
  uint8_t phase = 0; // 0..255 as one period
  Range range = Bipolar4;

  uint16_t save(uint16_t addr) {
    addr = Storage::write(addr, wave);
    addr = Storage::write(addr, factorNom[factor]);
    addr = Storage::write(addr, factorDen[factor]);
    addr = Storage::write(addr, phase);
    addr = Storage::write(addr, range);

    return addr;
  }

  uint16_t load(uint16_t addr) {
    addr = Storage::read(addr, wave);

    uint8_t nom, den;
    addr = Storage::read(addr, nom);
    addr = Storage::read(addr, den);

    for (uint8_t i = 0; i <= MAX_FACTOR; i++) {
      if (nom == factorNom[i] && den == factorDen[i]) {
        factor = i;
        break;
      }
    }

    addr = Storage::read(addr, phase);
    addr = Storage::read(addr, range);
    return addr;
  }

  void recalc(uint32_t mainIncrement) {
    increment = mainIncrement / factorNom[factor] * factorDen[factor];
    //int32_t inc = int32_t(increment);
    // swing 0 -> diff 0
    // swing +25 -> diff increment / 2
    // swing -25 -> diff -increment / 2
    //incrementA = inc * (100 + swing) / 100;
    //incrementB = inc * 100 / (100 + swing);
    // NO
    // Calculate the pos based on the swingPeriod.
    // Period 0: pos is swing% slower
    // Period 1, less than 256 + swing% ? pos + 256, but slower
    // Period 1, otherwise -> swing% faster
  }

  void reset() {
    period = 0;
  }

  void nextPeriod() {
    period++;
    if (period >= swingPeriods) {
      period -= swingPeriods;
    }
  }

  void resetPhase() {
    constexpr auto OK_TO_RESET_MIN = uint32_t(2) << sinePosScaleBits;
    constexpr auto OK_TO_RESET_MAX = uint32_t(254) << sinePosScaleBits;

    if (factor >= FACTOR_ONE) {
      tablePos = 0;
      nextPeriod();
    } else if (tablePos < OK_TO_RESET_MIN || tablePos > OK_TO_RESET_MAX) {
      tablePos = 0;
      nextPeriod();
    }
  }

  void performStep() {
    tablePos += increment;
    if (tablePos > sinePosMod) {
      tablePos -= sinePosMod;
    }
  }

  int16_t applyRange(int16_t value) {
    switch(range) {
      case Negative4: return (value / 2) - 2000;
      case Bipolar4: return value;
      case Positive4: return (value / 2) + 2000;
      case Positive8: return value + 4000;
    }
    return 0;
  }

  int16_t getRawTableValue() {
    uint32_t pos;
    if (swing > 0) {
      if (period == 0) {
        // We should be swing% slower
        pos = (tablePos * (100 - swing)) / 100;
      } else if (period == 1) {
        uint8_t period0End = uint16_t(255) * (swing) / 100;
        if ((tablePos >> sinePosScaleBits) <= period0End) {
          uint8_t zero = uint16_t(256) * (100 - swing) / 100;
          uint8_t end = 255;
          pos = (uint32_t(zero) << sinePosScaleBits) + (tablePos * (end - zero) / period0End);
        } else {
          pos = (tablePos - (uint32_t(period0End) << sinePosScaleBits)) * 256 / (uint16_t(256) - period0End);
        }
      } else {
        pos = tablePos;
      }
    } else {
      pos = tablePos;
    }

    switch (wave) {
      case Sine: return Waves::Sine::get(pos, phase);
      case Triangle: return Waves::Triangle::get(pos, phase);
      case SawUp: return Waves::SawUp::get(pos, phase);
      case SawDown: return Waves::SawDown::get(pos, phase);
      case Square: return Waves::Square::get(pos, phase);
    }
    return 0;
  }

  int16_t getTableValue() {
    return applyRange(getRawTableValue());
  }
};

constexpr uint8_t N_SHAPES = 4;
constexpr uint8_t CONTROLS_PER_SHAPE = 8;
Shape shapes[N_SHAPES];

uint16_t bpm = 60;

const char title[] PROGMEM = "LFO      ";
const char bpm_t[] PROGMEM = "BPM:  ";
const char wave1_t[] PROGMEM = "A:";
const char wave2_t[] PROGMEM = "B:";
const char wave3_t[] PROGMEM = "1:";
const char wave4_t[] PROGMEM = "2:";
const char page1[] PROGMEM = "Wave Spd. Phase Range";
const char page2[] PROGMEM = "Swing SwPer Prob Slop";

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
  uint8_t selectedControlIdx = (currentControlIdx - 2) % CONTROLS_PER_SHAPE;

  if (currentControlIdx < 2) {
    drawSelected(0, 16, 1);
    drawTextPgm(7, 16, bpm_t);
    drawDecimal(40, 16, bpm, 14);
  } else if (selectedControlIdx <= 3) {
    drawTextPgm(0, 16, page1);
  } else {
    drawTextPgm(0, 16, page2);
  }

  drawTextPgm(0, 24, wave1_t);
  drawTextPgm(0, 32, wave2_t);
  drawTextPgm(0, 40, wave3_t);
  drawTextPgm(0, 48, wave4_t);


  for (uint8_t s = 0; s < N_SHAPES; s++) {
    uint8_t controlIdx = s * CONTROLS_PER_SHAPE + 2;
    uint8_t y = s * 8 + 24;

    if (selectedControlIdx > 3 && selectedControlIdx < CONTROLS_PER_SHAPE) {
      drawSelected(12, y, controlIdx + 4);
      drawDecimal(18, y, shapes[s].swing, '%', 4);

      drawSelected(42, y, controlIdx + 5);
      drawText(48, y, "x");
      drawDecimal(54, y, shapes[s].swingPeriods, 1);

      drawSelected(66, y, controlIdx + 6);
      drawText(72, y, "100%");

      drawSelected(96, y, controlIdx + 7);
      drawText(102, y, "100%");
    } else {
      drawSelected(12, y, controlIdx);
      drawTextPgm(18, y, waveTitles[shapes[s].wave]);

      drawSelected(36, y, controlIdx + 1);
      char str[6];
      formatFactor(str, shapes[s].factor);
      drawText(42, y, str);

      drawSelected(60, y, controlIdx + 2);
      uint16_t ph = (uint32_t(shapes[s].phase) * 360) >> 8;
      drawDecimal(66, y, ph, '\'', 4);

      drawSelected(90, y, controlIdx + 3);
      drawTextPgm(96, y, rangeTitles[shapes[s].range]);
    }
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
          shapes[shapeIdx].recalc(increment);
          break;
        case 2:
          shapes[shapeIdx].phase = applyDelta<uint8_t>(shapes[shapeIdx].phase, d, 0, 255);
          break;
        case 3:
          shapes[shapeIdx].range = Range(applyDelta<uint8_t>(shapes[shapeIdx].range, d, 0, N_RANGE - 1));
          break;
        case 4:
          shapes[shapeIdx].swing = applyDelta<int8_t>(shapes[shapeIdx].swing, d, /*-25*/0, 25);
          shapes[shapeIdx].recalc(increment);
          break;
        case 5:
          shapes[shapeIdx].swingPeriods = applyDelta<int8_t>(shapes[shapeIdx].swingPeriods, d, 2, 8);
          shapes[shapeIdx].recalc(increment);
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

void reset() {
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].reset();
  }
}

void start() {
  recalc();
  reset();
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

void save(uint16_t addr) {
  uint8_t version = 1;
  Storage::write(addr, version);
  addr++;
  Storage::write(addr, bpm);
  addr += 2;

  for (uint8_t i = 0; i < N_SHAPES; i++) {
    addr = shapes[i].save(addr);
  }
}

void load(uint16_t addr) {
  uint8_t version;
  Storage::read(addr, version);
  addr++;
  if (version != 1) return;

  Storage::read(addr, bpm);
  addr += 2;

  for (uint8_t i = 0; i < N_SHAPES; i++) {
    addr = shapes[i].load(addr);
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
  NULL,
  &save,
  &load
};

} // FuncGen
