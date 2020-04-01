#include "a8/midi/midi.h"
#include "a8/util/utfconv.h"

#include <exception>
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
  os << std::dec << '}';
  return os;
}

namespace a8::midi_impl::winmm {

class InputPort final : public midi::InputPort {
public:
  using base = midi::InputPort;
  using typename base::promise_type;
  using typename base::future_type;

  ~InputPort() {
    if (_handle) {
      midiInClose(_handle);
    }
  }

  static void CALLBACK midiInProc(
    HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
    DWORD_PTR dwParam1, DWORD_PTR dwParam2
  ) {
    // auto port = reinterpret_cast<InputPort *>(dwInstance);
  }

  static future_type create(UINT deviceId) {
    promise_type promise;
    auto unique_self = std::make_unique<InputPort>();
    auto result = midiInOpen(
      &unique_self->_handle, deviceId,
      reinterpret_cast<DWORD_PTR>(&InputPort::midiInProc),
      reinterpret_cast<DWORD_PTR>(unique_self.get()),
      CALLBACK_FUNCTION | MIDI_IO_STATUS
    );
    switch (result) {
      case MMSYSERR_NOERROR:
        promise.set_value(std::move(unique_self));
        break;
      default:
        promise.set_exception(
          std::make_exception_ptr(std::runtime_error("midiInOpen"))
        );
        break;
    }

    return promise.get_future();
  }

private:
  HMIDIIN _handle = nullptr;
};

class OutputPort final : public midi::OutputPort {
public:
  using base = midi::OutputPort;
  using typename base::promise_type;
  using typename base::future_type;

  ~OutputPort() {
    if (_handle) {
      midiOutClose(_handle);
    }
  }

  static void CALLBACK midiOutProc(
    HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance,
    DWORD_PTR dwParam1, DWORD_PTR dwParam2
  ) {
    // auto port = reinterpret_cast<OutputPort *>(dwInstance);
  }

  static future_type create(UINT deviceId) {
    promise_type promise;
    auto unique_self = std::make_unique<OutputPort>();
    auto result = midiOutOpen(
      &unique_self->_handle, deviceId,
      reinterpret_cast<DWORD_PTR>(&OutputPort::midiOutProc),
      reinterpret_cast<DWORD_PTR>(unique_self.get()),
      CALLBACK_FUNCTION
    );
    switch (result) {
      case MMSYSERR_NOERROR:
        promise.set_value(std::move(unique_self));
        break;
      default:
        promise.set_exception(
          std::make_exception_ptr(std::runtime_error("midiOutOpen"))
        );
    }

    return promise.get_future();
  }

private:
  HMIDIOUT _handle = nullptr;
};

struct PortInfo {
  PortInfo(UINT id, const MIDIINCAPS2W &caps)
    : id(id), nameGuid(caps.NameGuid), name(to_utf8(caps.szPname))
  {}

  PortInfo(UINT id, const MIDIOUTCAPS2W &caps)
    : id(id), nameGuid(caps.NameGuid), name(to_utf8(caps.szPname))
  {}

  UINT id;
  GUID nameGuid;
  std::string name;
};

template<typename P>
class PortManager final : public midi::PortManager<typename P::base> {
public:
  using base = midi::PortManager<typename P::base>;
  using typename base::promise_type;
  using typename base::future_type;
  using typename base::iterator;

  explicit PortManager() {}
  ~PortManager() {}

  std::future<void> scan() override {
    // TODO: If a lock can't be acquired now, the scan should be performed
    // later.
    std::lock_guard<std::mutex> lock{this->_mutex};
    realScan();
    std::promise<void> promise;
    promise.set_value();
    return promise.get_future();
  }

  void realScan();

  void startWatching() override {
    Log::warn("winmm watcher APIs not implemented");
  }

  void stopWatching() override {
    Log::warn("winmm watcher APIs not implemented");
  }

  bool hasNext(size_t index) const noexcept override {
    return index < _ports.size();
  }

  size_t size() const noexcept override {
    return _ports.size();
  }

  iterator begin() const noexcept override {
    if (_ports.empty()) {
      return {this};
    }
    return {this, 0};
  }

  typename P::future_type open(const iterator &it) override {
    // TODO: Check to make sure device indexes haven't changed.
    return P::create(base::iteratorIndex(it));
  }

private:
  std::vector<PortInfo> _ports;
};

template<>
void PortManager<InputPort>::realScan() {
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
void PortManager<OutputPort>::realScan() {
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

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi_impl::winmm

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::create<Backend::Winmm>() noexcept
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::winmm;
  return util::make_unique_nothrow<impl::PortManager<impl::InputPort>>();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::Winmm>() noexcept
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::winmm;
  return util::make_unique_nothrow<impl::PortManager<impl::OutputPort>>();
}

} // namespace a8::midi
