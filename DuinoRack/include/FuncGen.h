#pragma once

#include "Module.h"

namespace FuncGen {

const char title[] PROGMEM = "FuncGen   ";
const char clear[] PROGMEM = "            ";

void draw() {
  drawTextPgm(0, 16, clear);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

constexpr Module module = {
  title,
  0,
  &draw
};

} // FuncGen
