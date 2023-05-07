#pragma once
#include <Arduino.h>
#include "OutputBuf.h"

constexpr uint16_t SAMPLERATE = 12195; // To limit rounding errors when calculating OCR2A
constexpr uint8_t MODULE_COUNT = 6;

// TODO replace with clearChars
const char clear[] PROGMEM = "                ";

void setModuleIdx(int8_t idx);
extern uint8_t currentControlIdx;
void clearChars(uint8_t x, uint8_t y, uint8_t count);
void drawSelected(uint8_t x, uint8_t y, uint8_t control);
void drawChar(uint8_t x, uint8_t y, char ch);
void drawDecimal(uint8_t x, uint8_t y, int16_t value);
void drawDecimal(uint8_t x, uint8_t y, int16_t value, uint8_t expectedChars);
void drawDecimal(uint8_t x, uint8_t y, int16_t value, char suffix, uint8_t expectedChars);
void drawText(uint8_t x, uint8_t y, const char *s);
void drawTextPgm(uint8_t x, uint8_t y, const char *s);

struct Module {
  Module() = delete;
  using Callback = void (*)();
  using AdjustCallback = void (*)(int8_t);
  using PresetCallback = void (*)(uint16_t);
  using BufferCallback = void (*)(OutputBuf::Buffer &);

  const char *name;
  uint8_t controlCount;
  Callback draw;
  Callback start;
  Callback stop;
  AdjustCallback adjust;
  BufferCallback fillBuffer;
  Callback adjustPressed;
  PresetCallback save;
  PresetCallback load;
};

extern Module currentMod;
extern uint8_t currentModIdx;
