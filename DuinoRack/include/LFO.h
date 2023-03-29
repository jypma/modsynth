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
constexpr Q16n16 MIN_NOTE = 65536; // Note 1

uint32_t sinePos = 0;
const uint16_t *table = SIN_DATA;
uint32_t increment;
uint16_t bpm = 120;
enum Wave: uint8_t { Sine = 0, Triangle = 1, SawUp = 2, SawDown = 3 };
Wave wave = Sine;

const char title[] PROGMEM = "LFO   ";
const char bpm_t[] PROGMEM = "BPM:  ";
const char wave_t[] PROGMEM = "Wave: ";
const char sine_t[] PROGMEM = "Sine    ";
const char triangle_t[] PROGMEM = "Triangle";
const char saw_up_t[] PROGMEM = "Saw Up   ";
const char saw_down_t[] PROGMEM = "Saw Down ";

bool lastGate = false;
uint32_t lastGateTime = -1;

void recalc() {
  // Multiply by OUTBUFSIZE, since we increment only once per buffer
  increment = (((uint32_t(bpm) * TABLE_SIZE) << sinePosScaleBits)) / 60 * OUTBUFSIZE / SAMPLERATE;
}

void draw() {
  drawTextPgm(8, 16, bpm_t);
  drawDecimal(40, 16, bpm);
  drawTextPgm(8, 24, wave_t);
  drawText(0, 16, (currentControlIdx == 1) ? ">" : " ");
  drawText(0, 24, (currentControlIdx == 2) ? ">" : " ");
  switch (wave) {
    case Sine: drawTextPgm(52, 24, sine_t); break;
    case Triangle: drawTextPgm(52, 24, triangle_t); break;
    case SawUp: drawTextPgm(52, 24, saw_up_t); break;
    case SawDown: drawTextPgm(52, 24, saw_down_t); break;
  }
}

void adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      bpm = constrain(bpm + d, 1, 1000);
      recalc();
      break;
    case 2:
      wave = Wave((wave + d) % 4);
      switch (wave) {
        case Sine: table = SIN_DATA; break;
        case Triangle: table = TRIANGLE_DATA; break;
        case SawUp: table = SAW_UP_DATA; break;
        case SawDown: table = SAW_DOWN_DATA; break;
      }
      break;
  }
}

void checkGate() {
  bool gate = IO::getGate1In();
  if (gate != lastGate) {
    lastGate = gate;
    if (gate) {
      auto time = micros();
      Serial.println(time);
      if (lastGateTime != -1) {
        // Reset the phase on incoming pulse
        sinePos = 0;

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
  lastGateTime = -1;
}

void stop() {}

uint16_t getTableValue(uint32_t pos) {
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
  sinePos = (sinePos + increment);
  if (sinePos > sinePosMod) {
    sinePos -= sinePosMod;
  }
  const uint16_t value = getTableValue(sinePos);

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = value;
    buf->cv2 = value;
    buf->gate1 = value >> 2;
    buf->gate2 = buf->gate1;
    buf++;
  }
}

constexpr Module module = {
  title,
  2,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer
};

} // FuncGen
