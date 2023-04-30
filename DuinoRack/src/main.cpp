#include "MCP_DAC.h"
#include <lcdgfx.h>
#include <Versatile_RotaryEncoder.h>
#include "Module.h"
#include "OutputBuf.h"
#include "FuncGen.h"
#include "Calibrate.h"
#include "ADSR.hpp"
#include "IO.h"
#include "LFO.h"
#include "Quantize.h"
#include "MIDIMod.h"
#include "canvas/canvas.h"
#include "pins_arduino.h"
#include "Storage.hpp"
#include "Debug.hpp"

// display: 300 bytes RAM

Module currentMod = FuncGen::module;
uint8_t shownModIdx = 0;
uint8_t currentModIdx = 0;
uint8_t currentControlIdx = 0;

// Encoder 1 is "Select"
auto encoder1 = Versatile_RotaryEncoder(2,3,4); // PD2, PD3, PD4
// Encoder 2 is "Adjust"
auto encoder2 = Versatile_RotaryEncoder(5,6,7); // PD5, PD6, PD7

OutputBuf::Buffer a, b;
OutputFrame *current = b;

DisplaySSD1306_128x64_I2C display(-1);

MCP4821 MCP;
uint16_t count;
uint32_t lastTime = 0;
uint8_t oldOverruns = 0;

void fillBuffer() {
  if (OutputBuf::needNextBuffer()) {
    current = (current == a) ? b : a;
    currentMod.fillBuffer(current);
    OutputBuf::setNextBuffer(current);
  } else if (encoder1.ReadEncoder()) {
    debugSerial("E1");
  } else if (encoder2.ReadEncoder()) {
    debugSerial("E2");
  } else {
    IO::readIfNeeded();
  }
}

ISR(TIMER2_COMPA_vect){
  OutputBuf::advance();
}

void setModuleIdx(int8_t idx) {
  currentMod.stop();
  currentControlIdx = 0;
  currentModIdx = idx % MODULE_COUNT;
  debugSerial(currentModIdx);
  switch(currentModIdx) {
    case 0: currentMod = FuncGen::module; break;
    case 1: currentMod = ADSR::module; break;
    case 2: currentMod = MIDIMod::module; break;
    case 3: currentMod = LFO::module; break;
    case 4: currentMod = Quantize::module; break;
    default: currentMod = Calibrate::module;
  }
  // Startup trigger
  currentMod.start();
  currentMod.adjust(0);
}

void drawText(uint8_t x, uint8_t y, const char *s) {
  if (!*s) return;
  display.setTextCursor(x, y);
  while (*s) {
    fillBuffer();
    display.printChar(*s);
    fillBuffer();
    s++;
  }
}

void drawSelected(uint8_t x, uint8_t y, uint8_t control) {
  drawChar(x, y, (currentControlIdx == control) ? '>' : ' ');
}

void drawChar(uint8_t x, uint8_t y, char ch) {
  if (!ch) return;
  display.setTextCursor(x, y);
  fillBuffer();
  display.printChar(ch);
}

void clearChars(uint8_t x, uint8_t y, uint8_t count) {
  display.setTextCursor(x, y);
  while (count > 0) {
    fillBuffer();
    display.printChar(' ');
    count--;
  }
}

void drawDecimal(uint8_t x, uint8_t y, int16_t value) {
  char str[7];
  itoa(value, str, 10);
  drawText(x, y, str);
  drawTextPgm(x + strlen(str) * 6, y, clear);
}

void drawDecimal(uint8_t x, uint8_t y, int16_t value, uint8_t expectedChars) {
  char str[7];
  itoa(value, str, 10);
  drawText(x, y, str);
  uint8_t len = strlen(str);
  clearChars(x + len * 6, y, (expectedChars > len) ? expectedChars - len : 0);
}

void drawDecimal(uint8_t x, uint8_t y, int16_t value, char suffix, uint8_t expectedChars) {
  char str[7];
  itoa(value, str, 10);
  drawText(x, y, str);
  uint8_t len = strlen(str);
  drawChar(x + len * 6, y, suffix);
  len++;
  clearChars(x + len * 6, y, (expectedChars > len) ? expectedChars - len : 0);
}

void drawTextPgm(uint8_t x, uint8_t y, const char *s) {
  uint8_t ch = pgm_read_byte(s);
  if (!ch) return;
  display.setTextCursor(x, y);
  while (ch) {
    fillBuffer();
    display.printChar(ch);
    fillBuffer();
    s++;
    ch = pgm_read_byte(s);
  }
}

bool showLongPress = false;
void showModule() {
  if (shownModIdx != currentModIdx) {
    display.fill(0x00);
  }
  if (currentControlIdx == 0) {
    drawText(0, 8, ">");
  } else {
    drawText(0, 8, " ");
  }
  drawText(120, 8, showLongPress ? "S" : " ");
  drawTextPgm(8, 8, currentMod.name);
  shownModIdx = currentModIdx;
}

// Encoder 2 is "Adjust"
void handleEncoder2Rotate(int8_t rotation) {
  static uint32_t prevTime = 0;
  static uint8_t accel = 1;
  auto time = millis();
  uint8_t maxAccel = (currentControlIdx == 0) ? 1 : 16;
  if ((time - prevTime < 100) && (accel < maxAccel)) {
    accel <<= 1;
  } else {
    accel = 1;
  }
  prevTime = time;

  // Don't draw to screen here, as this may be called in the middle of drawing to the screen!
  if (currentControlIdx == 0) {
    setModuleIdx(applyDelta<uint8_t>(currentModIdx, rotation, 0, MODULE_COUNT - 1));
  } else {
    // TODO control acceleration differently for different controls
    rotation *= accel;
    currentMod.adjust(rotation);
  }
}

void handleEncoder2PressRelease() {
  showLongPress = false;
  if (currentMod.adjustPressed != NULL) {
    currentMod.adjustPressed();
  }
}

// Encoder 1 is "Select"
void handleEncoder1Rotate(int8_t rotation) {
  static uint32_t prevTime = 0;
  static uint8_t accel = 1;
  auto time = millis();
  uint8_t maxAccel = 4;
  if ((time - prevTime < 100) && (accel < maxAccel)) {
    accel <<= 1;
  } else {
    accel = 1;
  }
  prevTime = time;
  rotation *= accel;

  // Don't draw to screen here, as this may be called in the middle of drawing to the screen!
  currentControlIdx = (currentControlIdx + (currentMod.controlCount + 1) + rotation) % (currentMod.controlCount + 1);
}


void handleEncoder1LongPress() {
  showLongPress = true;
}

void handleEncoder1LongPressRelease() {
  // TODO: Show a menu here to load/save preset slots. For now, we just save in preset 0.
  Storage::savePreset(0);
  showLongPress = false;
}

void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(31250);
  debugSerial("DuinoRack");
  Serial.flush();
#endif
  IO::setup();
  Storage::load();

  // We need input pull-ups for our encoder inputs
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);

  currentMod.start();
  encoder1.setHandleRotate(handleEncoder1Rotate);
  encoder1.setHandleLongPress(handleEncoder1LongPress);
  encoder1.setHandleLongPressRelease(handleEncoder1LongPressRelease);
  encoder2.setHandleRotate(handleEncoder2Rotate);
  encoder2.setHandlePressRelease(handleEncoder2PressRelease);

  display.begin();
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  showModule();
  debugSerial("Display ready.");

  // fill initial buffer
  fillBuffer();

  MCP.begin(PIN_A3);  // select pin = 17, PC3 / A3 / D17
  MCP.fastWriteA(0);
  MCP.fastWriteB(0);

  // Set up timer2 for maximum speed
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register to sample rate (def. 12000)
  constexpr uint8_t ovfValue = uint32_t(F_CPU) / 8 / SAMPLERATE;
  OCR2A = ovfValue;
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // 8 prescaler
  TCCR2B = (1 << CS21);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}

void loop() {
  fillBuffer();
  uint32_t now = micros();
  fillBuffer();

  if (OutputBuf::overruns != oldOverruns) {
    oldOverruns = OutputBuf::overruns;
    // TODO show XRun on screen
    debugSerial("Xrun! ");
    debugSerial(oldOverruns);
  }

  if (now - lastTime > 100000)
  {
    showModule();
    fillBuffer();
    currentMod.draw();
  }
}
