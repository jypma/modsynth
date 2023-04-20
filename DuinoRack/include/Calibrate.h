#pragma once

#include "Arduino.h"
#include "Module.h"
#include "IO.h"

namespace Calibrate {
  const char title[] PROGMEM = "Calibrate ";
  const char cal1[] PROGMEM = "iA  0V: ";
  const char cal2[] PROGMEM = "iA +4V: ";
  const char cal3[] PROGMEM = "iB  0V: ";
  const char cal4[] PROGMEM = "iB +4V: ";
  const char cal5[] PROGMEM = "oA  0V: ";
  const char cal6[] PROGMEM = "oA +4V: ";
  const char cal7[] PROGMEM = "oB  0V: ";
  const char cal8[] PROGMEM = "oB +4V: ";
  const char cal9[] PROGMEM = "g1  0V: ";
  const char cal10[] PROGMEM = "g1 +4V: ";
  const char cal11[] PROGMEM = "g2  0V: ";
  const char cal12[] PROGMEM = "g2 +4V: ";
  const char calSave[] PROGMEM = "Save? (press Adj)";

  const char *getCalLabel() {
    switch (currentControlIdx) {
      case 1: return cal1;
      case 2: return cal2;
      case 3: return cal3;
      case 4: return cal4;
      case 5: return cal5;
      case 6: return cal6;
      case 7: return cal7;
      case 8: return cal8;
      case 9: return cal9;
      case 10: return cal10;
      case 11: return cal11;
      case 12: return cal12;
      default: return NULL;
    }
  }

int16_t getCalValue() {
  switch (currentControlIdx) {
    case 1: return IO::cvIn1_0V;
    case 2: return IO::cvIn1_4V;
    case 3: return IO::cvIn2_0V;
    case 4: return IO::cvIn2_4V;
    case 5: return IO::getcvOut1_0V();
    case 6: return IO::getcvOut1_4V();
    case 7: return IO::getcvOut2_0V();
    case 8: return IO::getcvOut2_4V();
    case 9: return IO::getGate1_0V();
    case 10: return IO::getGate1_4V();
    case 11: return IO::getGate2_0V();
    case 12: return IO::getGate2_4V();
    default: return 0;
  }
}

void format(int16_t mV, char *target) {
  strcpy(target, " 0.000 V");
  if (mV < 0) {
    target[0] = '-';
    mV = -mV;
  }
  target[1] = '0' + mV / 1000;
  mV %= 1000;
  target[3] = '0' + mV / 100;
  mV %= 100;
  target[4] = '0' + mV / 10;
  mV %= 10;
  target[5] = '0' + mV;
}

void draw() {
  char txt[12];
  format(IO::getCV1In(), txt);
  drawText(0, 16, "inA: ");
  drawText(25, 16, txt);

  format(IO::getCV2In(), txt);
  drawText(0, 24, "inB: ");
  drawText(25, 24, txt);

  drawText(0, 32, "in gates: ");
  drawText(55, 32, IO::getGate1In() ? "1" : "0");
  drawText(61, 32, IO::getGate2In() ? "1" : "0");
  drawText(67, 32, IO::getGate3In() ? "1" : "0");

  if (currentControlIdx == 13) {
    drawText(0, 40, "> ");
    drawTextPgm(16, 40, calSave);
  } else {
    const char *lbl = getCalLabel();
    int16_t val = getCalValue();
    if (lbl != NULL) {
      drawText(0, 40, "> ");
      drawTextPgm(16, 40, lbl);
      drawDecimal(72, 40, val);
    } else {
      drawTextPgm(0, 40, clear);
    }
  }
}

void start() {
}

void stop() {

}

uint16_t addIn(int16_t in, int8_t d) {
  return applyDelta<uint16_t>(in, d, 0, 1023);
}

void adjust(int8_t d) {
  switch (currentControlIdx) {
    case 1: IO::cvIn1_0V = addIn(IO::cvIn1_0V, d); break;
    case 2: IO::cvIn1_4V = addIn(IO::cvIn1_4V, d); break;
    case 3: IO::cvIn2_0V = addIn(IO::cvIn2_0V, d); break;
    case 4: IO::cvIn2_4V = addIn(IO::cvIn2_4V, d); break;
    case 5: IO::calibrateCVOut1_0V(d); break;
    case 6: IO::calibrateCVOut1_4V(d); break;
    case 7: IO::calibrateCVOut2_0V(d); break;
    case 8: IO::calibrateCVOut2_4V(d); break;
    case 9: IO::calibrateGate1_0V(d); break;
    case 10: IO::calibrateGate1_4V(d); break;
    case 11: IO::calibrateGate2_0V(d); break;
    case 12: IO::calibrateGate2_4V(d); break;
    default: break;
  }
}

  void adjustPressed() {
    if (currentControlIdx == 13) {
      IO::saveCalibration();
      currentControlIdx = 0;
      drawTextPgm(0, 40, clear);
      draw();
    }
  }

void fillBuffer(OutputFrame *buf) {
  bool calibInCV4V = (currentControlIdx == 2 || currentControlIdx == 4);
  auto out_mVa = (calibInCV4V || (currentControlIdx == 6)) ? 4000 : 0;
  auto out_mVb = (calibInCV4V || (currentControlIdx == 8)) ? 4000 : 0;
  auto outA = IO::calcCV1Out(out_mVa);
  auto outB = IO::calcCV2Out(out_mVb);
  auto gate1 = IO::calcGate1Out((currentControlIdx == 10) ? 4000 : 0);
  auto gate2 = IO::calcGate2Out((currentControlIdx == 12) ? 4000 : 0);
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = outA;
    buf->cv2 = outB;
    buf->gate1 = gate1;
    buf->gate2 = gate2;
    buf++;
  }
}

constexpr Module module = {
  title,
  13, // controlCount
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  &adjustPressed,
};
}
