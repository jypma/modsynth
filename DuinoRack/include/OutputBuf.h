#pragma once

#include <Arduino.h>
#include "IO.h"
#include "MCP_DAC.h"

extern MCP4821 MCP;

struct OutputFrame {
  // FIXME rename to cvOut1 etc., to align with methods in IO::.
  uint16_t cvA, cvB, cvC, cvD;
};

#define OUTBUFSIZE 16

namespace OutputBuf {
  extern volatile OutputFrame *current;
  extern volatile OutputFrame *next;
  extern volatile uint8_t overruns;
  extern volatile uint8_t currentPos;

  typedef OutputFrame Buffer[OUTBUFSIZE];

  bool needNextBuffer();
  void setNextBuffer(OutputFrame *buf);

  // We assume to be already inside an ISR here, so no atomic.
  // One transmission to MCP takes 5us.
  // Whole ISR takes 15us.
  INLINE void advance() {
    if (current == NULL) {
      overruns++;
      return;
    }

    // PC2, pin 16 (for measuring timing)
    //PORTC |= (1 << 2);

    // Output the current frame
    // TODO save a little time by setting CV and D while we're waiting for SPI result.
    MCP.fastWriteAGain(current[currentPos].cvA);
    MCP.fastWriteBGain(current[currentPos].cvB);
    // TODO inline setting the PWMs
    IO::setGate1PWM(current[currentPos].cvC);
    IO::setGate2PWM(current[currentPos].cvD);

    // Advance to the next frame or buffer
    if (currentPos < OUTBUFSIZE - 1) {
      currentPos++;
    } else if (next != NULL) {
      currentPos = 0;
      current = next;
      next = NULL;
    } else {
      current = NULL;
    }

    // PC2, pin 14
    //PORTC &= ~(1 << 2);
  }
}
