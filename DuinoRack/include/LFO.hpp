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

enum Range: uint8_t { Negative4, Bipolar4, Positive4, Positive8, Positive5 };

static constexpr uint8_t factorNom[] = { 4, 3, 2, 1, 1, 1, 1 };
static constexpr uint8_t factorDen[] = { 1, 1, 1, 1, 2, 3, 4 };
constexpr uint8_t FACTOR_ONE = 3; // The factor of 1/1, i.e. normal BPM
constexpr uint8_t MAX_FACTOR = 6;

constexpr uint8_t N_SHAPES = 5;
constexpr uint8_t CONTROLS_PER_SHAPE = 8;

constexpr uint32_t NO_TIME = 0xFFFFFFFF;

struct Shape {
  uint32_t tablePos = 0;
  uint32_t increment;  // For further periods
  uint8_t prob = 100;
  int8_t swing = 0;
  int8_t period = 0;
  bool skipThisPeriod = false;
  bool skippedPrevPeriod = false;
  uint8_t mainPeriod = 0;
  int8_t swingPeriods = 2;
  // slop: 7 random values around +/- 49%(?), add them up, 8th is to bring it back in sync.
  // Always resync after 8 periods.
  // Directly affects tablePos (unlike swing)
  uint8_t factor = FACTOR_ONE;
  Wave wave = Sine;
  uint8_t phase = 0; // 0..255 as one period
  Range range = Bipolar4;

  uint8_t resetAfterMainPeriods();
  uint16_t save(uint16_t addr);
  uint16_t load(uint16_t addr);
  void recalc(uint32_t mainIncrement);
  void reset();
  void nextPeriod();
  void resetPhase();
  void performStep();
  int16_t applyRange(int16_t value);
  int16_t getRawTableValue();
  int16_t getTableValue();
  void draw(uint8_t y, uint8_t controlIdx);
};

class LFO {
  uint32_t mainPos;
  uint32_t increment;
  Shape shapes[N_SHAPES];
  uint16_t bpm = 60;
  bool lastGate = false;
  uint32_t lastGateTime = NO_TIME;
  void recalc();
  void resetPhase();
  void checkGate();
  void reset();
public:
  void draw();
  void adjust(int8_t d);
  void start();
  void fillBuffer(OutputBuf::Buffer &buf);
  void save(uint16_t addr);
  void load(uint16_t addr);
};

extern LFO lfo;

inline void draw() { lfo.draw(); }
inline void start() { lfo.start(); }
inline void stop() {}
inline void adjust(int8_t d) { lfo.adjust(d); }
inline void fillBuffer(OutputBuf::Buffer &buf) { lfo.fillBuffer(buf); }
inline void save(uint16_t addr) { lfo.save(addr); }
inline void load(uint16_t addr) { lfo.load(addr); }

extern const char title[] PROGMEM;
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
