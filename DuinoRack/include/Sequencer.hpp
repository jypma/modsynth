#pragma once

#include <Arduino.h>
#include <cstdlib>
#include "IO.h"
#include "OutputBuf.h"
#include "Random.hpp"

// This is a CV sequencer. Let's do a drums-only sequencer separately (which could use multiple patterns since it uses much less memory).

namespace Sequencer {

struct AdvStep {
  // If note, value is the MIDI note.
  // If CV, value is mapped between CV min and CV max for that channel (linearly).
  uint8_t value;
  // Probability of the note firing (and a gate being there). 255 is always on, 0 is always off.
  uint8_t prob;
  uint8_t flags; // 0-3: length (1..16 steps); 4: glide;

  int16_t scaleValue(int16_t min, int16_t max, bool noteMode) {
    if (noteMode) {
      // 1 Volt per octave, so 1/12 Volt per note
      return uint32_t(value) * 1000 / 12;
    } else {
      return min + ((int32_t(value) * (max - min)) >> 8);
    }
  }
};

// 256 bytes per preset -> 3 presets

struct Short {
  // Two variants: A,1,2 vs B.3, or everything linked.
  AdvStep channels[3]; // A, 1, and 2 are linked (Note/CV, Note/CV, Gate). B, 3 are linked (Note/CV, Gate)
};

struct Long {
  AdvStep channel; // A is note or CV, 1 is gate
};

struct Medium {
  // Two variants: Separate, Linked (only one gate)
  AdvStep channels[2]; // A and B is note or CV, 1 and 2 is gate
};

constexpr uint8_t MAX_MEM = 224;

// Possible layouts (1 sequencer):
// CV                             value, prob, flags
// Note, Gate                     value, prob, flags
// Note, CV, Gate                 value, value, prob, flags
// Note, Note, Gate               value, value, prob, flags
// Note, CV, CV, Gate             value, value, value, prob, flags
// Note, Note, CV, Gate           value, value, value, prob, flags
// Note, CV, CV, CV, Gate         value, value, value, value, prob, flags
// Note, Note, CV, CV, Gate       value, value, value, value, prob, flags
// Inputs:
// - Start/Stop/Forward
// - Reset

// Possible layouts (2 sequencers):
// CV |  CV                       value, prob, flags | value, prob, flags
// Note, Gate | Note, Gate        value, prob, flags | value, prob, flags
// Note, Gate | CV                value, prob, flags | value, prob, flags
// Note, CV, Gate | Note, Gate    value, value, prob, flags | value, prob, flags
// Inputs:
// - Start/Stop/Forward
// - Reset

// We do only Note, CV Gate or Note, Note Gate, or CV, CV, for now.

template <uint8_t Size>
struct Format {
  static constexpr uint8_t MAX_STEPS = MAX_MEM / (Size + 2);

  uint8_t flags;
  uint8_t prob;
  uint8_t values[Size];
};

union Steps {
  Format<1> format1[Format<1>::MAX_STEPS];
  Format<2> format2[Format<2>::MAX_STEPS];
};

bool probToGate(uint8_t prob) {
  return (prob == 0) ? false : (prob == 255) ? true : (Random::nextByte() <= prob);
}

template <uint16_t calcOut(int16_t), uint16_t OutputFrame::*field>
struct Channel {
  int16_t min = 0, max = 4000;
  bool noteMode = false;

  void apply(uint8_t value, OutputBuf::Buffer &buf) {
    int16_t mV;
    if (noteMode) {
      // 1 Volt per octave, so 1/12 Volt per note
      mV = uint32_t(value) * 1000 / 12;
    } else {
      mV = min + ((int32_t(value) * (max - min)) >> 8);
    }
    auto val = calcOut(mV);
    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      buf.analog[i].*field = val;
    }
  }

  void applyGate(uint8_t prob, OutputBuf::Buffer &buf) {
    auto val = probToGate(prob) ? max : min;
    for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
      buf.analog[i].*field = val;
    }
  }
};

using ChannelA = Channel<&IO::calcCV1Out, &OutputFrame::cv1>;
using ChannelB = Channel<&IO::calcCV2Out, &OutputFrame::cv2>;
using Channel1 = Channel<&IO::calcGate1Out, &OutputFrame::gate1>;
using Channel2 = Channel<&IO::calcGate2Out, &OutputFrame::gate2>;
struct Channel3 {
  static void apply(uint8_t value, OutputBuf::Buffer &buf) {
    buf.setGate3All(value > 0);
  }
  static void applyGate(uint8_t prob, OutputBuf::Buffer &buf) {
    buf.setGate3All(probToGate(prob));
  }
};

enum FormatType: uint8_t { NoteGate, NoteNoteGate };

class Sequencer {
  Steps steps;
  FormatType format = NoteNoteGate;
  // FIXME: Different pos per sequencer
  uint8_t pos;

  ChannelA channelA;
  ChannelB channelB;
  Channel1 channel1;
  Channel2 channel2;
public:
  // TODO: Handle length
  // TODO: Handle glide

  void fillBuffer(OutputBuf::Buffer &buf) {
    auto rnd = rand();
    switch(format) {
      case NoteGate:
        channelA.apply(steps.format1[pos].values[0], buf);
        channel1.applyGate(steps.format1[pos].prob, buf);
        break;
      case NoteNoteGate:
        channelA.apply(steps.format2[pos].values[0], buf);
        channelB.apply(steps.format2[pos].values[1], buf);
        channel1.applyGate(steps.format2[pos].prob, buf);
    }
  }
};

// Master settings:
// 32 bytes total.
// 240 bytes for steps.
// - Glide times for CV and note, per channel (in %) 1x4: 4
// - Mode (1, SHORT, MEDIUM. LONG)
// - CV Channel modes (CV or Note), 1 bit per channel: 1
// - Internal BPM (2)
// - Min and max CV per channel (4*4=16)
// - Length per channel (4 bytes?)

// SHORT (4 channels + gate):
// - 12 bytes per step
// - 18 steps

// LONG (1 channel + gate):
// - 3 bytes per step
// - 74 steps

// MEDIUM (2 channels + gate)
// - 4 bytes per step
// - 56 steps


}
