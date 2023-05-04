#include "LFO.hpp"

namespace LFO {

 const char title[] PROGMEM = "LFO      ";

constexpr uint8_t N_WAVES = 5;
const char sine_t[] PROGMEM = "Sin";
const char triangle_t[] PROGMEM = "Tri";
const char saw_up_t[] PROGMEM = "Saw";
const char saw_down_t[] PROGMEM = "iSw";
const char square_t[] PROGMEM = "Sqr";

constexpr uint8_t N_RANGE = 4;
const char neg4_t[] PROGMEM = "-4 0V";
const char bip4_t[] PROGMEM = "+/-4V";
const char pos4_t[] PROGMEM = "0 +4V";
const char pos8_t[] PROGMEM = "0 +8V";

const char bpm_t[] PROGMEM = "BPM:  ";
const char wave1_t[] PROGMEM = "A:";
const char wave2_t[] PROGMEM = "B:";
const char wave3_t[] PROGMEM = "1:";
const char wave4_t[] PROGMEM = "2:";
const char page1[] PROGMEM = "Wave Spd. Phase Range";
const char page2[] PROGMEM = "Swing SwPer Prob Slop";

LFO lfo;
const char * waveTitles[] = {sine_t, triangle_t, saw_up_t, saw_down_t, square_t};
const char *rangeTitles[] = {neg4_t, bip4_t, pos4_t, pos8_t};

void formatFactor(char *str, uint8_t factor) {
  str[3] = 0;
  if (factorDen[factor] == 1) {
    str[0] = '0' + factorNom[factor];
    str[1] = 'x';
    str[2] = ' ';
  } else {
    str[0] = '0' + factorNom[factor];
    str[1] = '/';
    str[2] = '0' + factorDen[factor];
  }
}

uint16_t Shape::save(uint16_t addr) {
  addr = Storage::write(addr, wave);
    addr = Storage::write(addr, factorNom[factor]);
    addr = Storage::write(addr, factorDen[factor]);
    addr = Storage::write(addr, phase);
    addr = Storage::write(addr, range);

    return addr;
}

uint16_t Shape::load(uint16_t addr) {
  addr = Storage::read(addr, wave);

  uint8_t nom, den;
  addr = Storage::read(addr, nom);
  addr = Storage::read(addr, den);

  for (uint8_t i = 0; i <= MAX_FACTOR; i++) {
    if (nom == factorNom[i] && den == factorDen[i]) {
      factor = i;
      break;
    }
  }

  addr = Storage::read(addr, phase);
  addr = Storage::read(addr, range);
  return addr;
}

void Shape::recalc(uint32_t mainIncrement) {
  increment = mainIncrement / factorNom[factor] * factorDen[factor];
}

void Shape::reset() {
  period = 0;
}

void Shape::nextPeriod() {
  period++;
  if (period >= swingPeriods) {
    period -= swingPeriods;
  }
}

void Shape::resetPhase() {
  constexpr auto OK_TO_RESET_MIN = uint32_t(2) << sinePosScaleBits;
  constexpr auto OK_TO_RESET_MAX = uint32_t(254) << sinePosScaleBits;

  if (factor >= FACTOR_ONE) {
    tablePos = 0;
    nextPeriod();
  } else if (tablePos < OK_TO_RESET_MIN || tablePos > OK_TO_RESET_MAX) {
    tablePos = 0;
    nextPeriod();
  }
}

void Shape::performStep() {
  tablePos += increment;
  if (tablePos > sinePosMod) {
    tablePos -= sinePosMod;
  }
}

int16_t Shape::applyRange(int16_t value) {
  switch(range) {
    case Negative4: return (value / 2) - 2000;
    case Bipolar4: return value;
    case Positive4: return (value / 2) + 2000;
    case Positive8: return value + 4000;
  }
  return 0;
}

int16_t Shape::getRawTableValue() {
  uint32_t pos;
  if (swing > 0) {
    if (period == 0) {
      // We should be swing% slower
      pos = (tablePos * (100 - swing)) / 100;
    } else if (period == 1) {
      uint8_t period0End = uint16_t(255) * (swing) / 100;
      if ((tablePos >> sinePosScaleBits) <= period0End) {
        uint8_t zero = uint16_t(256) * (100 - swing) / 100;
        uint8_t end = 255;
        pos = (uint32_t(zero) << sinePosScaleBits) + (tablePos * (end - zero) / period0End);
      } else {
        pos = (tablePos - (uint32_t(period0End) << sinePosScaleBits)) * 256 / (uint16_t(256) - period0End);
      }
    } else {
      pos = tablePos;
    }
  } else {
    pos = tablePos;
  }

  switch (wave) {
    case Sine: return Waves::Sine::get(pos, phase);
    case Triangle: return Waves::Triangle::get(pos, phase);
    case SawUp: return Waves::SawUp::get(pos, phase);
    case SawDown: return Waves::SawDown::get(pos, phase);
    case Square: return Waves::Square::get(pos, phase);
  }
  return 0;
}

int16_t Shape::getTableValue() { return applyRange(getRawTableValue()); }

void Shape::draw(uint8_t y, uint8_t controlIdx) {
  uint8_t selectedControlIdx = (currentControlIdx - 2) % CONTROLS_PER_SHAPE;

  if (selectedControlIdx > 3 && selectedControlIdx < CONTROLS_PER_SHAPE) {
    drawSelected(12, y, controlIdx + 4);
    drawDecimal(18, y, swing, '%', 4);

    drawSelected(42, y, controlIdx + 5);
    drawText(48, y, "x");
    drawDecimal(54, y, swingPeriods, 1);

    drawSelected(66, y, controlIdx + 6);
    drawText(72, y, "100%");

    drawSelected(96, y, controlIdx + 7);
    drawText(102, y, "100%");
  } else {
    drawSelected(12, y, controlIdx);
    drawTextPgm(18, y, waveTitles[wave]);

    drawSelected(36, y, controlIdx + 1);
    char str[6];
    formatFactor(str, factor);
    drawText(42, y, str);

    drawSelected(60, y, controlIdx + 2);
    uint16_t ph = (uint32_t(phase) * 360) >> 8;
    drawDecimal(66, y, ph, '\'', 4);

    drawSelected(90, y, controlIdx + 3);
    drawTextPgm(96, y, rangeTitles[range]);
  }

}

void LFO::recalc() {
  // Multiply by OUTBUFSIZE, since we increment only once per buffer
  increment = (((uint32_t(bpm) * TABLE_SIZE) << sinePosScaleBits)) / 60 * OUTBUFSIZE / SAMPLERATE;
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].recalc(increment);
  }
}

void LFO::draw() {
  uint8_t selectedControlIdx = (currentControlIdx - 2) % CONTROLS_PER_SHAPE;

  if (currentControlIdx < 2) {
    drawSelected(0, 16, 1);
    drawTextPgm(7, 16, bpm_t);
    drawDecimal(40, 16, bpm, 14);
  } else if (selectedControlIdx <= 3) {
    drawTextPgm(0, 16, page1);
  } else {
    drawTextPgm(0, 16, page2);
  }

  drawTextPgm(0, 24, wave1_t);
  drawTextPgm(0, 32, wave2_t);
  drawTextPgm(0, 40, wave3_t);
  drawTextPgm(0, 48, wave4_t);

  for (uint8_t s = 0; s < N_SHAPES; s++) {
    uint8_t controlIdx = s * CONTROLS_PER_SHAPE + 2;
    uint8_t y = s * 8 + 24;

    shapes[s].draw(y, controlIdx);
  }
}

void LFO::adjust(int8_t d) {
  switch(currentControlIdx) {
    case 1:
      bpm = applyDelta<uint16_t>(bpm, d, 1, 1000);
      recalc();
      break;
    default:
      uint8_t shapeIdx = (currentControlIdx - 2) / CONTROLS_PER_SHAPE;
      uint8_t controlIdx = (currentControlIdx - 2) % CONTROLS_PER_SHAPE;

      switch(controlIdx) {
        case 0:
          shapes[shapeIdx].wave = Wave(applyDelta<uint8_t>(shapes[shapeIdx].wave, d, 0, N_WAVES - 1));
          break;
        case 1:
          shapes[shapeIdx].factor = applyDelta<uint32_t>(shapes[shapeIdx].factor, d, 0, MAX_FACTOR);
          shapes[shapeIdx].recalc(increment);
          break;
        case 2:
          shapes[shapeIdx].phase = applyDelta<uint8_t>(shapes[shapeIdx].phase, d, 0, 255);
          break;
        case 3:
          shapes[shapeIdx].range = Range(applyDelta<uint8_t>(shapes[shapeIdx].range, d, 0, N_RANGE - 1));
          break;
        case 4:
          shapes[shapeIdx].swing = applyDelta<int8_t>(shapes[shapeIdx].swing, d, /*-25*/0, 25);
          shapes[shapeIdx].recalc(increment);
          break;
        case 5:
          shapes[shapeIdx].swingPeriods = applyDelta<int8_t>(shapes[shapeIdx].swingPeriods, d, 2, 8);
          shapes[shapeIdx].recalc(increment);
          break;
      }
  }
}

void  LFO::resetPhase() {
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].resetPhase();
  }
}

void LFO::checkGate() {
  bool gate = IO::getGate1In();
  if (gate != lastGate) {
    lastGate = gate;
    if (gate) {
      auto time = millis(); // millis is significantly faster than micros(), which has a long division.
      if (lastGateTime != NO_TIME) {
        // Reset the phase on incoming pulse
        mainPos = 0;
        resetPhase();

        auto delayMs = time - lastGateTime;
        uint16_t newbpm = uint32_t(1000) * 60 / delayMs;
        if (newbpm != bpm && newbpm > 30) {
          bpm = newbpm;
          recalc();
        }
      }
      lastGateTime = time;
    }
  }
}

void LFO::reset() {
  for (uint8_t i = 0; i < N_SHAPES; i++) {
    shapes[i].reset();
  }
}

void LFO::start() {
  recalc();
  reset();
  lastGate = IO::getGate1In();
  lastGateTime = NO_TIME;
}

void LFO::fillBuffer(OutputFrame *buf) {
  checkGate();

  // only use one value for the entire buffer. 750Hz is perfectly acceptable for an LFO.
  mainPos += increment;
  if (mainPos > sinePosMod) {
    mainPos -= sinePosMod;
    resetPhase();
  } else {
    for (uint8_t i = 0; i < N_SHAPES; i++) {
      shapes[i].performStep();
    }
  }
  const uint16_t value1 = IO::calcCV1Out(shapes[0].getTableValue());
  const uint16_t value2 = IO::calcCV2Out(shapes[1].getTableValue());
  const uint16_t value3 = IO::calcGate1Out(shapes[2].getTableValue());
  const uint16_t value4 = IO::calcGate2Out(shapes[3].getTableValue());

  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = value1;
    buf->cv2 = value2;
    buf->gate1 = value3;
    buf->gate2 = value4;
    buf++;
  }
}

void LFO::save(uint16_t addr) {
  uint8_t version = 1;
  Storage::write(addr, version);
  addr++;
  Storage::write(addr, bpm);
  addr += 2;

  for (uint8_t i = 0; i < N_SHAPES; i++) {
    addr = shapes[i].save(addr);
  }
}

void LFO::load(uint16_t addr) {
  uint8_t version;
  Storage::read(addr, version);
  addr++;
  if (version != 1) return;

  Storage::read(addr, bpm);
  addr += 2;

  for (uint8_t i = 0; i < N_SHAPES; i++) {
    addr = shapes[i].load(addr);
  }
}

}
