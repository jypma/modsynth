#pragma once

#include <Arduino.h>
#include "IO.h"
#include "MCP_DAC.h"
#include "Debug.hpp"

extern MCP4821 MCP;

struct OutputFrame {
  uint16_t cv1, cv2, gate1, gate2;
};

#define OUTBUFSIZE 16

namespace OutputBuf {
struct Buffer {
  OutputFrame analog[OUTBUFSIZE];
  uint8_t gate3[OUTBUFSIZE >> 3]; // 1 bit per sample

  void setAll(uint16_t cv1, uint16_t cv2, uint16_t gate1, uint16_t gate2, bool gate3);
  INLINE bool getGate3(uint8_t pos) volatile {
    return gate3[pos >> 3] & (1 << (pos & 7));
  }
  void setGate3(uint8_t pos, bool on);
  void setGate3All(bool on);
};

  extern volatile Buffer *current;
  extern volatile Buffer *next;
  extern volatile uint8_t overruns;
  extern volatile uint8_t currentPos;
  extern volatile uint16_t samples;

  bool needNextBuffer();
  void setNextBuffer(Buffer *buf);

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
    MCP.fastWriteAGain(current->analog[currentPos].cv1);
    MCP.fastWriteBGain(current->analog[currentPos].cv2);
    IO::setGate1Out(current->analog[currentPos].gate1);
    IO::setGate2Out(current->analog[currentPos].gate2);
#ifndef DEBUG_SERIAL
    IO::setGate3Out(current->getGate3(currentPos));
#endif

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
