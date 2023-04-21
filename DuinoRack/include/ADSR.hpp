#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "Waves.hpp"

namespace ADSR {

const char title[] PROGMEM = "ADSR      ";

enum Phase: uint8_t { Idle, Attack, Decay, Sustain, Release };
enum Envelope: uint8_t { Root3, Sin, Lin, Power2, Power3 };

Phase phase = Idle;
uint32_t phasePos = 0;
Envelope attackEnv = Root3;
Envelope decayEnv = Lin;
Envelope releaseEnv = Lin;

uint16_t zero = 0;
uint16_t top = 0;
uint16_t sustain = 0;
uint16_t level = 0; // Current level
uint16_t phaseLevel = 0; // Level at start of current phase

uint32_t attackPosInc = 1630;
uint32_t decayPosInc = 1630;
uint32_t releasePosInc = 1630;

void draw() {
  drawTextPgm(0, 16, clear);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

void start() {
  zero = IO::calcCV1Out(0);
  top = IO::calcCV1Out(8000);
  sustain = (top + zero) / 2;
  level = zero;
  phaseLevel = zero;
}

void stop() {

}

void adjust(int8_t d) {
}

uint16_t getEnvelope(Envelope env, uint32_t pos) {
  switch(env) {
    case Root3: return Waves::EnvelopeRoot3::get(pos);
    case Sin: return Waves::EnvelopeSin::get(pos);
    case Power2: return Waves::EnvelopePower2::get(pos);
    case Power3: return Waves::EnvelopePower3::get(pos);
    case Lin: return pos >> (Waves::posScaleBits - 8);
    default: return 0;
  }
}

int16_t nextLevel() {
  switch (phase) {
    case Idle:
      if (IO::getGate1In()) {
        phase = Attack;
        level = zero;
        phaseLevel = zero;
        phasePos = 0;
      }
      break;
    case Attack:
    case Decay:
    case Sustain:
      if (!IO::getGate1In()) {
        phase = Release;
        phaseLevel = level;
        phasePos = 0;
      }
      break;
    case Release:
      if (IO::getGate1In()) {
        phase = Attack;
        phaseLevel = level; // Retrigger from current release level, if any
        phasePos = 0;
      }
      break;
  }

  Q15n16 next = level;
  switch(phase) {
    case Idle:
      break;
    case Attack: {
      const uint16_t scale = (top - phaseLevel);
      next = phaseLevel + ((uint32_t(getEnvelope(attackEnv, phasePos)) * scale) >> 16);
      phasePos += attackPosInc;
      if (phasePos >= Waves::maxPos) {
        next = top;
        phaseLevel = top;
        phase = Decay;
        phasePos = 0;
      }
      break;
    }
    case Decay: {
      const uint16_t scale = phaseLevel - sustain;
      next = phaseLevel - ((uint32_t(getEnvelope(decayEnv, phasePos)) * scale) >> 16);
      phasePos += decayPosInc;
      if (phasePos >= Waves::maxPos) {
        next = sustain;
        phaseLevel = sustain;
        phase = Sustain;
        phasePos = 0;
      }
      break;
    }
    case Sustain:
      break; // Wait for IO to go low, which we're checking before.
    case Release: {
      const uint16_t scale = phaseLevel - zero;
      next = phaseLevel - ((uint32_t(getEnvelope(releaseEnv, phasePos)) * scale) >> 16);
      phasePos += releasePosInc;
      if (phasePos >= Waves::maxPos) {
        next = zero;
        phaseLevel = zero;
        phase = Idle;
        phasePos = 0;
      }
    }
  }
  level = next;
  return next;
}

void fillBuffer(OutputFrame *buf) {
  // TODO disable interpolation and calculate each buffer value for "fast" curves.
  auto l = nextLevel();
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = l;
    buf++;
  }
}

constexpr Module module = {
  title,
  0,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  NULL
};
}
