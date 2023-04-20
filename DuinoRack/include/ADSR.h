#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"

namespace ADSR {
  const char title[] PROGMEM = "ADSR      ";

  enum Phase { Idle, Attack, Decay, Sustain, Release };

  Phase phase = Idle;
  Q15n16 zero = 0;
  Q15n16 max = 0;
  Q16n16 level = 0;
  Q16n16 attack = float_to_Q16n16(10);
  Q16n16 decay = float_to_Q16n16(10);
  Q16n16 release = float_to_Q16n16(1);
  Q16n16 sustain = Q16n0_to_Q16n16(4000);

  void draw() {
    drawTextPgm(0, 16, clear);
    drawTextPgm(0, 24, clear);
    drawTextPgm(0, 32, clear);
  }

  void start() {

  }

  void stop() {

  }

  void adjust(int8_t d) {
    zero = Q16n0_to_Q15n16(IO::calcCV1Out(0));
    max = Q16n0_to_Q15n16(IO::calcCV1Out(8000));
  }

  int16_t nextLevel() {
    switch (phase) {
    case Idle:
      if (IO::getGate1In()) {
        phase = Attack;
        level = zero;
      }
      break;
    case Attack:
    case Decay:
    case Sustain:
      if (!IO::getGate1In()) {
        phase = Release;
      }
      break;
    case Release:
      if (IO::getGate1In()) {
        phase = Attack;
      }
      break;
    }

    Q15n16 next = level;
    switch(phase) {
    case Idle:
      break;
    case Attack:
      next = level + attack;
      if (next >= max) {
        next = max;
        phase = Decay;
      }
      break;
    case Decay:
      next = Q15n16(level) - decay;
      if (next <= Q15n16(sustain)) {
        next = sustain;
        phase = Sustain;
      }
      break;
    case Sustain:
      break; // Wait for IO to go low, which we're checking before.
    case Release:
      next = Q15n16(level) - release;
      if (next <= zero) {
        next = zero;
        phase = Idle;
      }
    }
    level = next;
    return Q15n16_to_Q15n0(level);
  }

  void fillBuffer(OutputFrame *buf) {
    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      buf->cv1 = nextLevel();
      // TODO separate ADSR for cvB
      buf->cv2 = buf->cv1;
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
