#pragma once

#include <stdint.h>

namespace Random {
void addSeed(uint8_t seed);
void addSeed(uint16_t seed);
uint32_t nextLong();
uint8_t nextByte();
}
