#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "Waves.hpp"
#include "Debug.hpp"

namespace ADSR {

const char title[] PROGMEM = "ADSR      ";

const char attack_t[] PROGMEM = "A:";
const char decay_t[] PROGMEM = "D:";
const char sustain_t[] PROGMEM = "S:";
const char release_t[] PROGMEM = "R:";

const char envRoot3_t[] PROGMEM = "R3";
const char envSin_t[] PROGMEM = "Sn";
const char envLin_t[] PROGMEM = "Ln";
const char envPower2_t[] PROGMEM = "P2";
const char envPower3_t[] PROGMEM = "P3";

enum Phase: uint8_t { Idle, Attack, Decay, Sustain, Release };
enum Envelope : uint8_t { Root3, Sin, Lin, Power2, Power3 };
constexpr uint8_t N_ENVELOPES = 5;

uint16_t getEnvelope(Envelope env, uint32_t pos);

const char *getEnvelopeTitle(Envelope env);

static constexpr uint8_t CONTROLS_PER_INSTANCE = 7;

class Segment {
  uint32_t pos;
  uint32_t posInc;
  uint16_t length;
  uint16_t start;
  uint16_t end;
  uint8_t controlIdx;
public:
  Envelope env = Lin;

  void reset(uint8_t controlIdx);
  void setLength(uint16_t ms);
  void adjustLength(int8_t d);
  void adjustEnv(int8_t d);
  void begin(uint16_t _start, uint16_t _end);
  uint16_t advance();
  bool isEnded();
  void draw(uint8_t xs, uint8_t ys);
};

class Instance {
  uint8_t index;
  uint8_t ys;

  Phase phase = Idle;
  Segment attack, decay, release;

  uint16_t zero = 0;
  uint16_t top = 0;
  uint8_t sustainLevel = 50;
  uint16_t sustain = 0;
  uint16_t level = 0; // Current level
  void setSustain(uint8_t s);

public:
  void draw();
  void adjust(int8_t d);
  void reset(uint8_t _idx, uint16_t z, uint16_t t);
  void handleGate(bool gate);
  uint16_t nextLevel();
};

extern Instance adsr1, adsr2, adsr3;

extern void draw();
extern void start();
extern void stop();
extern void adjust(int8_t d);
extern void fillBuffer(OutputFrame *buf) ;

constexpr Module module = {
  title,
  3 * CONTROLS_PER_INSTANCE,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  NULL
};
}
