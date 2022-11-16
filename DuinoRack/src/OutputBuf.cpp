#include <util/atomic.h>
#include "OutputBuf.h"

namespace OutputBuf {

volatile OutputFrame *current = NULL;
volatile OutputFrame *next = NULL;
volatile uint8_t currentPos = 0;
volatile uint8_t overruns = 0;

bool needNextBuffer() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    return next == NULL;
  }
  return next == NULL; // only to satisfy compiler warning
}

void setNextBuffer(OutputFrame *buf) {
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
