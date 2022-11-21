#pragma once
#include <Arduino.h>

extern uint8_t currentControlIdx;
void drawText(uint8_t x, uint8_t y, const char *s);
void drawTextPgm(uint8_t x, uint8_t y, const char *s);

struct Module {
  Module() = delete;
  using Callback = void (*)();

  const char *name;
  uint8_t controlCount;
  Callback draw;
};
