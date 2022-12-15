#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <MIDI.h>

#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "midi_Defs.h"

namespace MIDIMod {

MIDI_CREATE_DEFAULT_INSTANCE();

const char title[] PROGMEM = "MIDI      ";
const char noteLbl[] PROGMEM = "Note: ";

uint8_t noteCount = 0;
uint8_t currentNote = 0;
uint16_t outA = 0;
uint16_t outB = 0;

void draw() {
  drawTextPgm(0, 16, noteLbl);
  drawDecimal(40, 16, currentNote);
  drawTextPgm(0, 24, clear);
  drawTextPgm(0, 32, clear);
}

void adjust(int8_t d) {
}

constexpr int16_t getCV(uint8_t note) {
  return uint16_t(note) * 1000 / 12;
}

void fillBuffer(OutputFrame *buf) {
  if (MIDI.read()) {
    switch (MIDI.getType()) {
      case midi::NoteOn:
        noteCount++;
        currentNote = MIDI.getData1() - 21;
        Serial.println(currentNote);
        IO::setGate1Out(true);
        outA = IO::calcCV1Out(getCV(currentNote));
        Serial.println(outA);
        break;

      case midi::NoteOff:
        if (noteCount > 0) {
          noteCount--;
        }
        if (noteCount == 0) {
          IO::setGate1Out(false);
        }
        break;

      default:
        // Ignore other messages.
        break;
    }
  }

  // TODO: Decide if we have time to handle multiple MIDI messages in 1 fillBuffer()

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cvA = outA;
    buf->cvB = outB;
    buf++;
  }
}

void start() {
  MIDI.begin(1); // Listen on channel 1
  MIDI.turnThruOff(); // for now.
  noteCount = 0;
  outA = IO::calcCV1Out(0);
  outB = IO::calcCV2Out(0);
}

void stop() {
}

constexpr Module module = {
  title,
  0,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer
};

}
