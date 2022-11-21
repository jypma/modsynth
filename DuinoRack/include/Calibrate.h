#pragma once

#include "Module.h"
#include "IO.h"

namespace Calibrate {
const char title[] PROGMEM = "Calibrate ";
const char cal1[] PROGMEM = ">A  0V: ";
const char cal2[] PROGMEM = ">A +4V: ";
const char cal3[] PROGMEM = ">B  0V: ";
const char cal4[] PROGMEM = ">B +4V: ";

const char *getCalLabel() {
  switch (currentControlIdx) {
    case 1: return cal1;
    case 2: return cal2;
    case 3: return cal3;
    case 4: return cal4;
    default: return NULL;
  }
}

void draw() {
  String txt = String(IO::getCV1()) + "    ";
  drawText(0, 16, txt.c_str());

  txt = String(IO::getCV2()) + "    ";
  drawText(0, 24, txt.c_str());

  const char *lbl = getCalLabel();
  if (lbl != NULL) {
    drawTextPgm(0, 32, lbl);
  } else {
    drawText(0, 32, "             ");
  }
}

constexpr Module module = {
  title,
  4, // controlCount
  &draw
};
}
