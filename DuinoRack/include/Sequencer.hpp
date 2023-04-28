#pragma once

#include <Arduino.h>

// This is a CV sequencer. Let's do a drums-only sequencer separately.

namespace Sequencer {

struct AdvStep {
  // If note, value is the MIDI note.
  // If CV, value is mapped between CV min and CV max for that channel (linearly).
  uint8_t value;
  // Probability of the note firing (and the gate being there). 255 is always on, 0 is always off.
  uint8_t prob;
  uint8_t flags; // 0-3: length (1..16 steps); 4: glide;
};

// 256 bytes per preset -> 3 presets


// Master settings:
// 32 bytes total.
// 240 bytes for steps.
// - Glide times for CV and note, per channel (in %) 1x4: 4
// - Mode (1, SHORT, MEDIUM. LONG)
// - CV Channel modes (CV or Note), 1 bit per channel: 1
// - Internal BPM (2)
// - Min and max CV per channel (4*4=16)

// SHORT (4 channels + gate):
// - 12 bytes per step
// - 18 notes

// LONG (1 channel + gate):
// - 3 bytes per step
// - 74 steps

// MEDIUM (2 channels + gate)
// - 4 bytes per step
// - 56 steps


}
