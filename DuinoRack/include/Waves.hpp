#pragma once

#include "Arduino.h"
#include "Inline.h"
#include "mozzi_pgmspace.h"
#include "tables/sin256.h"

namespace Waves {
  constexpr uint8_t posScaleBits = 14;
  constexpr uint32_t posFractionMask = (uint32_t(1) << posScaleBits) - 1;
  constexpr uint32_t maxFractionPos = (uint32_t(255) << posScaleBits) + posFractionMask;

  template<const int16_t *table, uint8_t tablePosUnusedBits = 0>
  class Wave {
  public:
    /** Gets an exact table value */
    static INLINE int16_t get(uint8_t pos) {
      return FLASH_OR_RAM_READ(table + (pos >> tablePosUnusedBits));
    }

    /** Gets and interpolates a value between two table values. */
    static int16_t get(uint32_t pos) {
      uint8_t idx1 = pos >> posScaleBits;
      // Let idx wrap around since table is 256 entries
      uint8_t idx2 = idx1 + 1;

      // TODO cache v1, v2, delta for more performance, while idx doesn't change.
      int16_t v1 = FLASH_OR_RAM_READ(table + idx1);
      int16_t v2 = FLASH_OR_RAM_READ(table + idx2);
      int16_t delta = (v2 > v1) ? v2 - v1 : -(v1 - v2);
      // TODO only use right-most part of [pos] for the logical and, since posScaleBits is less than 16.
      int16_t fraction = (int32_t(delta) * (pos & posFractionMask)) >> posScaleBits;

      return v1 + fraction;
    }
  };

  namespace SawUp {
    inline int16_t get(uint8_t pos) {
      uint16_t intermediate = (pos * 125) >> 2; // 0..32000, divided by 4 is 0..8000
      return int16_t(intermediate) - 4000;
    }

    inline int16_t get(uint32_t pos) {
      uint32_t intermediate = pos >> (posScaleBits - 8); // now 8 bits pos, 8 bits fraction, i.e. 0..65536
      uint32_t scaled = (intermediate * 8000) >> 16;     // Now 0..8000
      return int16_t(scaled) - 4000;
    }
  }

  namespace SawDown {
    inline int16_t get(uint8_t pos) {
      uint16_t intermediate = ((255 - pos) * 125) >> 2; // 0..32000, divided by 4 is 0..8000
      return int16_t(intermediate) - 4000;
    }

    inline int16_t get(uint32_t pos) {
      uint32_t intermediate = (maxFractionPos - pos) >> (posScaleBits - 8); // now 8 bits pos, 8 bits fraction, i.e. 0..65536
      uint32_t scaled = (intermediate * 8000) >> 16;     // Now 0..8000
      return int16_t(scaled) - 4000;
    }
  }

  namespace Triangle {
    inline int16_t get(uint8_t pos) {
      if (pos < 64) {
        return (int16_t(pos) * 62);
      } else if (pos < 192) {
        //return 1000;
        return ((int16_t(128) - pos) * 62);
      } else {
        return ((int16_t(pos) - 256) * 62);
      }
    }

    constexpr uint32_t q2pos = (uint32_t(128) << posScaleBits);
    constexpr uint32_t q3pos = (uint32_t(192) << posScaleBits);
    constexpr uint32_t q4pos = (uint32_t(256) << posScaleBits);
    inline int16_t get(uint32_t pos) {
      uint8_t p = pos >> posScaleBits;
      if (p < 64) {
        uint32_t intermediate = pos >> (posScaleBits - 8); // now 8 bits pos, 8 bits fraction, i.e. 0..65536
        uint32_t scaled = (intermediate * 8000) >> 15;
        return int16_t(scaled);
      } else if (p < 192) {
        uint32_t intermediate = (q2pos - pos) >> (posScaleBits - 8); // now 8 bits pos, 8 bits fraction, i.e. 0..65536
        uint32_t scaled = (intermediate * 8000) >> 15;
        return int16_t(scaled);
      } else {
        uint32_t intermediate = (pos - q4pos) >> (posScaleBits - 8); // now 8 bits pos, 8 bits fraction, i.e. 0..65536
        uint32_t scaled = (intermediate * 8000) >> 15;
        return int16_t(scaled);
      }
    }
  }

  typedef Wave<SIN_DATA> Sine;
}
