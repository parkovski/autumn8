#ifndef A8_MIDI_MESSAGE_H_
#define A8_MIDI_MESSAGE_H_

#include "a8/a8.h"

#include <cstdint>
#include <cstddef>
#include <span>
#include <iosfwd>

namespace a8::midi {

#if 0
enum MessageKind {
  ChannelVoice             =      0x1,
    KeyValue               =      0x3,
      NoteOnOff            =      0x7,
        NoteOff            =      0xF,
        NoteOn             =     0x17,
      PolyphonicAftertouch =     0x23,
      ControlChange        =     0x43,
    ProgramChange          =     0x81,
    ChannelAftertouch      =    0x101,
    PitchBend              =    0x201,
  TimeCodeQuarterFrame     =    0x400,
  SongPositionPointer      =    0x800,
  SongSelect               =   0x1000,
  TuneRequest              =   0x2000,
  RealTime                 =   0x4000,
    TimingClock            =   0xC000,
    Start                  =  0x14000,
    Continue               =  0x24000,
    Stop                   =  0x44000,
    ActiveSensing          =  0x84000,
    Reset                  = 0x104000,
  SysEx                    = 0x200000,
};
#endif

enum class Status : uint8_t {
  None                 = 0x00,
  NoteOff              = 0x80,
  NoteOn               = 0x90,
  PolyphonicAftertouch = 0xA0,
  ControlChange        = 0xB0,
  ProgramChange        = 0xC0,
  ProgramBank          = 0x0F, // FIXME
  ChannelAftertouch    = 0xD0,
  PitchBend            = 0xE0,
  SysEx                = 0xF0,
  QuarterFrame         = 0xF1,
  SongPosition         = 0xF2,
  SongSelect           = 0xF3,
  TuneRequest          = 0xF4,
  TimingClock          = 0xF8,
  Start                = 0xFA,
  Continue             = 0xFB,
  Stop                 = 0xFC,
  ActiveSensing        = 0xFE,
  Reset                = 0xFF,
};

std::ostream &operator<<(std::ostream &os, Status status);

inline uint8_t msbu(uint16_t u14) noexcept {
  assert((u14 & 0xC000) == 0 && "Expected 14-bit number");
  return (uint8_t)(u14 >> 7);
}

inline uint8_t msbi(int16_t i14) noexcept {
  return msbu((uint16_t)(i14 + 0x2000));
}

inline uint8_t lsbu(uint16_t u14) noexcept {
  assert((u14 & 0xC000) == 0 && "Expected 14-bit number");
  return (uint8_t)(u14 & 0x7F);
}

inline uint8_t lsbi(int16_t i14) noexcept {
  return lsbu((uint16_t)(i14 + 0x2000));
}

struct Message {
  explicit Message(Status status, uint8_t b0, uint8_t b1, uint8_t b2) noexcept
    : _ptr{nullptr}, _bytes{(uint8_t)status, b0, b1, b2}
  {}

  explicit Message(Status status, uint8_t b0, uint8_t b1) noexcept
    : Message(status, b0, b1, 0)
  {}

  explicit Message(Status status, uint8_t b0) noexcept
    : Message(status, b0, 0, 0)
  {}

  explicit Message(Status status) noexcept
    : Message(status, 0, 0, 0)
  {}

  explicit Message(std::span<const uint8_t> data) noexcept
    : _ptr{data.data()}, _len{(uint32_t)data.size()}
  {}

  static Message None() noexcept
  { return Message(Status::None); }

  static Message NoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
    noexcept
  { return Message(Status::NoteOff, channel, note, velocity); }

  static Message NoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
    noexcept
  { return Message(Status::NoteOn, channel, note, velocity); }

  static Message PolyphonicAftertouch(uint8_t channel, uint8_t note,
                                      uint8_t value) noexcept
  { return Message(Status::PolyphonicAftertouch, channel, note, value); }

  static Message ControlChange(uint8_t channel, uint8_t control,
                               uint8_t value) noexcept
  { return Message(Status::ControlChange, channel, control, value); }

  static Message ProgramChange(uint8_t channel, uint8_t program) noexcept
  { return Message(Status::ProgramChange, channel, program); }

  static Message ProgramBank(uint8_t channel, uint8_t program, uint8_t msb,
                             uint8_t lsb) noexcept
  { return Message(Status::ProgramBank, channel, program); }

  static Message ChannelAftertouch(uint8_t channel, uint8_t value) noexcept
  { return Message(Status::ChannelAftertouch, channel, value); }

  static Message PitchBend(uint8_t channel, int16_t value) noexcept
  { return Message(Status::PitchBend, channel, msbi(value), lsbi(value)); }

  static Message QuarterFrame(uint8_t type, uint8_t value) noexcept
  { return Message(Status::QuarterFrame, type, value); }

  static Message SongPosition(uint16_t value) noexcept
  { return Message(Status::SongPosition, msbu(value), lsbu(value)); }

  static Message SongSelect(uint8_t song) noexcept
  { return Message(Status::SongSelect, song); }

  static Message TuneRequest() noexcept
  { return Message(Status::TuneRequest); }

  static Message TimingClock() noexcept
  { return Message(Status::TimingClock); }

  static Message Start() noexcept
  { return Message(Status::Start); }

  static Message Continue() noexcept
  { return Message(Status::Continue); }

  static Message Stop() noexcept
  { return Message(Status::Stop); }

  static Message ActiveSensing() noexcept
  { return Message(Status::ActiveSensing); }

  static Message Reset() noexcept
  { return Message(Status::Reset); }

  static Message SysEx(std::span<const uint8_t> data) noexcept
  { return Message(data); }

  Status status() const noexcept {
    return _ptr ? Status::SysEx : (Status)_bytes[0];
  }

  const uint8_t *data() const noexcept {
    return _ptr ? _ptr : _bytes;
  }

  const uint32_t size() const noexcept {
    return _ptr ? _len : 4;
  }

  std::span<const uint8_t> span() const noexcept {
    return {data(), size()};
  }

  operator std::span<const uint8_t>() const noexcept {
    return span();
  }

private:
  const uint8_t *_ptr;
  union {
    uint8_t _bytes[4];
    uint32_t _len;
  };
};

std::ostream &operator<<(std::ostream &os, const Message &message);

} // namespace a8::midi

#endif // A8_MIDI_MESSAGE_H_
