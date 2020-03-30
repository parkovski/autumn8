#include "a8/midi/midi.h"
#include "a8/util/utfconv.h"

#include <vector>
#include <iostream>
#include <string>
#include <string_view>
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm")

using namespace a8;
using namespace a8::util::utfconv;

static std::ostream &operator<<(std::ostream &os, const GUID &guid) {
  os << std::hex
    << '{' << guid.Data1 << '-' << guid.Data2 << '-' << guid.Data3
    << '-';
  for(int i = 0; i < 8; ++i) {
    os << (int)guid.Data4[i];
  }
  os << '}';
  return os;
}

namespace a8::midi_impl::winmm {

struct WinmmPort {
  WinmmPort(UINT id, const MIDIINCAPS2W &caps)
    : id(id), name(to_utf8(caps.szPname))
  {}

  WinmmPort(UINT id, const MIDIOUTCAPS2W &caps)
    : id(id), name(to_utf8(caps.szPname))
  {}

  UINT id;
  std::string name;
};

template<typename P>
class PortManager : public midi::PortManager<P> {
public:
  using base = midi::PortManager<P>;
  using typename base::promise_type;
  using typename base::future_type;
  using typename base::iterator;

  explicit PortManager() {}
  ~PortManager() {}

  static future_type create() {
    promise_type promise;
    auto unique_self = std::make_unique<PortManager>();
    unique_self->collect();
    promise.set_value(std::move(unique_self));
    return promise.get_future();
  }

  void collect();

  bool hasNext(size_t index) const noexcept override {
    return index < _ports.size();
  }

  iterator begin() const noexcept override {
    if (_ports.empty()) {
      return {this};
    }
    return {this, 0};
  }

  typename P::future_type open() override {
    throw nullptr;
  }

private:
  std::vector<WinmmPort> _ports;
};

template<>
void PortManager<midi::InputPort>::collect() {
  std::lock_guard<std::mutex> lock{_mutex};
  _ports.clear();
  auto count = midiInGetNumDevs();
  _ports.reserve(count);
  for(decltype(count) i = 0; i < count; ++i) {
    MIDIINCAPS2W caps;
    midiInGetDevCapsW(i, reinterpret_cast<MIDIINCAPSW *>(&caps),
                      sizeof(caps));
    _ports.emplace_back(i, caps);
    Log::debug("MIDI In: {} / Mfg: {} / Name: {} / Product: {} /"
               "Support: {} / DriverVer: {} / Mid: {} / Pid: {}",
               _ports.back().name, caps.ManufacturerGuid,
               caps.NameGuid, caps.ProductGuid, caps.dwSupport,
               caps.vDriverVersion, caps.wMid, caps.wPid);
  }
}

template<>
void PortManager<midi::OutputPort>::collect() {
  std::lock_guard<std::mutex> lock{_mutex};
  _ports.clear();
  auto count = midiInGetNumDevs();
  _ports.reserve(count);
  for(decltype(count) i = 0; i < count; ++i) {
    MIDIOUTCAPS2W caps;
    if (
      midiOutGetDevCapsW(
        i, reinterpret_cast<MIDIOUTCAPSW *>(&caps), sizeof(caps)
      ) == MMSYSERR_NOERROR
    ) {
      _ports.emplace_back(i, caps);
      Log::debug("MIDI Out: {} / Mfg: {} / Name: {} / Product: {} /"
                 "Support: {} / DriverVer: {} / Mid: {} / Pid: {} /"
                 "ChMask: {} / Notes: {} / Tech: {} / Voices: {}",
                 _ports.back().name, caps.ManufacturerGuid,
                 caps.NameGuid, caps.ProductGuid, caps.dwSupport,
                 caps.vDriverVersion, caps.wMid, caps.wPid,
                 caps.wChannelMask, caps.wNotes, caps.wTechnology,
                 caps.wVoices);
    }
  }
}

template class PortManager<midi::InputPort>;
template class PortManager<midi::OutputPort>;

} // namespace a8::midi_impl::winmm

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::create<Backend::Winmm>() -> future_type {
  return midi_impl::winmm::PortManager<InputPort>::create();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::Winmm>() -> future_type {
  return midi_impl::winmm::PortManager<OutputPort>::create();
}

} // namespace a8::midi
