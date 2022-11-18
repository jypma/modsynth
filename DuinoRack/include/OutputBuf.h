#pragma once

#include <Arduino.h>
#include "MCP_DAC.h"

extern MCP4821 MCP;

struct OutputFrame {
  uint16_t cvA, cvB;
  uint8_t gates;
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
  // ISR is currently called every 125us (8kHz)
  INLINE void advance() {
    if (current == NULL) {
      overruns++;
      return;
    }

    // PC0, pin 14 (for measuring timing)
    // PORTC |= (1 << 0);

    // Output the current frame
    MCP.fastWriteAGain(current[currentPos].cvA);
    MCP.fastWriteBGain(current[currentPos].cvB);

    // PC1, pin 15
    if ((current[currentPos].gates & 1) != 0) {
      PORTC |= (1 << 1);
    } else {
      PORTC &= ~(1 << 1);
    }

    // PC2, pin 16
    if ((current[currentPos].gates & 2) != 0) {
      PORTC |= (1 << 2);
    } else {
      PORTC &= ~(1 << 2);
    }

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

    // PC0, pin 14
    //PORTC &= ~(1 << 0);
  }
}
