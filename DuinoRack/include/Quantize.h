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
const char notenames[] PROGMEM = "C-C#D-D#E-F-F#G-G#A-A#B-";
const char scale_t[] PROGMEM = "Scale: ";
const char titleA[] PROGMEM = "Note 1: ";
const char titleB[] PROGMEM = "Note 2: ";

uint8_t scaleIdx = 0;
auto scale = Scale(scaleIdx);

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
  drawTextPgm(7, 16, scale_t);
  drawText(0, 16, (currentControlIdx == 1) ? ">" : " ");
  drawTextPgm(56, 16, scale.getLabel());

  char txt[5];
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
      scaleIdx = applyDelta<uint8_t>(scaleIdx, d, 0, NUM_SCALES - 1);
      scale = Scale(scaleIdx);
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
  1,
  &draw,
  &start,
  &stop,
  &adjust,
  &fillBuffer
};

} // FuncGen
