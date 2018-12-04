#include "a8/midi/midi.h"
#include "a8/util/utfconv.h"

#include <vector>
#include <iostream>
#include <string>
#include <string_view>
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm")

using namespace a8::util::utfconv;

namespace a8 {

struct MmDevice {
  std::string name;
  UINT id;

  MmDevice(const WCHAR *name, UINT id)
    : name(to_utf8(name)), id(id)
  {}
};

class WinmmMidiManager final : public MidiManager {
  std::vector<MmDevice> _inPorts;
  std::vector<MmDevice> _outPorts;

  static constexpr int OutMask = 0x10000000;

  const MmDevice *operator[](int index) const {
    if (index < 0) {
      return nullptr;
    }
    if (index & OutMask) {
      index &= ~OutMask;
      if (index >= _outPorts.size()) {
        return nullptr;
      }
      return &_outPorts[index];
    }
    if (index >= _inPorts.size()) {
      return nullptr;
    }
    return &_inPorts[index];
  }

public:
  virtual void watchDevices(std::function<void (int, bool)> f) override {
    UINT numIn = midiInGetNumDevs();
    UINT numOut = midiOutGetNumDevs();
    for (UINT i = 0; i < numIn; ++i) {
      MIDIINCAPS2W caps;
      if (midiInGetDevCapsW(
          i, reinterpret_cast<LPMIDIINCAPSW>(&caps), sizeof(MIDIINCAPS2W)
      ) == MMSYSERR_NOERROR)
      {
        _inPorts.push_back({caps.szPname, i});
        f(i, true);
      }
    }
    for (UINT o = 0; o < numOut; ++o) {
      MIDIOUTCAPS2W caps;
      if (midiOutGetDevCapsW(
        o, reinterpret_cast<LPMIDIOUTCAPSW>(&caps), sizeof(MIDIOUTCAPS2W)
      ) == MMSYSERR_NOERROR)
      {
        _outPorts.push_back({caps.szPname, o});
        f(o | OutMask, true);
      }
    }
  }

  virtual void stopWatchingDevices() override { }

  virtual bool isDeviceValid(int id) const override {
    return !!(*this)[id];
  }

  virtual bool isInputDevice(int id) const override {
    return (id & OutMask) == 0;
  }

  virtual std::string_view deviceName(int id) const override {
    auto *dev = (*this)[id];
    if (!dev) {
      return "";
    }
    return dev->name;
  }

  virtual int findNamedDevice(std::string_view name) const override {
    return InvalidDeviceId;
  }
};

MidiManager *newWinmmMidiManager() {
  return new WinmmMidiManager;
}

}
