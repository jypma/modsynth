#pragma once

#include <Arduino.h>
#include <avr/eeprom.h>

namespace Storage {

template <typename T, size_t size> struct OpsImpl {};

template <typename T> struct OpsImpl<T,1> {
  static void read(uint16_t addr, T &target) {
    target = static_cast<T>(eeprom_read_byte( (uint8_t*) addr ));
  }

  static void write(uint16_t addr, const T &target) {
    T current;
    read(addr, current);
    if (current != target) {
      eeprom_write_byte((uint8_t *) addr, target);
    }
  }
};

template <typename T> struct OpsImpl<T,2> {
  static void read(uint16_t addr, T &target) {
    target = eeprom_read_word( (uint16_t*) addr );
  }

  static void write(uint16_t addr, const T &target) {
    T current;
    read(addr, current);
    if (current != target) {
      eeprom_write_word( (uint16_t*) addr, target);
    }
  }
};

template <typename T>
void read(uint16_t addr, T &target) {
  return OpsImpl<T, sizeof(T)>::read(addr, target);
}

template <typename T>
void write(uint16_t addr, const T &target) {
  OpsImpl<T, sizeof(T)>::write(addr, target);
}

void savePreset(uint8_t index);
void loadPreset(uint8_t index);
void load();

}
