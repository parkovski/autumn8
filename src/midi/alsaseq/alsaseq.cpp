#include "a8/a8.h"
#include "alsaseq.h"

#include <ostream>

namespace a8::midi {
inline namespace alsaseq {

std::ostream &operator<<(std::ostream &os, AlsaEventType type) {
  switch (type.value) {
#define CASE(Name) case SND_SEQ_EVENT_##Name: os << #Name; break
  CASE(SYSTEM);
  CASE(RESULT);
  CASE(NOTE);
  CASE(NOTEON);
  CASE(NOTEOFF);
  CASE(KEYPRESS);
  CASE(CONTROLLER);
  CASE(PGMCHANGE);
  CASE(CHANPRESS);
  CASE(PITCHBEND);
  CASE(CONTROL14);
  CASE(NONREGPARAM);
  CASE(REGPARAM);
  CASE(SONGPOS);
  CASE(SONGSEL);
  CASE(QFRAME);
  CASE(TIMESIGN);
  CASE(KEYSIGN);
  CASE(START);
  CASE(CONTINUE);
  CASE(STOP);
  CASE(SETPOS_TICK);
  CASE(SETPOS_TIME);
  CASE(TEMPO);
  CASE(CLOCK);
  CASE(TICK);
  CASE(QUEUE_SKEW);
  CASE(SYNC_POS);
  CASE(TUNE_REQUEST);
  CASE(RESET);
  CASE(SENSING);
  CASE(ECHO);
  CASE(OSS);
  CASE(CLIENT_START);
  CASE(CLIENT_EXIT);
  CASE(CLIENT_CHANGE);
  CASE(PORT_START);
  CASE(PORT_EXIT);
  CASE(PORT_CHANGE);
  CASE(PORT_SUBSCRIBED);
  CASE(PORT_UNSUBSCRIBED);
  CASE(USR0);
  CASE(USR1);
  CASE(USR2);
  CASE(USR3);
  CASE(USR4);
  CASE(USR5);
  CASE(USR6);
  CASE(USR7);
  CASE(USR8);
  CASE(USR9);
  CASE(SYSEX);
  CASE(BOUNCE);
  CASE(USR_VAR0);
  CASE(USR_VAR1);
  CASE(USR_VAR2);
  CASE(USR_VAR3);
  CASE(USR_VAR4);
  CASE(NONE);
#undef CASE
  default: os << "(unknown: " << type.value << ")"; break;
  }
  return os;
}

Message to_message(const snd_seq_event_t &ev) {
  switch (ev.type) {
  case SND_SEQ_EVENT_NOTEON:
    return Message::NoteOn(ev.data.note.channel + 1, ev.data.note.note,
                           ev.data.note.velocity);

  case SND_SEQ_EVENT_NOTEOFF:
    return Message::NoteOff(ev.data.note.channel + 1, ev.data.note.note,
                            ev.data.note.velocity);

  case SND_SEQ_EVENT_KEYPRESS:
    return Message::PolyphonicAftertouch(ev.data.note.channel + 1,
                                         ev.data.note.note,
                                         ev.data.note.velocity);

  case SND_SEQ_EVENT_CONTROLLER:
    return Message::ControlChange(ev.data.control.channel + 1,
                                  ev.data.control.param,
                                  (uint8_t)ev.data.control.value);

  case SND_SEQ_EVENT_PGMCHANGE:
    return Message::ProgramChange(ev.data.control.channel + 1,
                                  (uint8_t)ev.data.control.value);

  case SND_SEQ_EVENT_CHANPRESS:
    return Message::ChannelAftertouch(ev.data.control.channel + 1,
                                      (uint8_t)ev.data.control.value);

  case SND_SEQ_EVENT_PITCHBEND:
    return Message::PitchBend(ev.data.control.channel + 1,
                              (int16_t)ev.data.control.value);

  case SND_SEQ_EVENT_CONTROL14:
    // TODO 14-bit controller
    return Message::None();

  case SND_SEQ_EVENT_NONREGPARAM:
    // TODO 14-bit NRPN
    return Message::None();

  case SND_SEQ_EVENT_REGPARAM:
    // TODO 14-bit RPN
    return Message::None();

  case SND_SEQ_EVENT_SONGPOS:
    return Message::SongPosition((uint16_t)ev.data.control.value);

  case SND_SEQ_EVENT_SONGSEL:
    return Message::SongSelect((uint8_t)ev.data.control.value);

  case SND_SEQ_EVENT_QFRAME:
    return Message::QuarterFrame(ev.data.control.param,
                                 (uint8_t)ev.data.control.value);

  case SND_SEQ_EVENT_START:
    return Message::Start();

  case SND_SEQ_EVENT_CONTINUE:
    return Message::Continue();

  case SND_SEQ_EVENT_STOP:
    return Message::Stop();

  case SND_SEQ_EVENT_CLOCK:
    return Message::TimingClock();

  case SND_SEQ_EVENT_TICK:
    // TODO Real time tick
    return Message::None();

  case SND_SEQ_EVENT_TUNE_REQUEST:
    return Message::TuneRequest();

  case SND_SEQ_EVENT_RESET:
    return Message::Reset();

  case SND_SEQ_EVENT_SENSING:
    return Message::ActiveSensing();

  default:
    return Message::None();
  }
}

snd_seq_event_t from_message(const Message &msg) {
  snd_seq_event_t ev{};
  ev.type = SND_SEQ_EVENT_NONE;
  return ev;
}

#define PRINTFLAG(Kind, Flag)               \
  do {                                      \
    if (v & SND_SEQ_PORT_##Kind##_##Flag) { \
      if (first) {                          \
        first = false;                      \
      } else {                              \
        os << ", ";                         \
      }                                     \
      os << #Flag;                          \
    }                                       \
  } while (0)

std::ostream &operator<<(std::ostream &os, AlsaPortCaps caps) {
  unsigned v = caps.value;
  if (v == 0) {
    return os << "<NONE>";
  }
  bool first = true;
  PRINTFLAG(CAP, READ);
  PRINTFLAG(CAP, WRITE);
  PRINTFLAG(CAP, SYNC_READ);
  PRINTFLAG(CAP, SYNC_WRITE);
  PRINTFLAG(CAP, DUPLEX);
  PRINTFLAG(CAP, SUBS_READ);
  PRINTFLAG(CAP, SUBS_WRITE);
  PRINTFLAG(CAP, NO_EXPORT);
  return os;
}

std::ostream &operator<<(std::ostream &os, AlsaPortType type) {
  unsigned v = type.value;
  if (v == 0) {
    return os << "<NONE>";
  }
  bool first = true;
  PRINTFLAG(TYPE, SPECIFIC);
  PRINTFLAG(TYPE, MIDI_GENERIC);
  PRINTFLAG(TYPE, MIDI_GM);
  PRINTFLAG(TYPE, MIDI_GS);
  PRINTFLAG(TYPE, MIDI_XG);
  PRINTFLAG(TYPE, MIDI_MT32);
  PRINTFLAG(TYPE, MIDI_GM2);
  PRINTFLAG(TYPE, SYNTH);
  PRINTFLAG(TYPE, DIRECT_SAMPLE);
  PRINTFLAG(TYPE, SAMPLE);
  PRINTFLAG(TYPE, HARDWARE);
  PRINTFLAG(TYPE, SOFTWARE);
  PRINTFLAG(TYPE, SYNTHESIZER);
  PRINTFLAG(TYPE, PORT);
  PRINTFLAG(TYPE, APPLICATION);
  return os;
}

#undef PRINTFLAG

} // inline namespace alsaseq

template<>
std::future<std::shared_ptr<Access>>
Access::request<Backend::AlsaSeq>(Options options) {
  std::promise<std::shared_ptr<Access>> promise;
  auto future = promise.get_future();
  AlsaAccess::create(std::move(promise), options);
  return future;
}

} // namespace a8::midi
