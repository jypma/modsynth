#pragma once

#include "Module.h"

namespace FuncGen {

const char title[] PROGMEM = "FuncGen   ";

void draw() {
  drawTextPgm(0, 16, clear);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

void adjust(int8_t d) {

}

constexpr Module module = {
  title,
  0,
  &draw,
  &adjust
};

} // FuncGen
