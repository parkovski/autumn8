#ifndef A8_MIDI_MESSAGE_H_
#define A8_MIDI_MESSAGE_H_

#include "a8/a8.h"

#include <cstdint>
#include <cstddef>
#include <span>
#include <iosfwd>

namespace a8::midi {

enum class Status : uint8_t {
  None                 = 0x00,
  NoteOff              = 0x80,
  NoteOn               = 0x90,
  PolyphonicAftertouch = 0xA0,
  ControlChange        = 0xB0,
  ProgramChange        = 0xC0,
  ChannelAftertouch    = 0xD0,
  PitchBend            = 0xE0,
  SysEx                = 0xF0,
  QuarterFrame         = 0xF1,
  SongPosition         = 0xF2,
  SongSelect           = 0xF3,
  TuneRequest          = 0xF6,
  EOX                  = 0xF7,
  TimingClock          = 0xF8,
  Start                = 0xFA,
  Continue             = 0xFB,
  Stop                 = 0xFC,
  ActiveSensing        = 0xFE,
  Reset                = 0xFF,
};

std::ostream &operator<<(std::ostream &os, Status status);

constexpr inline bool message_has_channel(Status status) noexcept {
  return status != Status::None && ((uint8_t)status & 0xF0) != 0xF0;
}

constexpr inline unsigned message_size(Status status) noexcept {
  switch (status) {
  default:                           return 0;
  case Status::NoteOff:              return 3;
  case Status::NoteOn:               return 3;
  case Status::PolyphonicAftertouch: return 3;
  case Status::ControlChange:        return 3;
  case Status::ProgramChange:        return 2; // or ProgramBank: 4
  case Status::ChannelAftertouch:    return 2;
  case Status::PitchBend:            return 3;
  case Status::SysEx:                return 0;
  case Status::QuarterFrame:         return 2;
  case Status::SongPosition:         return 3;
  case Status::SongSelect:           return 2;
  case Status::TuneRequest:          return 1;
  case Status::EOX:                  return 0;
  case Status::TimingClock:          return 1;
  case Status::Start:                return 1;
  case Status::Continue:             return 1;
  case Status::Stop:                 return 1;
  case Status::ActiveSensing:        return 1;
  case Status::Reset:                return 1;
  }
}

constexpr inline uint8_t msbu(uint16_t u14) noexcept {
  assert((u14 & 0xC000) == 0 && "Expected 14-bit number");
  return (uint8_t)(u14 >> 7);
}

constexpr inline uint8_t msbi(int16_t i14) noexcept {
  return msbu((uint16_t)(i14 + 0x2000));
}

constexpr inline uint8_t lsbu(uint16_t u14) noexcept {
  assert((u14 & 0xC000) == 0 && "Expected 14-bit number");
  return (uint8_t)(u14 & 0x7F);
}

constexpr inline uint8_t lsbi(int16_t i14) noexcept {
  return lsbu((uint16_t)(i14 + 0x2000));
}

constexpr inline uint16_t make_u14(uint8_t msb, uint8_t lsb) noexcept {
  assert((msb & 0xF0) == 0 && "Expected 7-bit msb");
  assert((lsb & 0xF0) == 0 && "Expected 7-bit lsb");
  return (uint16_t)((uint16_t)(msb << 7) | lsb);
}

constexpr inline int16_t make_i14(uint8_t msb, uint8_t lsb) noexcept {
  return (int16_t)make_u14(msb, lsb) - 0x2000;
}

struct Message {
private:
  explicit Message(Status status, uint8_t ch, uint8_t b0, uint8_t b1,
                   uint8_t b2) noexcept
    : _ptr{nullptr},
      _bytes{(uint8_t)status, b0, b1, b2}
  {
    if (message_has_channel(status)) {
      assert(ch >= 1 && ch <= 16);
      _bytes[0] |= (ch - 1);
    }
  }

  explicit Message(Status status, uint8_t ch, uint8_t b0, uint8_t b1) noexcept
    : Message(status, ch, b0, b1, 0)
  {}

  explicit Message(Status status, uint8_t ch, uint8_t b0) noexcept
    : Message(status, ch, b0, 0, 0)
  {}

  explicit Message(Status status) noexcept
    : Message(status, 0, 0, 0, 0)
  {}

public:
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

  static Message ChannelAftertouch(uint8_t channel, uint8_t value) noexcept
  { return Message(Status::ChannelAftertouch, channel, value); }

  static Message PitchBend(uint8_t channel, int16_t value) noexcept
  { return Message(Status::PitchBend, channel, msbi(value), lsbi(value)); }

  static Message QuarterFrame(uint8_t type, uint8_t value) noexcept
  { return Message(Status::QuarterFrame, 0, type, value); }

  static Message SongPosition(uint16_t value) noexcept
  { return Message(Status::SongPosition, 0, msbu(value), lsbu(value)); }

  static Message SongSelect(uint8_t song) noexcept
  { return Message(Status::SongSelect, 0, song); }

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
    uint8_t b = data()[0];
    // MIDI status codes are either 1111NNNN or 1NNNCCCC,
    // where N is part of the status byte and C is a channel.
    if ((b & 0xF0) == 0xF0) {
      return (Status)b;
    }
    return (Status)(b & 0xF0);
  }

  // Returns the message channel from 1 to 16, or 0 if the message has no
  // channel.
  uint8_t channel() const noexcept {
    if (!message_has_channel(status())) {
      return 0;
    }
    return (data()[0] & 0xF) + 1;
  }

  const uint8_t *data() const noexcept {
    return _ptr ? _ptr : _bytes;
  }

  const unsigned size() const noexcept {
    return _ptr ? _len : message_size(status());
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
