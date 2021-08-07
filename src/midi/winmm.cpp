#include "a8/midi/midi.h"
#include "a8/midi/message.h"
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

struct PortInfo {
  PortInfo(UINT id, const MIDIINCAPS2W &caps)
    : id(id), nameGuid(caps.NameGuid), name(caps.szPname)
  {}

  PortInfo(UINT id, const MIDIOUTCAPS2W &caps)
    : id(id), nameGuid(caps.NameGuid), name(caps.szPname)
  {}

  UINT id;
  GUID nameGuid;
  std::wstring name;
};

class InputPort final : public midi::InputPort {
public:
  using base = midi::InputPort;
  using typename base::promise_type;
  using typename base::future_type;

  InputPort(std::string name, UINT deviceId)
    : midi::InputPort(std::move(name)),
      _hdr1{},
      _hdr2{}
  {
    auto result = midiInOpen(
      &_handle, deviceId,
      reinterpret_cast<DWORD_PTR>(&InputPort::midiInProc),
      reinterpret_cast<DWORD_PTR>(this),
      CALLBACK_FUNCTION | MIDI_IO_STATUS
    );

    _hdr1.lpData = _buf1;
    _hdr2.lpData = _buf2;
    _hdr1.dwBufferLength = _hdr2.dwBufferLength = _bufferSize;
    midiInPrepareHeader(_handle, &_hdr1, sizeof(MIDIHDR));
    midiInPrepareHeader(_handle, &_hdr2, sizeof(MIDIHDR));
    midiInAddBuffer(_handle, &_hdr1, sizeof(MIDIHDR));
    midiInAddBuffer(_handle, &_hdr2, sizeof(MIDIHDR));

    switch (result) {
      case MMSYSERR_NOERROR:
        switch (midiInStart(_handle)) {
          case MMSYSERR_NOERROR:
            break;
          default:
            throw std::runtime_error("midiInStart");
        }
        break;
      default:
        throw std::runtime_error("midiInOpen");
    }
  }

  void destroy() override {
    Log::debug("winmm::InputPort::destroy({})", name());
    if (_handle) {
      midiInStop(_handle);
      midiInReset(_handle);
      midiInClose(_handle);
      _handle = nullptr;
    }
  }

  void start() override {
    switch (midiInStart(_handle)) {
      case MMSYSERR_NOERROR:
        break;
      default:
        break;
    }
  }

  void stop() override {
    midiInStop(_handle);
  }

  static void CALLBACK midiInProc(
    HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
    DWORD_PTR dwParam1, DWORD_PTR dwParam2
  ) {
    auto port = reinterpret_cast<InputPort *>(dwInstance);
    switch (wMsg) {
      case MIM_OPEN:
        Log::trace("{}: MIM_OPEN", port->name());
        break;

      case MIM_CLOSE:
        Log::trace("{}: MIM_CLOSE", port->name());
        delete port;
        break;

      case MIM_MOREDATA:
        // Anything else to do for this?
      case MIM_DATA:
        {
          // timestamp = dwParam2
          unsigned char data[3];
          data[0] = (unsigned char)(dwParam1);
          data[1] = (unsigned char)(dwParam1 >> 8);
          data[2] = (unsigned char)(dwParam1 >> 16);
          Log::trace("{}: MIM_DATA: {:X}", port->name(), data[0]);
          midi::NonSysExMessage message{data[0], data[1], data[2]};
          port->emit(message, (uint32_t)dwParam2);
        }
        break;

      case MIM_LONGDATA:
        {
          auto hdr = reinterpret_cast<MIDIHDR *>(dwParam1);
          // timestamp = dwParam2
          Log::trace("{}: MIM_LONGDATA: {}B", port->name(),
                     hdr->dwBytesRecorded);
          // port->emit(reinterpret_cast<unsigned char *>(hdr->lpData),
          //            hdr->dwBytesRecorded);

          if (hdr->dwFlags & MHDR_DONE) {
            midiInUnprepareHeader(hMidiIn, hdr, sizeof(MIDIHDR));
          } else if (port->_handle) {
            hdr->dwBytesRecorded = 0;
            hdr->dwOffset = 0;
            midiInAddBuffer(port->_handle, hdr, sizeof(MIDIHDR));
          }
        }
        break;

      case MIM_ERROR:
        Log::trace("{}: MIM_ERROR", port->name());
        break;

      case MIM_LONGERROR:
        Log::trace("{}: MIM_LONGERROR", port->name());
        break;

      default:
        Log::trace("{}: Unknown ({})", port->name(), wMsg);
    }
  }

private:
  static constexpr size_t _bufferSize = 128;

  HMIDIIN _handle = nullptr;
  MIDIHDR _hdr1;
  MIDIHDR _hdr2;
  char _buf1[_bufferSize];
  char _buf2[_bufferSize];
};

class OutputPort final : public midi::OutputPort {
public:
  using base = midi::OutputPort;
  using typename base::promise_type;
  using typename base::future_type;

  OutputPort(std::string name, UINT deviceId)
    : midi::OutputPort(std::move(name))
  {
    auto result = midiOutOpen(
      &_handle, deviceId,
      reinterpret_cast<DWORD_PTR>(&OutputPort::midiOutProc),
      reinterpret_cast<DWORD_PTR>(this),
      CALLBACK_FUNCTION
    );

    switch (result) {
      case MMSYSERR_NOERROR:
        break;
      default:
        throw std::runtime_error("midiOutOpen");
    }
  }

  void destroy() override {
    if (_handle) {
      midiOutClose(_handle);
    }
  }

  static void CALLBACK midiOutProc(
    HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwInstance,
    DWORD_PTR dwParam1, DWORD_PTR dwParam2
  ) {
    auto port = reinterpret_cast<OutputPort *>(dwInstance);
    switch (wMsg) {
      case MOM_OPEN:
        Log::trace("{}: MOM_OPEN", port->name());
        break;

      case MOM_CLOSE:
        Log::trace("{}: MOM_CLOSE", port->name());
        delete port;
        break;

      case MOM_DONE:
        Log::trace("{}: MOM_DONE", port->name());
        break;

      case MOM_POSITIONCB:
        Log::trace("{}: MOM_POSITIONCB", port->name());
        break;
    }
  }

private:
  HMIDIOUT _handle = nullptr;
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

  size_t size() const noexcept override {
    return _ports.size();
  }

  typename P::future_type open(const iterator &it) override {
    // TODO: Check to make sure device indexes haven't changed.
    auto const &pi = _ports[base::iteratorIndex(it)];
    typename P::promise_type promise;
    try {
      promise.set_value(midi::Port::make_unique<P>(to_utf8(pi.name), pi.id));
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
    return promise.get_future();
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
               to_utf8(caps.szPname), caps.ManufacturerGuid,
               caps.NameGuid, caps.ProductGuid, caps.dwSupport,
               caps.vDriverVersion, caps.wMid, caps.wPid);
  }
}

template<>
void PortManager<OutputPort>::realScan() {
  _ports.clear();
  auto count = midiOutGetNumDevs();
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
                 to_utf8(caps.szPname), caps.ManufacturerGuid,
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
auto PortManager<InputPort>::create<Backend::Winmm>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::winmm;
  return std::make_unique<impl::PortManager<impl::InputPort>>();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::Winmm>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::winmm;
  return std::make_unique<impl::PortManager<impl::OutputPort>>();
}

} // namespace a8::midi
