#pragma once
#include <Arduino.h>

const char clear[] PROGMEM = "                ";

extern uint8_t currentControlIdx;
void drawDecimal(uint8_t x, uint8_t y, int16_t value);
void drawText(uint8_t x, uint8_t y, const char *s);
void drawTextPgm(uint8_t x, uint8_t y, const char *s);

struct Module {
  Module() = delete;
  using Callback = void (*)();
  using AdjustCallback = void (*)(int8_t);

  const char *name;
  uint8_t controlCount;
  Callback draw;
  AdjustCallback adjust;
};
