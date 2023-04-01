#pragma once

#include <Arduino.h>
#include <avr/pgmspace.h>

constexpr uint8_t NUM_SCALES = 24;

const char scaleNames[] PROGMEM =
  "Acoustic \0"
  "Aeolian  \0"
  "Altered  \0"
  "Augmented\0"
  "Blues    \0"
  "Chromatic\0"
  "Dorian   \0"
  "Half Dim.\0"
  "Harmo.Maj\0"
  "Harmo.Min\0"
  "Hunga.Maj\0"
  "Hunga.Min\0"
  "Iwato    \0"
  "Locrian  \0"
  "Lydian   \0"
  "Major,Ion\0"
  "Maj.Pent.\0"
  "Min.Pent.\0"
  "Mixolyd. \0"
  "NeapolMaj\0"
  "NeapolMin\0"
  "Phrygian \0"
  "UkrDorian\0"
  "WholeTone\0"
  ;

const uint16_t scales[] PROGMEM = {
  //C D EF G A B
  0b101010110110,
  0b101100111001,
  0b110100101010,
  0b100110011001,
  0b100101110010,
  0b111111111111,
  0b101101010110,
  0b101101101010,
  0b101011011001,
  0b101101011001,
  0b100110110110,
  0b101100111001,
  0b110001100010,
  0b110101101010,
  0b101010110101,
  0b101011010101,
  0b101010010100,
  0b100101010010,
  0b101011010110,
  0b110101010101,
  0b110101011001,
  0b110101101010,
  0b101100110110,
  0b101010101010,
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

  uint16_t transposeUp(uint16_t notes, uint8_t n) {
    return ((notes >> n) | (notes << (12 - n))) & 0b111111111111;
  }
 public:
  Scale(uint8_t rootNote, uint8_t _scaleIdx): scaleIdx(_scaleIdx) {
    uint16_t enabledNotes = transposeUp(getNotesFor(scaleIdx), rootNote);
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
    return scaleNames + (scaleIdx * 10);
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
