#include "MCP_DAC.h"
#include <Wire.h>
#include "OutputBuf.h"
#include <lcdgfx.h>

// display: 300 bytes RAM

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
  if (OutputBuf::needNextBuffer()) {
    current = (current == a) ? b : a;
    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      temp_sinePos = (temp_sinePos + 1) % 360;
      current[i].cvA = sine[temp_sinePos];
      current[i].cvB = temp_sinePos * 4; // more or less sawtooth
    }
    OutputBuf::setNextBuffer(current);
  }
}

ISR(TIMER2_COMPA_vect){
  OutputBuf::advance();
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

void setup() {
  //  SPI.usingInterrupt(255); // TODO check if really needed, it seems to just say "noInterrupts()"?
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(14, OUTPUT);

  Serial.begin(115200);

  display.begin();
  Serial.println("Writing");
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  display.printFixed (0,  8, "Hello, world", STYLE_NORMAL);
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

  if (now % 1000 == 0) {
    Serial.println(OutputBuf::overruns);
  }

  if (now - lastTime > 500000)
  {
    count++;
    lastTime = now;
    fillBuffer();
    String txt = String(count);
    drawText(0, 16, txt.c_str());
    fillBuffer();
  }
}
