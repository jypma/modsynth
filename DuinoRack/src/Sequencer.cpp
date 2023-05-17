#include "Sequencer.hpp"
#include "Random.hpp"

namespace Sequencer {

const char title[] PROGMEM = "Sequencer";
Sequencer sequencer;

bool probToGate(uint8_t prob) {
  return (prob == 0) ? false : (prob == 255) ? true : (Random::nextByte() <= prob);
}

}
