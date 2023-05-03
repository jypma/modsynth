#pragma once

#include <Arduino.h>
#include "IO.h"
#include "MCP_DAC.h"

extern MCP4821 MCP;

struct OutputFrame {
  uint16_t cv1, cv2, gate1, gate2;
};

#define OUTBUFSIZE 16

namespace OutputBuf {
  extern volatile OutputFrame *current;
  extern volatile OutputFrame *next;
  extern volatile uint8_t overruns;
  extern volatile uint8_t currentPos;
  extern volatile uint16_t samples;

  typedef OutputFrame Buffer[OUTBUFSIZE];

  bool needNextBuffer();
  void setNextBuffer(OutputFrame *buf);

  // We assume to be already inside an ISR here, so no atomic.
  // One transmission to MCP takes 5us.
  // Whole ISR takes 15us.
  INLINE void advance() {
    samples++;
    if (current == NULL) {
      overruns++;
      return;
    }

    // PC2, pin 16 (for measuring timing)
    //PORTC |= (1 << 2);

    // Output the current frame
    // TODO save a little time by setting CV and D while we're waiting for SPI result.
    MCP.fastWriteAGain(current[currentPos].cv1);
    MCP.fastWriteBGain(current[currentPos].cv2);
    IO::setGate1Out(current[currentPos].gate1);
    IO::setGate2Out(current[currentPos].gate2);

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
