#include "MCP_DAC.h"
#include <Wire.h>
#include <lcdgfx.h>
#include <Versatile_RotaryEncoder.h>
#include "OutputBuf.h"
#include "FuncGen.h"
#include "Calibrate.h"
#include "IO.h"

// display: 300 bytes RAM

Module currentMod = FuncGen::module;
uint8_t currentModIdx = 0;
uint8_t currentControlIdx = 0;

auto encoder1 = Versatile_RotaryEncoder(2,3,4); // PD2, PD3, PD4
auto encoder2 = Versatile_RotaryEncoder(5,6,7); // PD5, PD6, PD7

OutputBuf::Buffer a, b;
OutputFrame *current = b;

DisplaySSD1306_128x64_I2C display(-1);

MCP4821 MCP;
uint16_t count;
uint32_t lastTime = 0;

// LOOKUP TABLE SINE
uint16_t sine[361];
uint16_t temp_sinePos = 0;

void fillBuffer() {
  if (encoder1.ReadEncoder()) {
    Serial.println("E1");
  } else if (encoder2.ReadEncoder()) {
    Serial.println("E2");
  } else if (OutputBuf::needNextBuffer()) {
    current = (current == a) ? b : a;
    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      temp_sinePos = (temp_sinePos + 1) % 360;
      current[i].cvA = sine[temp_sinePos];
      current[i].cvB = temp_sinePos * 4; // more or less sawtooth
    }
    OutputBuf::setNextBuffer(current);
  }

  IO::readIfNeeded();
}

ISR(TIMER2_COMPA_vect){
  OutputBuf::advance();
}

void setModuleIdx(uint8_t idx) {
  constexpr uint8_t MODULE_COUNT = 2;
  currentModIdx = idx % MODULE_COUNT;
  Serial.println(currentModIdx);
  switch(currentModIdx) {
    case 0: currentMod = FuncGen::module; break;
    default: currentMod = Calibrate::module;
  }
}

void drawText(uint8_t x, uint8_t y, const char *s) {
  if (!*s) return;
  display.setTextCursor(x, y);
  while (*s) {
    fillBuffer();
    display.printChar(*s);
    s++;
  }
}

void drawTextPgm(uint8_t x, uint8_t y, const char *s) {
  uint8_t ch = pgm_read_byte(s);
  if (!ch) return;
  display.setTextCursor(x, y);
  while (ch) {
    fillBuffer();
    display.printChar(ch);
    s++;
    ch = pgm_read_byte(s);
  }
}

void showModule() {
  if (currentControlIdx == 0) {
    drawText(0, 8, ">");
  } else {
    drawText(0, 8, " ");
  }
  drawTextPgm(8, 8, currentMod.name);
}

void handleEncoder1Rotate(int8_t rotation) {
  Serial.println("R1!");
  if (currentControlIdx == 0) {
    setModuleIdx(currentModIdx + rotation);
    showModule();
  }
}

void handleEncoder2Rotate(int8_t rotation) {
  currentControlIdx = (currentControlIdx + 1) % (currentMod.controlCount + 1);
}

void setup() {
  IO::setup();

  // We need input pull-ups for our encoder inputs
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);

  Serial.begin(115200);

  setModuleIdx(0);
  encoder1.setHandleRotate(handleEncoder1Rotate);
  encoder2.setHandleRotate(handleEncoder2Rotate);

  display.begin();
  Serial.println("Writing");
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  showModule();
  Serial.println("Done");

  // fill table with sinus values for fast lookup
  for (int i = 0; i < 361; i++)
  {
    sine[i] = 2047 + round(2047 * sin(i * PI / 180));
  }

  // fill initial buffer
  fillBuffer();

  MCP.begin(10);  // select pin = 10, PB2
  MCP.fastWriteA(0);
  MCP.fastWriteB(0);

  // Set up timer2 for maximum speed
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for ~8kHz increments
  OCR2A = 250;
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

  if (now % 100000 == 0) {
    Serial.println(OutputBuf::overruns);
  }

  if (now - lastTime > 100000)
  {
    currentMod.draw();
  }
}
