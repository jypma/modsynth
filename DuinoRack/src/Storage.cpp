#include "Storage.hpp"
#include "IO.h"
#include <EEPROM.h>

namespace Storage {
  constexpr uint16_t MAGIC = 0x47A7;

  void load() {
    uint16_t magic;
    EEPROM.get(0, magic);
    uint8_t version;
    EEPROM.get(2, version);

    if (magic != MAGIC || version != 1) {
      Serial.println("Loading defaults");
      // Write defaults
      IO::saveCalibration();
      EEPROM.put(0, MAGIC);
      version = 1;
      EEPROM.put(2, version);
    } else {
      Serial.println("Loading EEPROM");
      IO::loadCalibration();
    }
  }
}
