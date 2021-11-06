#include "a8/midi/midi.h"
#include <ostream>

namespace a8::midi {

std::ostream &operator<<(std::ostream &os, Backend backend) {
  switch (backend) {
#define BACKEND(be) \
  case Backend::be: \
    os << #be;

A8_MIDI_BACKENDS(BACKEND)

#undef BACKEND

  default:
    A8_UNREACHABLE;
    break;
  }
  return os;
}

Port::~Port() {
  Log::debug("Port {} async destroy finished.", name());
  _destroyPromise.set_value();
}

template<typename P>
PortManager<P>::~PortManager() {}

template<typename P>
auto PortManager<P>::create() -> std::unique_ptr<PortManager> {
#if A8_IS_PLATFORM(Windows)
  // TODO: If UWP not available, return Winmm.
  return createInternal<Backend::UWP>();
#elif A8_IS_PLATFORM(macOS) || A8_IS_PLATFORM(iOS)
  return createInternal<Backend::CoreMIDI>();
#elif A8_IS_PLATFORM(Android)
  // TODO: If AMidi not available, return AndroidMediaMidi.
  return createInternal<Backend::AMidi>();
#elif A8_IS_PLATFORM(Linux)
  return createInternal<Backend::Alsa>();
#elif A8_IS_PLATFORM(Emscripten)
  return createInternal<Backend::WebMIDI>();
#else
# error "Unknown platform"
#endif
}

template<typename P>
auto PortManager<P>::create(Backend backend) -> std::unique_ptr<PortManager> {
  switch (backend) {
    case Backend::Null:
      return createInternal<Backend::Null>();

#if A8_IS_PLATFORM(Windows)
    case Backend::UWP:
      return createInternal<Backend::UWP>();
    case Backend::Winmm:
      return createInternal<Backend::Winmm>();

#elif A8_IS_PLATFORM(macOS) || A8_IS_PLATFORM(iOS)
    case Backend::CoreMIDI:
      return createInternal<Backend::CoreMIDI>();

#elif A8_IS_PLATFORM(Android)
    case Backend::AMidi:
      return createInternal<Backend::AMidi>();
    case Backend::AndroidMediaMidi:
      return createInternal<Backend::AndroidMediaMidi>();

#elif A8_IS_PLATFORM(Linux)
    case Backend::Alsa:
      return createInternal<Backend::Alsa>();

#elif A8_IS_PLATFORM(Emscripten)
    case Backend::WebMIDI:
      return createInternal<Backend::WebMIDI>();
#endif

#if A8_HAS_JACK
    case Backend::Jack:
      return createInternal<Backend::Jack>();
#endif

    default:
      return std::unique_ptr<PortManager>{};
  }
}

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

std::string_view ControlNames[128] = {
  "Bank select",
  "Modulation wheel",
  "Breath controller",
  "CC #3",
  "Foot controller",
  "Portamento time",
  "Data entry MSB",
  "Channel volume",
  "Balance",
  "CC #9",
  "Pan",
  "Expression controller",
  "Effect control 1",
  "Effect control 2",
  "CC #14",
  "CC #15",
  "General purpose controller 1",
  "General purpose controller 2",
  "General purpose controller 3",
  "General purpose controller 4",
  "CC #20"
  "CC #21"
  "CC #22"
  "CC #23"
  "CC #24"
  "CC #25"
  "CC #26"
  "CC #27"
  "CC #28"
  "CC #29"
  "CC #30"
  "CC #31",
  "Bank select LSB",
  "Modulation wheel LSB",
  "Breath controller LSB",
  "CC #3 LSB",
  "Foot controller LSB",
  "Portamento time LSB",
  "Data entry LSB",
  "Channel volume LSB",
  "Balance LSB",
  "CC #9 LSB",
  "Pan LSB",
  "Expression controller LSB",
  "Effect control 1 LSB",
  "Effect control 2 LSB",
  "CC #14 LSB",
  "CC #15 LSB",
  "General purpose controller 1 LSB",
  "General purpose controller 2 LSB",
  "General purpose controller 3 LSB",
  "General purpose controller 4 LSB",
  "CC #20 LSB"
  "CC #21 LSB"
  "CC #22 LSB"
  "CC #23 LSB"
  "CC #24 LSB"
  "CC #25 LSB"
  "CC #26 LSB"
  "CC #27 LSB"
  "CC #28 LSB"
  "CC #29 LSB"
  "CC #30 LSB"
  "CC #31 LSB",
  "Sustain (damper) pedal on/off",
  "Portamento on/off",
  "Sostenuto on/off",
  "Soft pedal on/off",
  "Legato footswitch on/off",
  "Hold 2 on/off",
  "Sound controller 1 (sound variation)",
  "Sound controller 2 (harmonic intensity)",
  "Sound controller 3 (release time)",
  "Sound controller 4 (attack time)",
  "Sound controller 5 (brightness)",
  "Sound controller 6 (decay time)",
  "Sound controller 7 (vibrato rate)",
  "Sound controller 8 (vibrato depth)",
  "Sound controller 9 (vibrato delay)",
  "Sound controller 10",
  "General purpose controller 5",
  "General purpose controller 6",
  "General purpose controller 7",
  "General purpose controller 8",
  "Portamento control",
  "CC #85",
  "CC #86",
  "CC #87",
  "High resolution velocity prefix",
  "CC #89",
  "CC #90",
  "Effect 1 depth (reverb send level)",
  "Effect 2 depth (tremolo depth)",
  "Effect 3 depth (chorus depth)",
  "Effect 4 depth (celeste [detune] depth)",
  "Effect 5 depth (phaser depth)",
  "Data increment",
  "Data decrement",
  "NRPN LSB",
  "NRPN MSB",
  "RPN LSB",
  "RPN MSB",
  "CC #102",
  "CC #103",
  "CC #104",
  "CC #105",
  "CC #106",
  "CC #107",
  "CC #108",
  "CC #109",
  "CC #110",
  "CC #111",
  "CC #112",
  "CC #113",
  "CC #114",
  "CC #115",
  "CC #116",
  "CC #117",
  "CC #118",
  "CC #119",
  "All sound off",
  "Reset all controllers",
  "Local control on/off",
  "All notes off",
  "Omni mode off",
  "Omni mode on",
  "Mono mode on",
  "Poly mode on",
};

std::ostream &operator<<(std::ostream &os, Control control) {
  return os << ControlNames[(uint8_t)control];
}

std::ostream &operator<<(std::ostream &os, ChannelMode mode) {
  return os << ControlNames[(uint8_t)mode];
}

std::ostream &operator<<(std::ostream &os, rpnvalue_t rpn) {
  if (rpn.msb == 0) {
    switch (rpn.lsb) {
      case 0: return os << "Pitch bend sensitivity";
      case 1: return os << "Channel fine tuning";
      case 2: return os << "Channel corse tuning";
      case 3: return os << "Tuning program change";
      case 4: return os << "Tuning bank select";
      case 5: return os << "Modulation depth range";
      case 6: return os << "MPE configuration";
      default: break;
    }
  } else if (rpn.msb == 0x3D) {
    switch (rpn.lsb) {
      case 0: return os << "Azimuth angle";
      case 1: return os << "Elevation angle";
      case 2: return os << "Gain";
      case 3: return os << "Distance ratio";
      case 4: return os << "Maximum distance";
      case 5: return os << "Gain at maximum distance";
      case 6: return os << "Reference distance ratio";
      case 7: return os << "Pan spread angle";
      case 8: return os << "Roll angle";
      default: break;
    }
  } else if (rpn.msb == 0x7F && rpn.lsb == 0x7F) {
    return os << "Null";
  }

  return os << "(unknown)";
}

} // namespace a8::midi
