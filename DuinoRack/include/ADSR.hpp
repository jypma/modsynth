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

class Instance {
  Phase phase = Idle;
  uint32_t phasePos = 0;
  Envelope attackEnv = Root3;
  Envelope decayEnv = Lin;
  Envelope releaseEnv = Lin;

  uint16_t zero = 0;
  uint16_t top = 0;
  uint8_t sustainLevel = 50;
  uint16_t sustain = 0;
  uint16_t level = 0; // Current level
  uint16_t phaseLevel = 0; // Level at start of current phase

  // We increment once per buffer, so that's SAMPLE_RATE / BUF_SIZE = 750 Hz or 1.333ms
  uint32_t attackPosMs = 1000;
  uint32_t decayPosMs = 1000;
  uint32_t releasePosMs = 1000;
  uint32_t attackPosInc = 1630;
  uint32_t decayPosInc = 1630;
  uint32_t releasePosInc = 1630;
public:
  void reset(uint16_t z, uint16_t t) {
    phase = Idle;
    phasePos = 0;
    zero = z;
    top = t;
    sustain = (top - zero) * sustainLevel / 100;
    level = zero;
    phaseLevel = zero;
  }

  void handleGate(bool gate) {
    switch (phase) {
      case Idle:
        if (gate) {
          phase = Attack;
          level = zero;
          phaseLevel = zero;
          phasePos = 0;
        }
        break;
      case Attack:
      case Decay:
      case Sustain:
        if (!gate) {
          phase = Release;
          phaseLevel = level;
          phasePos = 0;
        }
        break;
      case Release:
        if (gate) {
          phase = Attack;
          phaseLevel = level; // Retrigger from current release level, if any
          phasePos = 0;
        }
        break;
    }
  }

  uint16_t nextLevel() {
    Q15n16 next = level;
    switch (phase) {
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
};

Instance adsr1, adsr2, adsr3;

void draw() {
  drawTextPgm(0, 16, clear);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

void start() {
  adsr1.reset(IO::calcCV1Out(0), IO::calcCV1Out(8000));
  adsr2.reset(IO::calcCV2Out(0), IO::calcCV2Out(8000));
  adsr3.reset(IO::calcGate1Out(0), IO::calcGate1Out(8000));
}

void stop() {

}

void adjust(int8_t d) {
}

int16_t nextLevel() {
  adsr1.handleGate(IO::getGate1In());
  return adsr1.nextLevel();
}

void fillBuffer(OutputFrame *buf) {
  auto gate2 = IO::calcGate2Out(0); // unused, always 0V

  // TODO disable interpolation and calculate each buffer value for "fast" curves.
  adsr1.handleGate(IO::getGate1In());
  auto cv1 = adsr1.nextLevel();
  adsr2.handleGate(IO::getGate2In());
  auto cv2 = adsr2.nextLevel();
  adsr3.handleGate(IO::getGate3In());
  auto gate1 = adsr3.nextLevel();
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = cv1;
    buf->cv2 = cv2;
    buf->gate1 = gate1;
    buf->gate2 = gate2;
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
