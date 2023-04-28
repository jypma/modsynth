#include "ADSR.hpp"
#include "Debug.hpp"
#include "Storage.hpp"

namespace ADSR {

Instance adsr1, adsr2, adsr3;

void Segment::reset(uint8_t idx) {
  controlIdx = idx;
  debugSerial(idx);
  setLength(length);
}

static constexpr uint32_t posPerSecond = ((uint32_t(256) * 1000) << Waves::posScaleBits) / (SAMPLERATE / OUTBUFSIZE);
void Segment::setLength(uint16_t ms) {
  length = ms;
  posInc = posPerSecond / ms;
}

void Segment::adjustLength(int8_t d) {
  int16_t delta = d;
  if (length >= 1000) {
    length = (length / 100) * 100;
    delta = delta * 100;
  } else if (length >= 100) {
    length = (length / 10) * 10;
    delta = delta * 10;
  }
  setLength(applyDelta16<uint16_t>(length, delta, 1, 60000));
}

void Segment::adjustEnv(int8_t d) {
  env = Envelope(applyDelta<uint8_t>(env, d, 0, N_ENVELOPES - 1));
}

void Segment::begin(uint16_t _start, uint16_t _end) {
  pos = 0;
  start = _start;
  end = _end;
}

uint16_t Segment::advance() {
  const int16_t scale = (end - start);
  uint16_t value = start + ((int32_t(getEnvelope(env, pos)) * scale) >> 16);
  pos += posInc;
  if (isEnded()) {
    return end;
  } else {
    return value;
  }
}

bool Segment::isEnded() {
  return pos >= Waves::maxPos;
}

void Segment::draw(uint8_t xs, uint8_t ys) {
  drawSelected(xs, ys, controlIdx);
  drawDecimal(xs + 6, ys, length, 4);
  drawSelected(xs + 30, ys, controlIdx + 1);
  drawTextPgm(xs + 36, ys, getEnvelopeTitle(env));
}

uint16_t Segment::save(uint16_t addr) {
  Storage::write(addr, length);
  addr += 2;
  Storage::write(addr, env);
  addr++;
  return addr;
}

uint16_t Segment::load(uint16_t addr) {
  Storage::read(addr, length);
  setLength(length);
  addr += 2;
  Storage::read(addr, env);
  addr++;
  return addr;
}

uint16_t Instance::nextLevel() {
  {
    Q15n16 next = level;
    switch (phase) {
      case Idle:
        break;
      case Attack: {
        next = attack.advance();
        if (attack.isEnded()) {
          phase = Decay;
          decay.begin(top, sustain);
        }
        break;
      }
      case Decay: {
        next = decay.advance();
        if (decay.isEnded()) {
          phase = Sustain;
        }
        break;
      }
      case Sustain:
        break; // Wait for IO to go low, which we're checking before.
      case Release: {
        next = release.advance();
        if (release.isEnded()) {
          phase = Idle;
        }
      }
    }
    level = next;
    return next;
  }
}

void Instance::setSustain(uint8_t s) {
  sustainLevel = s;
  sustain = zero + uint32_t(top - zero) * sustainLevel / 100;
}

void Instance::draw() {
  attack.draw(0, ys);
  decay.draw(20, ys + 8);

  drawSelected(64, ys, (index * CONTROLS_PER_INSTANCE) + 5);
  drawDecimal(64 + 6, ys, sustainLevel, 3);
  drawText(64 + 18, ys, "%");

  release.draw(78, ys + 8);
}

void Instance::adjust(int8_t d) {
  uint8_t control = (currentControlIdx - 1) % CONTROLS_PER_INSTANCE;
  switch(control) {
    case 0: attack.adjustLength(d); break;
    case 1: attack.adjustEnv(d); break;
    case 2: decay.adjustLength(d); break;
    case 3: decay.adjustEnv(d); break;
    case 4: setSustain(applyDelta<uint8_t>(sustainLevel, d, 0, 100)); break;
    case 5: release.adjustLength(d); break;
    case 6: release.adjustEnv(d); break;
    default: {}
  }
}

void Instance::reset(uint8_t _idx, uint16_t z, uint16_t t) {
  index = _idx;
  ys = 16 + 16 * index;
  phase = Idle;
  zero = z;
  top = t;
  setSustain(sustainLevel);
  level = zero;
  attack.reset(CONTROLS_PER_INSTANCE * index + 1);
  decay.reset(CONTROLS_PER_INSTANCE * index + 3);
  release.reset(CONTROLS_PER_INSTANCE * index + 6);
}

void Instance::handleGate(bool gate) {
  switch (phase) {
    case Idle:
      if (gate) {
        phase = Attack;
        level = zero;
        attack.begin(zero, top);
      }
      break;
    case Attack:
    case Decay:
    case Sustain:
      if (!gate) {
        phase = Release;
        release.begin(level, zero);
      }
      break;
    case Release:
      if (gate) {
        phase = Attack;
        // Retrigger from current release level, if any
        attack.begin(level, sustain);
      }
      break;
  }
}

uint16_t Instance::save(uint16_t addr) {
  Storage::write(addr, sustainLevel);
  addr++;
  addr = attack.save(addr);
  addr = decay.save(addr);
  addr = release.save(addr);
  return addr;
}

uint16_t Instance::load(uint16_t addr) {
  Storage::read(addr, sustainLevel);
  setSustain(sustainLevel);
  addr++;
  addr = attack.load(addr);
  addr = decay.load(addr);
  addr = release.load(addr);
  return addr;
}

uint16_t getEnvelope(Envelope env, uint32_t pos) {
  switch(env) {
    case Root3: return Waves::EnvelopeRoot3::get(pos);
    case Sin: return Waves::EnvelopeSin::get(pos);
    case Power2: return Waves::EnvelopePower2::get(pos);
    case Power3: return Waves::EnvelopePower3::get(pos);
    case Lin: return pos >> (Waves::posScaleBits - 8);
    default: return 0;
  }
}
const char *getEnvelopeTitle(Envelope env) {
  switch(env) {
    case Root3: return envRoot3_t;
    case Sin: return envSin_t;
    case Lin: return envLin_t;
    case Power2: return envPower2_t;
    default: return envPower3_t;
  }
}

void start() {
  adsr1.reset(0, IO::calcCV1Out(0), IO::calcCV1Out(8000));
  adsr2.reset(1, IO::calcCV2Out(0), IO::calcCV2Out(8000));
  adsr3.reset(2, IO::calcGate1Out(0), IO::calcGate1Out(8000));
}

void stop() {

}

void draw() {
  adsr1.draw();
  adsr2.draw();
  adsr3.draw();
}

void adjust(int8_t d) {
  uint8_t index = (currentControlIdx - 1) / CONTROLS_PER_INSTANCE;
  switch(index) {
    case 0: adsr1.adjust(d); break;
    case 1: adsr2.adjust(d); break;
    case 2: adsr3.adjust(d); break;
    default: {}
  }
}

void fillBuffer(OutputFrame *buf) {
  auto gate2 = IO::calcGate2Out(0); // unused, always 0V

  // TODO disable interpolation and calculate each buffer value for "fast" curves.
  adsr1.handleGate(IO::getGate1In());
  auto cv1 = adsr1.nextLevel();
  adsr2.handleGate(IO::getGate2In());
  auto cv2 = adsr2.nextLevel();
  adsr3.handleGate(IO::getGate3In());
  auto gate1 = adsr3.nextLevel();
  for (uint8_t i = 0; i < OUTBUFSIZE; i++) {
    buf->cv1 = cv1;
    buf->cv2 = cv2;
    buf->gate1 = gate1;
    buf->gate2 = gate2;
    buf++;
  }
}

void save(uint16_t addr) {
  uint8_t version = 1;
  Storage::write(addr, version);
  addr++;

  addr = adsr1.save(addr);
  addr = adsr2.save(addr);
  addr = adsr3.save(addr);
}

void load(uint16_t addr) {
  uint8_t version;
  Storage::read(addr, version);
  addr++;
  if (version != 1) return;

  addr = adsr1.load(addr);
  addr = adsr2.load(addr);
  addr = adsr3.load(addr);
}

}
