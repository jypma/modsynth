#pragma once

#include <avr/eeprom.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include "Arduino.h"

#include "Module.h"
#include "IO.h"
#include "OutputBuf.h"
#include "Scale.h"

namespace Quantize {

const char title[] PROGMEM = "Quantizer";
const char notenames[] PROGMEM = " CC# DD# E FF# GG# AA# B";
const char scale_t[] PROGMEM = "Scale:";
const char titleA[] PROGMEM = "Note 1: ";
const char titleB[] PROGMEM = "Note 2: ";

uint8_t scaleIdx = 0;
uint8_t rootNote = 0;
auto scale = Scale(rootNote, scaleIdx);

uint8_t noteA;
uint8_t noteB;

void formatNote(uint8_t note, char *txt)
{
  uint8_t octave = note / 12;
  uint8_t n = note % 12;
  txt[0] = pgm_read_byte(notenames + (n*2));
  txt[1] = pgm_read_byte(notenames + (n*2) + 1);
  txt[2] = '1' + octave;
  txt[3] = 0;
}

void draw() {
  char txt[5];
  drawTextPgm(0, 16, scale_t);
  drawSelected(40, 16, 1);
  formatNote(rootNote, txt);
  txt[2] = '\0';
  drawText(48, 16, txt);
  drawSelected(64, 16, 2);
  drawTextPgm(72, 16, scale.getLabel());

  formatNote(noteA, txt);
  drawTextPgm(8, 32, titleA);
  drawText(56, 32, txt);

  formatNote(noteB, txt);
  drawTextPgm(8, 40, titleB);
  drawText(56, 40, txt);
}

void adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      rootNote = applyDelta<uint8_t>(rootNote, d, 0, 11);
      scale = Scale(rootNote, scaleIdx);
      break;
    case 2:
      scaleIdx = applyDelta<uint8_t>(scaleIdx, d, 0, NUM_SCALES - 1);
      scale = Scale(rootNote, scaleIdx);
      break;
    default:
      break;
  }
}

void start() {
}

void stop() {}

uint8_t toMidiNote(int16_t in) {
  if (in < 0) { in = 0; }
  uint8_t octave = in / 1000;
  uint8_t note = (((in % 1000) * 10) + 417) / 833;
  return octave * 12 + note;
}

void fillBuffer(OutputFrame *buf) {
  //noteA = toMidiNote(IO::getCV1In());
  noteA = scale.quantize(IO::getCV1In());
  uint16_t valueA = IO::calcCV1Out(noteToVoltage(noteA));

  //noteB = toMidiNote(IO::getCV2In());
  noteB = scale.quantize(IO::getCV2In());
  uint16_t valueB = IO::calcCV2Out(noteToVoltage(noteB));

  uint16_t gate1 = IO::calcGate1Out(0);
  uint16_t gate2 = IO::calcGate2Out(0);

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = valueA;
    buf->cv2 = valueB;
    buf->gate1 = gate1;
    buf->gate2 = gate2;
    buf++;
  }
}

constexpr Module module = {
  title,
  2,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer
};

} // FuncGen
