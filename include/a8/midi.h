#pragma once

#include <functional>
#include <string_view>

namespace a8 {

class MidiManager {
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

void uwpMidiInitWinRT();
MidiManager *newUwpMidiManager();

} // namespace a8
