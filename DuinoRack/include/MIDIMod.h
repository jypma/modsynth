#pragma once

#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <midiXparser.h>

#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "Debug.hpp"

namespace MIDIMod {

  //MIDI_CREATE_DEFAULT_INSTANCE();

const char title[] PROGMEM = "MIDI      ";
const char noteLbl[] PROGMEM = "Note: ";

uint8_t noteCount = 0;
uint8_t currentNote = 0;
uint16_t out1 = 0;
uint16_t out2 = 0;
uint16_t gate1 = 0;
midiXparser midiParser;

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

void fillBuffer(OutputBuf::Buffer &buf) {
  // TODO: Decide if we have time to handle multiple MIDI messages in 1 fillBuffer()
  if (Serial.available()) {
    uint8_t in = Serial.read();
    if (midiParser.parse(in)) {
      if (midiParser.isMidiStatus(midiXparser::noteOnStatus)) {
        noteCount++;
        currentNote = midiParser.getMidiMsg()[1] - 21;
        debugSerial(currentNote);
        gate1 = IO::calcGate1Out(8000);
        out1 = IO::calcCV1Out(getCV(currentNote));
        debugSerial(out1);
      } else if (midiParser.isMidiStatus(midiXparser::noteOffStatus)) {
        if (noteCount > 0) {
          noteCount--;
        }
        if (noteCount == 0) {
          gate1 = IO::calcGate1Out(0);
        }
      }
    }
  }

  uint16_t gate2 = IO::calcGate2Out(0);
  buf.setAll(out1, out2, gate1, gate2, false);
}

void start() {
  midiParser.setMidiMsgFilter(midiXparser::channelVoiceMsgTypeMsk);
  noteCount = 0;
  out1 = IO::calcCV1Out(0);
  out2 = IO::calcCV2Out(0);
#ifndef DEBUG_SERIAL
  Serial.begin(31250);
#endif
}

void stop() {
#ifndef DEBUG_SERIAL
  Serial.end();
#endif
}

constexpr Module module = {
  title,
  0,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer,
  NULL
};

}
