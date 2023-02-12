#ifndef A8_MIDI_CONTROL_H_
#define A8_MIDI_CONTROL_H_

#include <cstdint>
#include <cassert>
#include <iosfwd>
#include <string_view>
#include <iterator>
#include <array>

#include <fmt/ostream.h>

namespace a8::midi {

enum class Control : uint8_t {
  BankSelect,
  ModWheel,
  Breath,
  CC3,
  Foot,
  PortamentoTime,
  DataEntryMSB,
  ChannelVolume,
  Balance,
  CC9,
  Pan,
  Expression,
  Effect1,
  Effect2,
  CC14,
  CC15,
  General1,
  General2,
  General3,
  General4,
  CC20,
  CC21,
  CC22,
  CC23,
  CC24,
  CC25,
  CC26,
  CC27,
  CC28,
  CC29,
  CC30,
  CC31,
  BankSelectLSB,
  ModWheelLSB,
  BreathLSB,
  CC3LSB,
  FootLSB,
  PortamentoTimeLSB,
  DataEntryLSB,
  ChannelVolumeLSB,
  BalanceLSB,
  CC9LSB,
  PanLSB,
  ExpressionLSB,
  Effect1LSB,
  Effect2LSB,
  Control14LSB,
  Control15LSB,
  General1LSB,
  General2LSB,
  General3LSB,
  General4LSB,
  CC20LSB,
  CC21LSB,
  CC22LSB,
  CC23LSB,
  CC24LSB,
  CC25LSB,
  CC26LSB,
  CC27LSB,
  CC28LSB,
  CC29LSB,
  CC30LSB,
  CC31LSB,
  SustainOnOff,
  PortamentoOnOff,
  SostenutoOnOff,
  SoftPedalOnOff,
  LegatoOnOff,
  Hold2OnOff,
  Sound1, SoundVariation = Sound1,
  Sound2, HarmonicIntensity = Sound2,
  Sound3, ReleaseTime = Sound3,
  Sound4, AttackTime = Sound4,
  Sound5, Brightness = Sound5,
  Sound6, DecayTime = Sound6,
  Sound7, VibratoRate = Sound7,
  Sound8, VibratoDepth = Sound8,
  Sound9, VibratoDelay = Sound9,
  Sound10,
  General5,
  General6,
  General7,
  General8,
  Portamento,
  CC85,
  CC86,
  CC87,
  HiResVelocityPrefix,
  CC89,
  CC90,
  // formerly External Effects Depth
  Effect1Depth, ReverbSendLevel = Effect1Depth,
  // formerly Tremolo Depth
  Effect2Depth, TremoloDepth = Effect2Depth,
  // formerly Chorus Depth
  Effect3Depth, ChorusSendLevel = Effect3Depth,
  // formerly Celeste [Detune] Depth
  Effect4Depth, CelesteDetuneDepth = Effect4Depth,
  // formerly Phaser Depth
  Effect5Depth, PhaserDepth = Effect5Depth,
  DataIncrement,
  DataDecrement,
  NRPNLSB,
  NRPNMSB,
  RPNLSB,
  RPNMSB,
  CC102,
  CC103,
  CC104,
  CC105,
  CC106,
  CC107,
  CC108,
  CC109,
  CC110,
  CC111,
  CC112,
  CC113,
  CC114,
  CC115,
  CC116,
  CC117,
  CC118,
  CC119,
  AllSoundOff,
  ResetAllControllers,
  LocalControlOnOff,
  AllNotesOff,
  OmniModeOff,
  OmniModeOn,
  MonoModeOn,
  PolyModeOn,
};

static_assert((uint8_t)Control::BankSelect ==   0, "Control number mismatch");
static_assert((uint8_t)Control::Balance    ==   8, "Control number mismatch");
static_assert((uint8_t)Control::Effect2    ==  13, "Control number mismatch");
static_assert((uint8_t)Control::General4   ==  19, "Control number mismatch");
static_assert((uint8_t)Control::Portamento ==  84, "Control number mismatch");
static_assert((uint8_t)Control::RPNMSB     == 101, "Control number mismatch");
static_assert((uint8_t)Control::PolyModeOn == 127, "Control number mismatch");

extern std::string_view ControlNames[128];
std::ostream &operator<<(std::ostream &os, Control control);

namespace detail {

struct defined_control_tag_t{};
struct defined_control_and_channel_mode_tag_t : defined_control_tag_t{};

// Forward iterator for named control values.
struct defined_control_iterator {
  typedef Control value_type;
  typedef void difference_type;
  typedef Control reference;
  typedef void pointer;
  typedef std::forward_iterator_tag iterator_category;

  constexpr defined_control_iterator(uint8_t value) noexcept
    : value(value)
  {}

  defined_control_iterator(const defined_control_iterator &) = default;

  defined_control_iterator &
  operator=(const defined_control_iterator &) = default;

  constexpr bool operator==(const defined_control_iterator &other) const {
    return value == other.value;
  }
  constexpr bool operator!=(const defined_control_iterator &other) const {
    return !(*this == other);
  }
  constexpr bool operator<(const defined_control_iterator &other) const {
    return value < other.value;
  }
  constexpr bool operator<=(const defined_control_iterator &other) const {
    return value <= other.value;
  }
  constexpr bool operator>(const defined_control_iterator &other) const {
    return value > other.value;
  }
  constexpr bool operator>=(const defined_control_iterator &other) const {
    return value >= other.value;
  }

  constexpr defined_control_iterator &operator++() {
    switch (value) {
      case 2: value = 4; break;
      case 8: value = 10; break;
      case 13: value = 16; break;
      case 19: value = 32; break;
      case 34: value = 36; break;
      case 40: value = 42; break;
      case 51: value = 64; break;
      case 84: value = 88; break;
      case 88: value = 91; break;
      case 101: value = 120; break;
      default: ++value; break;
    }
    return *this;
  }

  constexpr defined_control_iterator operator++(int) {
    defined_control_iterator it = *this;
    ++*this;
    return it;
  }

  constexpr Control operator*() const {
    return static_cast<Control>(value);
  }

private:
  uint8_t value;
};

constexpr defined_control_iterator
begin(const defined_control_tag_t &) {
  return defined_control_iterator(0);
}

constexpr defined_control_iterator
end(const defined_control_tag_t &) {
  return defined_control_iterator(120);
}

constexpr defined_control_iterator
end(const defined_control_and_channel_mode_tag_t &) {
  return defined_control_iterator(128);
}

} // namespace detail

// Enumerate all control values with a predefined name.
constexpr detail::defined_control_tag_t DefinedControls;
// Enumerate all control values with a predefined name, including channel mode
// messages.
constexpr detail::defined_control_and_channel_mode_tag_t
  DefinedControlsAndChannelModes;

enum class ChannelMode {
  AllSoundOff = 120,
  ResetAllControllers,
  LocalControlOnOff,
  AllNotesOff,
  OmniModeOff,
  OmniModeOn,
  MonoModeOn,
  PolyModeOn,
};
static_assert((uint8_t)ChannelMode::PolyModeOn == 127,
              "Channel mode number mismatch");

std::ostream &operator<<(std::ostream &os, ChannelMode mode);

struct RPN {
  constexpr uint16_t value() const {
    return (uint16_t)msb << 7 | lsb;
  }

  /// \see https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2
  constexpr bool isDefined() const {
    if (msb == 0) { return lsb <= 6; }
    else if (msb == 0x3D) { return lsb <= 8; }
    return msb == 0x7F && lsb == 0x7F;
  }

  uint8_t msb;
  uint8_t lsb;
};

std::ostream &operator<<(std::ostream &os, RPN rpn);

namespace rpn {
  constexpr static RPN PitchBendSensitivity = {0, 0};
  constexpr static RPN ChannelFineTuning    = {0, 1};
  constexpr static RPN ChannelCorseTuning   = {0, 2};
  constexpr static RPN TuningProgramChange  = {0, 3};
  constexpr static RPN TuningBankSelect     = {0, 4};
  constexpr static RPN ModDepthRange        = {0, 5};
  constexpr static RPN MPEConfig            = {0, 6};

  constexpr static RPN AzimuthAngle      = {0x3D, 0};
  constexpr static RPN ElevationAngle    = {0x3D, 1};
  constexpr static RPN Gain              = {0x3D, 2};
  constexpr static RPN DistanceRatio     = {0x3D, 3};
  constexpr static RPN MaxDistance       = {0x3D, 4};
  constexpr static RPN GainAtMaxDistance = {0x3D, 5};
  constexpr static RPN RefDistanceRatio  = {0x3D, 6};
  constexpr static RPN PanSpreadAngle    = {0x3D, 7};
  constexpr static RPN RollAngle         = {0x3D, 8};

  constexpr static RPN Null = {0x7F, 0x7F};
} // namespace rpn

constexpr std::array<RPN, 17> DefinedRPNs {
  rpn::PitchBendSensitivity,
  rpn::ChannelFineTuning,
  rpn::ChannelCorseTuning,
  rpn::TuningProgramChange,
  rpn::TuningBankSelect,
  rpn::ModDepthRange,
  rpn::MPEConfig,

  rpn::AzimuthAngle,
  rpn::ElevationAngle,
  rpn::Gain,
  rpn::DistanceRatio,
  rpn::MaxDistance,
  rpn::GainAtMaxDistance,
  rpn::RefDistanceRatio,
  rpn::PanSpreadAngle,
  rpn::RollAngle,

  rpn::Null,
};

} // namespace a8::midi

namespace fmt {
  template<> struct formatter<a8::midi::Control> : ostream_formatter {};
  template<> struct formatter<a8::midi::ChannelMode> : ostream_formatter {};
  template<> struct formatter<a8::midi::RPN> : ostream_formatter {};
}

#endif // A8_MIDI_CONTROL_H_
