#include "Random.hpp"

namespace Random {

int32_t x = 0x01457AF3;
constexpr uint32_t M = 0x7FFFFFFF;

uint32_t getNext() {  //random number generator; call with 1 <= x <=M-1
  x = (x >> 16) + ((x << 15) & M)  - (x >> 21) - ((x << 10) & M);
  //if (x < 0) x += M;
  return x;
}

void addSeed(uint8_t seed) { x ^= (uint32_t(seed) << uint8_t(seed & 0x0F)); }

void addSeed(uint16_t seed) { x ^= (uint32_t(seed) << uint8_t(seed & 0x07)); }

uint8_t nextByte() {
  // TODO we can call this 3 times with next byte before calling getNext()
  return getNext();
}

uint32_t nextLong() { return getNext(); }

} // namespace Random

