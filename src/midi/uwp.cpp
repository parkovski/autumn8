#include "a8/midi/midi.h"
#include "a8/util/utfconv.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string>
#include <string_view>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Midi.h>

#pragma comment(lib, "windowsapp")

namespace WF = winrt::Windows::Foundation;
namespace WDM = winrt::Windows::Devices::Midi;
namespace WDE = winrt::Windows::Devices::Enumeration;

using namespace a8::util::utfconv;

namespace a8::midi_impl::uwp {

template<typename P> struct winport;
template<> struct winport<midi::InputPort> {
  using type = WDM::MidiInPort;
  static constexpr const char *direction = "In";
};
template<> struct winport<midi::OutputPort> {
  using type = WDM::MidiOutPort;
  static constexpr const char *direction = "Out";
};

template<typename P>
class PortManager : public midi::PortManager<P> {
public:
  using base = midi::PortManager<P>;
  using typename base::promise_type;
  using typename base::future_type;
  using typename base::iterator;

  explicit PortManager()
    : _watcher{WDE::DeviceInformation::CreateWatcher(
        winport<P>::type::GetDeviceSelector()
      )}
  {
    _watcher.Added({this, &PortManager::onDeviceAdded});
    _watcher.Updated({this, &PortManager::onDeviceUpdated});
    _watcher.Removed({this, &PortManager::onDeviceRemoved});
    //_watcher.EnumerationCompleted({this, &PortManager::onEnumerationCompleted});
    //_watcher.Stopped({this, &PortManager::onStopped});
  }

  ~PortManager() {}

  static future_type create() {
    promise_type promise;
    auto self = new PortManager;
    auto future = promise.get_future();
    self->_watcher.EnumerationCompleted(
      [self, promise = std::move(promise)]
      (auto &&watcher, auto &&inspectable) mutable {
        self->onEnumerationCompleted(watcher, inspectable);
        promise.set_value(std::unique_ptr<PortManager>(self));
      }
    );
    // can this throw? how do we handle the promise stuff here?
    self->startWatching();
    return future;
  }

  void startWatching() {
    _watcher.Start();
  }

  void stopWatching() {
    _watcher.Stop();
  }

  bool hasNext(size_t index) const noexcept override {
    return false;
  }

  iterator begin() const noexcept override {
    return {this};
  }

  typename P::future_type open() override {
    throw nullptr;
  }

private:
  void onDeviceAdded(const WDE::DeviceWatcher &,
                     const WDE::DeviceInformation &info) {
    std::lock_guard<std::mutex> lock{_mutex};

    Log::debug(
      "MIDI {} Added: {} / Id: {} / Kind: {}",
      winport<P>::direction, to_utf8(info.Name()), to_utf8(info.Id()),
      (int)info.Kind()
    );
  }

  void onDeviceUpdated(const WDE::DeviceWatcher &,
                       const WDE::DeviceInformationUpdate &info) {
    std::lock_guard<std::mutex> lock{_mutex};

    Log::debug(
      "MIDI {} Update: Id {} / Kind: {}",
      winport<P>::direction, to_utf8(info.Id()), (int)info.Kind()
    );
  }

  void onDeviceRemoved(const WDE::DeviceWatcher &,
                       const WDE::DeviceInformationUpdate &info) {
    std::lock_guard<std::mutex> lock{_mutex};

    Log::debug(
      "MIDI {} Removed: Id {} / Kind {}",
      winport<P>::direction, to_utf8(info.Id()), (int)info.Kind()
    );
  }

  void onEnumerationCompleted(const WDE::DeviceWatcher &,
                              const WF::IInspectable &) {
    Log::debug("MIDI {} Enumeration completed", winport<P>::direction);

    stopWatching();
  }

  void onStopped(const WDE::DeviceWatcher &,
                 const WF::IInspectable &) {
    Log::debug("MIDI {} Stopped", winport<P>::direction);
  }

  WDE::DeviceWatcher _watcher;
};

template class PortManager<midi::InputPort>;
template class PortManager<midi::OutputPort>;

} // namespace a8::midi_impl::uwp

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::create<Backend::UWP>() -> future_type {
  return midi_impl::uwp::PortManager<InputPort>::create();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::UWP>() -> future_type {
  return midi_impl::uwp::PortManager<OutputPort>::create();
}

void uwpMidiInitWinRT() {
  winrt::init_apartment();
}

} // namespace a8::midi
