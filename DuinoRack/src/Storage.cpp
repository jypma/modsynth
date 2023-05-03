#include "Storage.hpp"
#include "Arduino.h"
#include "IO.h"
#include "Module.h"
#include <EEPROM.h>
#include "Debug.hpp"
#include <avr/eeprom.h>

namespace Storage {

uint16_t getPresetAddr(uint8_t index) {
  return (index == 0) ? 256 : (index == 1) ? 512  : 768;
}

void savePreset(uint8_t index) {
  if (!currentMod.save) return;
  uint16_t addr = getPresetAddr(index);
  uint8_t module = currentModIdx;
  EEPROM.put(addr, module);
  currentMod.save(addr + 1);
  debugSerial("Saved preset");
}

void loadPreset(uint8_t index) {
  uint16_t addr = getPresetAddr(index);
  uint8_t module;
  Storage::read(addr, module);
  if (module < MODULE_COUNT) {
    setModuleIdx(module);
    if (!currentMod.load) return;
    currentMod.load(addr + 1);
  }
}

constexpr uint16_t MAGIC = 0x47A7;

void load() {
  uint16_t magic;
  Storage::read(0, magic);
  uint8_t version;
  Storage::read(2, version);

  // Hold down encoder 2 ("Adjust") during boot to factory reset
  if (magic != MAGIC || version != 1 || !digitalRead(7)) {
    debugSerial("Loading defaults");
    // Write defaults
    IO::saveCalibration();
    EEPROM.put(0, MAGIC);
    version = 1;
    EEPROM.put(2, version);
    // Put the default startup module in all 3 presets
    savePreset(0);
    savePreset(1);
    savePreset(2);
  } else {
    debugSerial("Loading EEPROM");
    IO::loadCalibration();
    loadPreset(0);
  }
}

}
