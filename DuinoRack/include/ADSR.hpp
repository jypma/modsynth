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

const char *getEnvelopeTitle(Envelope env) {
  switch(env) {
    case Root3: return envRoot3_t;
    case Sin: return envSin_t;
    case Lin: return envLin_t;
    case Power2: return envPower2_t;
    default: return envPower3_t;
  }
}

static constexpr uint8_t CONTROLS_PER_INSTANCE = 7;

class Instance {
  uint8_t index;
  uint8_t ys;

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

  // Full envelope is selected at T = 1000 milliseconds
  // One increment is selected at T / 256 = 3.9 milliseconds
  // We increment once per buffer, so that's SAMPLE_RATE / BUF_SIZE = 750 Hz or once every 1.333ms
  // So every buffer we should increment by 1.3 / 3.9 = 0.3 increments
  //                                    (1 / 750) / (T / 256) = 256 / 750 / T (in seconds)
  uint16_t attack;
  uint16_t decay;
  uint16_t release;
  uint32_t attackPosInc;
  uint32_t decayPosInc;
  uint32_t releasePosInc;
  static constexpr uint32_t posPerSecond = ((uint32_t(256) * 1000) << Waves::posScaleBits) / (SAMPLERATE / OUTBUFSIZE);
public:
  Instance() {
    setAttack(100);
    setDecay(100);
    setRelease(100);
  }

  uint16_t getAttack() { return attack; }
  void setAttack(uint16_t ms) {
    attack = ms;
    attackPosInc = posPerSecond / ms;
  }

  uint16_t getDecay() { return decay; }
  void setDecay(uint16_t ms) {
    decay = ms;
    decayPosInc = posPerSecond / ms;
  }

  uint16_t getRelease() { return release; }
  void setRelease(uint16_t ms) {
    release = ms;
    releasePosInc = posPerSecond / ms;
  }

  uint8_t getSustain() { return sustainLevel; }
  void setSustain(uint8_t s) {
    sustainLevel = s;
    sustain = zero + uint32_t(top - zero) * sustainLevel / 100;
  }

  void drawSelected(uint8_t x, uint8_t y, uint8_t control) {
    uint8_t current = (currentControlIdx - 1) % CONTROLS_PER_INSTANCE;
    uint8_t currentIdx = (currentControlIdx - 1) / CONTROLS_PER_INSTANCE;
    bool selected = (currentIdx == index) && (control == current);
    drawChar(x, y, selected ? '>' : ' ');
  }

  void draw() {
    drawSelected(0, ys, 0);
    drawDecimal(6, ys, attack, 4);
    drawSelected(30, ys, 1);
    drawTextPgm(36, ys, getEnvelopeTitle(attackEnv));

    drawSelected(32, ys + 8, 2);
    drawDecimal(32 + 6, ys + 8, decay, 4);
    drawSelected(32 + 30, ys + 8, 3);
    drawTextPgm(32 + 36, ys + 8, getEnvelopeTitle(decayEnv));

    drawSelected(64, ys, 4);
    drawDecimal(64 + 6, ys, sustainLevel, 3);
    drawText(64 + 18, ys, "%");

    drawSelected(90, ys + 8, 5);
    drawDecimal(90 + 6, ys + 8, release, 4);
    drawSelected(90 + 30, ys + 8, 6);
    drawTextPgm(90 + 36, ys + 8, getEnvelopeTitle(releaseEnv));
  }

  void adjust(int8_t d) {
    uint8_t control = (currentControlIdx - 1) % CONTROLS_PER_INSTANCE;
    switch(control) {
      case 0: setAttack(applyDelta<uint16_t>(attack, d, 1, 60000)); break;
      case 1: attackEnv = Envelope(applyDelta<uint8_t>(attackEnv, d, 0, N_ENVELOPES - 1)); break;
      case 2: setDecay(applyDelta<uint16_t>(decay, d, 1, 60000)); break;
      case 3: decayEnv = Envelope(applyDelta<uint8_t>(decayEnv, d, 0, N_ENVELOPES - 1)); break;
      case 4: setSustain(applyDelta<uint8_t>(sustainLevel, d, 0, 100)); break;
      case 5: setRelease(applyDelta<uint16_t>(release, d, 1, 60000)); break;
      case 6: releaseEnv = Envelope(applyDelta<uint8_t>(releaseEnv, d, 0, N_ENVELOPES - 1)); break;
      default: {}
    }
  }

  void reset(uint8_t _idx, uint16_t z, uint16_t t) {
    index = _idx;
    ys = 16 + 16 * index;
    phase = Idle;
    phasePos = 0;
    zero = z;
    top = t;
    setSustain(sustainLevel);
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
  adsr1.draw();
  adsr2.draw();
  adsr3.draw();
}

void start() {
  adsr1.reset(0, IO::calcCV1Out(0), IO::calcCV1Out(8000));
  adsr2.reset(1, IO::calcCV2Out(0), IO::calcCV2Out(8000));
  adsr3.reset(2, IO::calcGate1Out(0), IO::calcGate1Out(8000));
}

void stop() {

}

void adjust(int8_t d) {
  uint8_t index = (currentControlIdx - 1) / CONTROLS_PER_INSTANCE;
  switch(index) {
    case 0: adsr1.adjust(d); break;
    case 1: adsr2.adjust(d); break;
    case 2: adsr3.adjust(d); break;
    default: {}
  }
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
  3 * CONTROLS_PER_INSTANCE,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  NULL
};
}
