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
const char cal6[] PROGMEM = "oA +8V: ";
const char cal7[] PROGMEM = "oB  0V: ";
const char cal8[] PROGMEM = "oB +8V: ";

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
    default: return NULL;
  }
}

int16_t *getCalValue() {
  switch (currentControlIdx) {
    case 1: return &IO::cvIn1_0V;
    case 2: return &IO::cvIn1_4V;
    case 3: return &IO::cvIn2_0V;
    case 4: return &IO::cvIn2_4V;
    case 5: return &IO::cvOut1_0V;
    case 6: return &IO::cvOut1_8V;
    case 7: return &IO::cvOut2_0V;
    case 8: return &IO::cvOut2_8V;
    default: return NULL;
  }
}

String format(int16_t mV) {
  String res = " 0.000 V";
  if (mV < 0) {
    res.setCharAt(0, '-');
    mV = -mV;
  }
  res.setCharAt(1, '0' + mV / 1000);
  mV %= 1000;
  res.setCharAt(3, '0' + mV / 100);
  mV %= 100;
  res.setCharAt(4, '0' + mV / 10);
  mV %= 10;
  res.setCharAt(5, '0' + mV);

  return res;
}

void draw() {
  String txt = format(IO::getCV1In()) + "    ";
  drawText(0, 16, "inA: ");
  drawText(40, 16, txt.c_str());

  txt = format(IO::getCV2In()) + "    ";
  drawText(0, 24, "inB: ");
  drawText(40, 24, txt.c_str());

  const char *lbl = getCalLabel();
  int16_t *val = getCalValue();
  if (lbl != NULL && val != NULL) {
    drawText(0, 32, "> ");
    drawTextPgm(16, 32, lbl);
    drawDecimal(72, 32, *val);
  } else {
    drawTextPgm(0, 32, clear);
  }
}

uint16_t addIn(int16_t in, int8_t d) {
  return constrain(in + d, 0, 1023);
}
uint16_t addOut(int16_t in, int8_t d) {
  return constrain(in + d, 0, 4095);
}

void adjust(int8_t d) {
  switch (currentControlIdx) {
    // FIXME contrain with the other calibration value, so we don't set them equal
    case 1: IO::cvIn1_0V = addIn(IO::cvIn1_0V, d); break;
    case 2: IO::cvIn1_4V = addIn(IO::cvIn1_4V, d); break;
    case 3: IO::cvIn2_0V = addIn(IO::cvIn2_0V, d); break;
    case 4: IO::cvIn2_4V = addIn(IO::cvIn2_4V, d); break;
    case 5: IO::cvOut1_0V = addOut(IO::cvOut1_0V, d); break;
    case 6: IO::cvOut1_8V = addOut(IO::cvOut1_8V, d); break;
    case 7: IO::cvOut2_0V = addOut(IO::cvOut2_0V, d); break;
    case 8: IO::cvOut2_8V = addOut(IO::cvOut2_8V, d); break;
  }
}

void fillBuffer(OutputFrame *buf) {
  auto outA = IO::calcCV1Out(0);
  auto outB = IO::calcCV2Out(0);
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cvA = outA;
    buf->cvB = outB;
    buf++;
  }
}

constexpr Module module = {
  title,
  8, // controlCount
  &draw,
  &adjust,
  &fillBuffer
};
}
