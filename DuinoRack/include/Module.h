#pragma once
#include <Arduino.h>
#include "OutputBuf.h"

constexpr uint16_t SAMPLERATE = 12000;

const char clear[] PROGMEM = "                ";

extern uint8_t currentControlIdx;
void drawDecimal(uint8_t x, uint8_t y, int16_t value);
void drawText(uint8_t x, uint8_t y, const char *s);
void drawTextPgm(uint8_t x, uint8_t y, const char *s);

struct Module {
  Module() = delete;
  using Callback = void (*)();
  using AdjustCallback = void (*)(int8_t);
  using BufferCallback = void (*)(OutputFrame *);

  const char *name;
  uint8_t controlCount;
  Callback draw;
  AdjustCallback adjust;
  BufferCallback fillBuffer;
};
