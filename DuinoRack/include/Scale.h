#pragma once

#include <Arduino.h>
#include <avr/pgmspace.h>

const char scale0[] PROGMEM = "Chromatic";
const char scale1[] PROGMEM = "Major    ";

constexpr uint8_t NUM_SCALES = 2;

const uint16_t scales[] PROGMEM = {
  0b111111111111,
  0b101011010101
};

int16_t noteToVoltage(uint8_t note) {
  uint8_t octave = note / 12;
  uint8_t n = note % 12;
  return uint16_t(1000) * octave + (uint16_t(833) * n) / 10;
}

class Scale {
  uint8_t scaleIdx;
  int16_t thresholds[13];
  uint8_t targetNotes[13];
  uint8_t notes;

  uint16_t getNotesFor(uint8_t idx) {
    if (idx >= NUM_SCALES) {
      idx = 0;
    }
    return pgm_read_word(scales + idx);
  }
 public:
  Scale(uint8_t _scaleIdx): scaleIdx(_scaleIdx) {
    uint16_t enabledNotes = getNotesFor(scaleIdx);
    notes = 0;
    for (uint8_t note = 0; note < 12; note++) {
      // We write the bits left to right, but the largest bit is actually note 0 (C)
      if ((enabledNotes & (0b100000000000 >> note)) != 0) {
        targetNotes[notes] = note;
        notes++;
      }
    }

    if (notes == 0) {
      notes = 1;
      thresholds[0] = 0;
      targetNotes[0] = 0;
    } else if (notes == 1) {
      thresholds[0] = 0;
    } else { // notes > 1
      for (uint8_t n = 0; n < notes; n++) {
        const uint8_t note = targetNotes[n];
        const uint8_t prev = targetNotes[(n + notes - 1) % notes];
        const int16_t note_mV = noteToVoltage(note);
        int16_t prev_mV = noteToVoltage(prev);
        if (n == 0) {
          // prev will be an octave lower
          prev_mV -= 1000;
        }
        int16_t threshold = (note_mV + prev_mV) / 2;
        thresholds[n] = threshold;
      }

      if (thresholds[0] < 0) {
        targetNotes[notes] = targetNotes[0] + 12;
        thresholds[notes] = thresholds[0] + 1000;
        thresholds[0] = 0;
        notes++;
      }
    }
  }

  uint8_t quantize(int16_t mV) {
    if (mV < 0) { mV = 0; }
    uint8_t octave = mV / 1000;
    mV %= 1000;
    uint8_t note = notes - 1;
    while (note >= 0 && mV < thresholds[note]) note--;
    return targetNotes[note] + (octave * 12);
  }

  const char *getLabel() {
    switch (scaleIdx) {
      case 0: return scale0;
      case 1: return scale1;
      default: return scale0;
    }
  }

  void print() {
    Serial.println("notes: ");
    for (uint8_t n = 0; n < notes; n++) {
      const uint8_t note = targetNotes[n];
      const uint8_t prev = targetNotes[(n + notes - 1) % notes];
      const int16_t note_mV = noteToVoltage(note);
      int16_t prev_mV = noteToVoltage(prev);
      Serial.println(note);
      Serial.print("mV: ");
      Serial.print(note_mV);
      Serial.print(", prev mV:");
      Serial.println(prev_mV);
    }
    Serial.println("thresholds:");
    for (uint8_t n = 0; n < notes; n++) {
      Serial.println(thresholds[n]);
    }
  }
};
