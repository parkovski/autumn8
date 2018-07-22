#ifndef _a8_midi_h_
#define _a8_midi_h_

#include "a8/a8.h"

#include <functional>
#include <string_view>

namespace a8 {

class A8_DLLEXPORT MidiManager {
public:
  static constexpr int InvalidDeviceId = -1;

  virtual void watchDevices(std::function<void (int, bool)>) = 0;
  virtual void stopWatchingDevices() = 0;

  template<typename T>
  void watchDevices(void (T::*f)(int, bool)) {
    watchDevices(std::mem_fn(f));
  }

  virtual bool isDeviceValid(int id) const = 0;
  virtual bool isInputDevice(int id) const = 0;
  virtual std::string_view deviceName(int id) const = 0;
  virtual int findNamedDevice(std::string_view name) const = 0;
};

A8_DLLEXPORT void uwpMidiInitWinRT();
A8_DLLEXPORT MidiManager *newUwpMidiManager();
A8_DLLEXPORT MidiManager *newWinmmMidiManager();

} // namespace a8

#endif // _a8_midi_h_
