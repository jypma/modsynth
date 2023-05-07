#include <util/atomic.h>
#include "Arduino.h"
#include "IO.h"
#include "OutputBuf.h"

namespace OutputBuf {

volatile Buffer *current = NULL;
volatile Buffer *next = NULL;
volatile uint8_t currentPos = 0;
volatile uint8_t overruns = 0;
volatile uint16_t samples = 0;

void Buffer::setAll(uint16_t cv1, uint16_t cv2, uint16_t gate1, uint16_t gate2, bool _gate3) {
  OutputFrame *buf = analog;
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = cv1;
    buf->cv2 = cv2;
    buf->gate1 = gate1;
    buf->gate2 = gate2;
    buf++;
  }
  uint8_t g = (_gate3) ? 0xFF : 0;
  for (uint8_t j = 0; j < (OUTBUFSIZE >> 3); j++) {
    gate3[j] = g;
  }
}

void Buffer::setGate3All(bool on) {
  for (uint8_t i = 0; i < (OUTBUFSIZE >> 3); i++) {
    gate3[i] = (on) ? 0xFF : 0;
  }
}

void Buffer::setGate3(uint8_t pos, bool on) {
  const uint8_t bit = (1 << (pos & 7));
  if (on) {
    gate3[pos >> 3] |= bit;
  } else {
    gate3[pos >> 3] &= ~bit;
  }
}


bool needNextBuffer() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    return next == NULL;
  }
  return next == NULL; // only to satisfy compiler warning
}

void setNextBuffer(Buffer *buf) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (current == NULL) {
      current = buf;
      currentPos = 0;
    } else if (next == NULL) {
      next = buf;
    }
  }
}


}
