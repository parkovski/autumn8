#ifndef A8_MIDI_MESSAGE_H_
#define A8_MIDI_MESSAGE_H_

#include "a8/a8.h"
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <cassert>

A8_PRAGMA_MS(warning(push));
A8_PRAGMA_MS(warning(disable: 4250)); // Inheritance via dominance

namespace a8::midi {

namespace detail {
  // Combines two 7-bit unsigned numbers into one 14-bit unsigned.
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline uint16_t to_14bit_unsigned(uint8_t msb, uint8_t lsb) {
    return (uint16_t)msb << 7 | lsb;
  }

  // Combines two 7-bit unsigned numbers into one 14-bit signed. The resulting
  // value is shifted into the range [-0x2000, 0x1FFF].
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline int16_t to_14bit_signed(uint8_t msb, uint8_t lsb) {
    return ((int16_t)msb << 7 | (int16_t)lsb) - 0x2000;
  }

  // Returns the high order (most significant) byte in a 14-bit uint.
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline uint8_t from_14bit_msb(uint16_t u14) {
    assert((u14 & 0xC000) == 0 && "Only the lower 14 bits are valid.");
    return (uint8_t)(u14 >> 7);
  }

  // Returns the low order (least significant) byte in a 14-bit uint.
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline uint8_t from_14bit_lsb(uint16_t u14) {
    return (uint8_t)(u14 & 0x7F);
  }

  // Returns the high order (most significant) byte in a 14-bit signed int,
  // normalized from the range [-0x2000, 0x1FFF] to [0, 0x3FFF].
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline uint8_t from_14bit_msb(int16_t i14) {
    assert(i14 >= -0x2000 && i14 <= 0x1FFF && "Value out of range.");
    return from_14bit_msb((uint16_t)(i14 + 0x2000));
  }

  // Returns the low order (least significant) byte in a 14-bit signed int,
  // normalized from the range [-0x2000, 0x1FFF] to [0, 0x3FFF].
  // Careful: MIDI uses big-endian order (lsb, msb).
  static inline uint8_t from_14bit_lsb(int16_t i14) {
    return from_14bit_lsb((uint16_t)(i14 + 0x2000));
  }
}

struct IMessage {
  virtual const uint8_t *data() const noexcept = 0;
  virtual const size_t size() const noexcept = 0;

  uint8_t status() const {
    uint8_t b = data()[0];
    // Status bytes are either SSSSCCCC, where SSSS < 0xF, or 1111SSSS.
    if ((b & 0xF0) < 0xF0) {
      return b & 0xF0;
    }
    return b;
  }
};

// Basic MIDI message: Just an array of bytes with the status code in byte 0.
struct Message : virtual IMessage {
  explicit Message(const uint8_t *data, size_t size) noexcept
    : _data{data}, _size{static_cast<uint32_t>(size)}
  {
    assert(data != nullptr && size > 0 && "Empty messages are not allowed.");
    assert(size < UINT32_MAX && "Seriously?????");
  }

  const uint8_t *data() const noexcept override final { return _data; }
  const size_t size() const noexcept override final { return _size; }

private:
  const uint8_t *_data;
  uint32_t _size;
};

// Short message, up to 3 bytes of data.
struct NonSysExMessage : virtual IMessage {
private:
  constexpr static uint8_t checkedStatus(uint8_t status) {
    assert((status & 0x80) == 0x80 && "Status bytes expect the top bit set.");
    return status;
  }

  constexpr static uint8_t checked7bit(uint8_t data) {
    assert((data & 0x80) == 0 && "7-bit data expected");
    return data;
  }

public:
  explicit NonSysExMessage(uint8_t status) noexcept
    : _data{checkedStatus(status), 0, 0, 1}
  {}

  explicit NonSysExMessage(uint8_t status, uint8_t data1) noexcept
    : _data{checkedStatus(status), checked7bit(data1), 0, 2}
  {}

  explicit NonSysExMessage(uint8_t status, uint8_t data1, uint8_t data2)
    noexcept
    : _data{checkedStatus(status), checked7bit(data1), checked7bit(data2), 3}
  {}

  const uint8_t *data() const noexcept override final { return _data; }
  const size_t size() const noexcept override final { return _data[3]; }

private:
  uint8_t _data[4];
};

// Channel voice messages split the status code between a message type and
// a channel number.
struct IChannelVoiceMessage : virtual IMessage {
  static constexpr size_t Size = 1;

  uint8_t status() const {
    return data()[0] & 0xF0;
  }

  uint8_t channel() const {
    return data()[0] & 0xF;
  }
};

struct ChannelVoiceMessage : virtual IChannelVoiceMessage, NonSysExMessage {
private:
  static uint8_t checkedStatusByte(uint8_t status, uint8_t channel) {
    assert((status & 0xF) == 0 && "Low nibble must be clear in status");
    assert((channel & 0xF0) == 0 && "High nibble must be clear in channel");
    return status | channel;
  }

public:
  explicit ChannelVoiceMessage(uint8_t status, uint8_t channel) noexcept
    : NonSysExMessage(checkedStatusByte(status, channel))
  {}

  explicit ChannelVoiceMessage(uint8_t status, uint8_t channel, uint8_t data1)
    noexcept
    : NonSysExMessage(checkedStatusByte(status, channel), data1)
  {}

  explicit ChannelVoiceMessage(uint8_t status, uint8_t channel, uint8_t data1,
                               uint8_t data2) noexcept
    : NonSysExMessage(checkedStatusByte(status, channel), data1, data2)
  {}
};

struct IKeyValueMessage : virtual IChannelVoiceMessage {
  static constexpr size_t Size = 3;

  uint8_t key() const {
    return data()[1];
  }

  uint8_t value() const {
    return data()[2];
  }
};

struct KeyValueMessage : virtual IKeyValueMessage, ChannelVoiceMessage {
  explicit KeyValueMessage(uint8_t status, uint8_t channel, uint8_t key,
                           uint8_t value) noexcept
    : ChannelVoiceMessage(status, channel, key, value)
  {}
};

struct INoteOnOffMessage : virtual IKeyValueMessage {
  using IKeyValueMessage::Size;
  static constexpr uint8_t StatusOff = 0x80;
  static constexpr uint8_t StatusOn = 0x90;

  bool on() const {
    return (data()[0] & 0xF0) == 0x90;
  }
};

struct INoteOffMessage : virtual INoteOnOffMessage {
  using INoteOnOffMessage::Size;
  static constexpr uint8_t Status = INoteOnOffMessage::StatusOff;

  bool on() const { return false; }
};

struct INoteOnMessage : virtual INoteOnOffMessage {
  using INoteOnOffMessage::Size;
  static constexpr uint8_t Status = INoteOnOffMessage::StatusOn;

  bool on() const { return true; }
};

struct NoteOnOffMessage : virtual INoteOnOffMessage, KeyValueMessage {
  using INoteOnOffMessage::StatusOff;
  using INoteOnOffMessage::StatusOn;

  explicit NoteOnOffMessage(uint8_t channel, uint8_t key, uint8_t velocity,
                            bool on) noexcept
    : KeyValueMessage((on ? StatusOn : StatusOff), channel, key, velocity)
  {}

  uint8_t velocity() const {
    return value();
  }
};

struct NoteOffMessage : virtual INoteOffMessage, NoteOnOffMessage {
  static constexpr uint8_t Status = NoteOnOffMessage::StatusOff;

  explicit NoteOffMessage(uint8_t channel, uint8_t key, uint8_t velocity)
    noexcept
    : NoteOnOffMessage(channel, key, velocity, false)
  {}
};

struct NoteOnMessage : virtual INoteOnMessage, NoteOnOffMessage {
  static constexpr uint8_t Status = NoteOnOffMessage::StatusOn;

  explicit NoteOnMessage(uint8_t channel, uint8_t key, uint8_t velocity)
    noexcept
    : NoteOnOffMessage(channel, key, velocity, true)
  {}
};

struct IPolyphonicAftertouchMessage : virtual IKeyValueMessage {
  using IKeyValueMessage::Size;
  static constexpr uint8_t Status = 0xA0;
};

struct PolyphonicAftertouchMessage : virtual IPolyphonicAftertouchMessage,
                                     KeyValueMessage {
  using IPolyphonicAftertouchMessage::Status;

  explicit PolyphonicAftertouchMessage(uint8_t channel, uint8_t key,
                                       uint8_t value) noexcept
    : KeyValueMessage(Status, channel, key, value)
  {}
};

struct IControlChangeMessage : virtual IKeyValueMessage {
  using IKeyValueMessage::Size;
  static constexpr uint8_t Status = 0xB0;

  uint8_t control() const {
    return key();
  }
};

struct ControlChangeMessage : virtual IControlChangeMessage, KeyValueMessage {
  using IControlChangeMessage::Status;

  explicit ControlChangeMessage(uint8_t channel, uint8_t control,
                                uint8_t value) noexcept
    : KeyValueMessage(Status, channel, control, value)
  {}
};

struct IProgramChangeMessage : virtual IChannelVoiceMessage {
  static constexpr size_t Size = 2;
  static constexpr uint8_t Status = 0xC0;

  uint8_t program() const {
    return data()[1];
  }
};

struct ProgramChangeMessage : virtual IProgramChangeMessage,
                              ChannelVoiceMessage {
  using IProgramChangeMessage::Status;

  explicit ProgramChangeMessage(uint8_t channel, uint8_t program) noexcept
    : ChannelVoiceMessage(Status, channel, program)
  {}
};

struct IChannelAftertouchMessage : virtual IChannelVoiceMessage {
  static constexpr size_t Size = 2;
  static constexpr uint8_t Status = 0xD0;

  uint8_t value() const {
    return data()[1];
  }
};

struct ChannelAftertouchMessage : virtual IChannelAftertouchMessage,
                                  ChannelVoiceMessage {
  using IChannelAftertouchMessage::Status;

  ChannelAftertouchMessage(uint8_t channel, uint8_t value) noexcept
    : ChannelVoiceMessage(Status, channel, value)
  {}
};

struct IPitchBendMessage : virtual IChannelVoiceMessage {
  static constexpr size_t Size = 3;
  static constexpr uint8_t Status = 0xE0;

  uint8_t lsb() const {
    return data()[1];
  }

  uint8_t msb() const {
    return data()[2];
  }

  // Returns a 14-bit signed integer where the resting position is 0.
  int16_t value() const {
    return detail::to_14bit_signed(msb(), lsb());
  }
};

struct PitchBendMessage : virtual IPitchBendMessage, ChannelVoiceMessage {
  using IPitchBendMessage::Status;

  explicit PitchBendMessage(uint8_t channel, uint8_t msb, uint8_t lsb) noexcept
    : ChannelVoiceMessage(Status, channel, lsb, msb)
  {}

  explicit PitchBendMessage(uint8_t channel, int16_t int14Value) noexcept
    : ChannelVoiceMessage(Status, channel,
                          detail::from_14bit_lsb(int14Value),
                          detail::from_14bit_msb(int14Value))
  {}
};

struct ISysExMessage : virtual IMessage {
  static constexpr uint8_t Status = 0xF0;
  static constexpr uint8_t Start = 0xF0;
  static constexpr uint8_t End = 0xF7;

  uint32_t mfgId() const {
    return 0; // TODO
  }
};

struct ITimeCodeQuarterFrameMessage : virtual IMessage {
  static constexpr uint8_t Status = 0xF1;
  static constexpr size_t Size = 2;

  uint8_t type() const {
    return data()[1] >> 4;
  }

  uint8_t value() const {
    return data()[1] & 0xF;
  }
};

struct TimeCodeQuarterFrameMessage : virtual ITimeCodeQuarterFrameMessage,
                                     NonSysExMessage {
private:
  static uint8_t checked(uint8_t type, uint8_t value) {
    assert((type & 0xF8) == 0 && "Valid range for type is 0-7");
    assert((value & 0xF0) == 0 && "Valid range for value is 0-15");
    return (type << 4) | value;
  }

public:
  using ITimeCodeQuarterFrameMessage::Status;

  explicit TimeCodeQuarterFrameMessage(uint8_t type, uint8_t value) noexcept
    : NonSysExMessage(Status, checked(type, value))
  {}
};

struct ISongPositionPointerMessage : virtual IMessage {
  static constexpr uint8_t Status = 0xF2;
  static constexpr size_t Size = 3;

  uint8_t lsb() const {
    return data()[1];
  }

  uint8_t msb() const {
    return data()[2];
  }

  uint16_t position() const {
    return detail::to_14bit_unsigned(msb(), lsb());
  }
};

struct SongPositionPointerMessage : virtual ISongPositionPointerMessage,
                                    NonSysExMessage {
  using ISongPositionPointerMessage::Status;

  explicit SongPositionPointerMessage(uint8_t msb, uint8_t lsb) noexcept
    : NonSysExMessage(Status, lsb, msb)
  {}

  explicit SongPositionPointerMessage(uint16_t uint14Value) noexcept
    : NonSysExMessage(Status, detail::from_14bit_lsb(uint14Value),
                      detail::from_14bit_msb(uint14Value))
  {}
};

struct ISongSelectMessage : virtual IMessage {
  static constexpr uint8_t Status = 0xF3;
  static constexpr size_t Size = 2;

  uint8_t songNumber() const {
    return data()[1];
  }
};

struct SongSelectMessage : virtual ISongSelectMessage, NonSysExMessage {
  using ISongSelectMessage::Status;

  explicit SongSelectMessage(uint8_t songNumber) noexcept
    : NonSysExMessage(Status, songNumber)
  {}
};

struct ITuneRequestMessage : virtual IMessage {
  static constexpr uint8_t Status = 0xF6;
  static constexpr size_t Size = 1;
};

struct TuneRequestMessage : virtual ITuneRequestMessage, NonSysExMessage {
  using ITuneRequestMessage::Status;

  explicit TuneRequestMessage() noexcept
    : NonSysExMessage(Status)
  {}
};

enum class RealTimeStatus : uint8_t {
  Undefined     = 0,
  TimingClock   = 0xF8,
  Reserved1     = 0xF9,
  Start         = 0xFA,
  Continue      = 0xFB,
  Stop          = 0xFC,
  Reserved2     = 0xFD,
  ActiveSensing = 0xFE,
  Reset         = 0xFF,
};

template<RealTimeStatus SC = RealTimeStatus::Undefined>
struct IRealTimeMessage;

template<>
struct IRealTimeMessage<> : virtual IMessage {
  static constexpr size_t Size = 1;

  RealTimeStatus status() const {
    return static_cast<RealTimeStatus>(data()[0]);
  }
};

template<RealTimeStatus SC>
struct IRealTimeMessage : virtual IRealTimeMessage<> {
  static constexpr uint8_t Status = static_cast<uint8_t>(SC);
  using IRealTimeMessage<>::Size;

  RealTimeStatus status() const {
    return SC;
  }
};

using ITimingClockMessage   = IRealTimeMessage<RealTimeStatus::TimingClock>;
using IStartMessage         = IRealTimeMessage<RealTimeStatus::Start>;
using IContinueMessage      = IRealTimeMessage<RealTimeStatus::Continue>;
using IStopMessage          = IRealTimeMessage<RealTimeStatus::Stop>;
using IActiveSensingMessage = IRealTimeMessage<RealTimeStatus::ActiveSensing>;
using IResetMessage         = IRealTimeMessage<RealTimeStatus::Reset>;

template<RealTimeStatus SC = RealTimeStatus::Undefined>
struct RealTimeMessage;

template<>
struct RealTimeMessage<> : virtual IRealTimeMessage<>, NonSysExMessage {
  explicit RealTimeMessage(RealTimeStatus status) noexcept
    : NonSysExMessage(static_cast<uint8_t>(status))
  {}
};

template<RealTimeStatus SC>
struct RealTimeMessage : virtual IRealTimeMessage<SC>, RealTimeMessage<> {
  using IRealTimeMessage<SC>::Status;

  explicit RealTimeMessage() noexcept
    : RealTimeMessage<>(SC)
  {}
};

using TimingClockMessage   = RealTimeMessage<RealTimeStatus::TimingClock>;
using StartMessage         = RealTimeMessage<RealTimeStatus::Start>;
using ContinueMessage      = RealTimeMessage<RealTimeStatus::Continue>;
using StopMessage          = RealTimeMessage<RealTimeStatus::Stop>;
using ActiveSensingMessage = RealTimeMessage<RealTimeStatus::ActiveSensing>;
using ResetMessage         = RealTimeMessage<RealTimeStatus::Reset>;

} // namespace a8::midi

A8_PRAGMA_MS(warning(pop));

#endif // A8_MIDI_MESSAGE_H_
