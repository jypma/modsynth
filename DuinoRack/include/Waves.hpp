#pragma once

#include "Arduino.h"
#include "Inline.h"
#include "mozzi_pgmspace.h"
#include "tables/sin256.h"
#include "tables/ev_pw2_128.h"
#include "tables/ev_pw3_128.h"
#include "tables/ev_rt3_128.h"
#include "tables/ev_sin128.h"

namespace Waves {
  constexpr uint16_t TABLE_SIZE = 256;

  constexpr uint8_t posScaleBits = 14;
  constexpr uint32_t posFractionMask = (uint32_t(1) << posScaleBits) - 1;
  constexpr uint32_t maxFractionPos = (uint32_t(255) << posScaleBits) + posFractionMask;

  constexpr uint32_t q1pos = (uint32_t(64) << posScaleBits);
  constexpr uint32_t q2pos = (uint32_t(128) << posScaleBits);
  constexpr uint32_t q3pos = (uint32_t(192) << posScaleBits);
  constexpr uint32_t q4pos = (uint32_t(256) << posScaleBits);
  constexpr uint32_t maxPos = q4pos;

  constexpr uint32_t toPos(uint8_t tablePos) {
    return uint32_t(tablePos) << posScaleBits;
  }

  namespace Internal {
    template<typename T, uint8_t tablePosUnusedBits>
    static INLINE T waveGet(const T *table, uint8_t pos) {
      return FLASH_OR_RAM_READ(table + (pos >> tablePosUnusedBits));
    }

  template<typename T, uint8_t tablePosUnusedBits>
  static T waveGet(const T *table, uint32_t pos) {
    uint8_t idx1 = (pos >> posScaleBits);
    // Let idx wrap around since table is 256 entries
    uint8_t idx2 = idx1 + 1;

    // TODO cache v1, v2, delta for more performance, while idx doesn't change.
    T v1 = waveGet<T,tablePosUnusedBits>(table, idx1);
    T v2 = waveGet<T,tablePosUnusedBits>(table, idx2);
    T delta = (v2 > v1) ? v2 - v1 : -(v1 - v2);
    // TODO only use right-most part of [pos] for the logical and, since posScaleBits is less than 16.
    T fraction = (int32_t(delta) * (pos & posFractionMask)) >> posScaleBits;

    return v1 + fraction;
  }
  }

  template<typename T, const T *table, uint8_t tablePosUnusedBits = 0>
  class Wave {
  public:
    /** Gets an exact table value */
    static INLINE T get(uint8_t pos) {
      return Internal::waveGet<T, tablePosUnusedBits>(table, pos);
    }

    /** Gets and interpolates a value between two table values. */
    static T get(uint32_t pos) {
      return Internal::waveGet<T, tablePosUnusedBits>(table, pos);
    }
  };

  namespace Square {
    inline int16_t get(uint8_t pos) {
      return (pos < 128) ? 4000 : -4000;
    }

  inline int16_t get(uint32_t pos) {
    return (pos < q2pos) ? 4000 : -4000;
  }

  }

  namespace SawUp {
    inline int16_t get(uint8_t pos) {
      uint16_t intermediate = (pos * 125) >> 2; // 0..32000, divided by 4 is 0..8000
      return int16_t(intermediate) - 4000;
    }

    inline int16_t get(uint32_t pos) {
      // now 8 bits pos, 8 bits fraction, i.e. 0..65536
      uint16_t intermediate = uint16_t(pos >> (posScaleBits - 8));
      uint32_t scaled = (uint32_t(intermediate) * 8000) >> 16;     // Now 0..8000
      return int16_t(scaled) - 4000;
    }
  }

  namespace SawDown {
    inline int16_t get(uint8_t pos) {
      uint16_t intermediate = ((255 - pos) * 125) >> 2; // 0..32000, divided by 4 is 0..8000
      return int16_t(intermediate) - 4000;
    }

    inline int16_t get(uint32_t pos) {
      // now 8 bits pos, 8 bits fraction, i.e. 0..65536
      uint16_t intermediate = uint16_t((maxFractionPos - pos) >> (posScaleBits - 8));
      uint32_t scaled = (uint32_t(intermediate) * 8000) >> 16;     // Now 0..8000
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

    inline int16_t get(uint32_t pos) {
      uint8_t p = (pos >> posScaleBits);
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

  typedef Wave<int16_t, SIN_DATA> Sine;
  typedef Wave<uint16_t, EV_PW3_128_DATA, 1> EnvelopePower3;
  typedef Wave<uint16_t, EV_PW2_128_DATA, 1> EnvelopePower2;
  typedef Wave<uint16_t, EV_RT3_128_DATA, 1> EnvelopeRoot3;
  typedef Wave<uint16_t, EV_SIN128_DATA, 1> EnvelopeSin;
}
