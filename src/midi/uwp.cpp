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

class InputPort : public midi::InputPort {
public:
  using base = midi::InputPort;
  using typename base::promise_type;
  using typename base::future_type;
  using winrt_type = WDM::MidiInPort;
  static constexpr const char *direction = "In";

  explicit InputPort(WDM::IMidiInPort port)
    : _port(std::move(port))
  {}

  void foobar() {
  }

private:
  WDM::IMidiInPort _port;
};

class OutputPort : public midi::OutputPort {
public:
  using base = midi::OutputPort;
  using typename base::promise_type;
  using typename base::future_type;
  using winrt_type = WDM::MidiOutPort;
  static constexpr const char *direction = "Out";

  explicit OutputPort(WDM::IMidiOutPort port)
    : _port(std::move(port))
  {}

private:
  WDM::IMidiOutPort _port;
};

struct PortInfo {
  explicit PortInfo(const WDE::DeviceInformation &info)
    : id{info.Id()}, kind{info.Kind()}
  {}

  bool operator==(const WDE::DeviceInformation &info) const {
    return id == info.Id() && kind == info.Kind();
  }
  bool operator!=(const WDE::DeviceInformation &info) const {
    return !(*this == info);
  }

  bool operator==(const WDE::DeviceInformationUpdate &info) const {
    return id == info.Id() && kind == info.Kind();
  }
  bool operator!=(const WDE::DeviceInformationUpdate &info) const {
    return !(*this == info);
  }

  std::wstring id;
  WDE::DeviceInformationKind kind;
};

template<typename P>
class PortManager : public midi::PortManager<typename P::base> {
public:
  using base = midi::PortManager<typename P::base>;
  using typename base::promise_type;
  using typename base::future_type;
  using typename base::iterator;

  explicit PortManager()
    : _watcher{WDE::DeviceInformation::CreateWatcher(
        P::winrt_type::GetDeviceSelector()
      )}
  {
    _watcher.Added({this, &PortManager::onDeviceAdded});
    _watcher.Updated({this, &PortManager::onDeviceUpdated});
    _watcher.Removed({this, &PortManager::onDeviceRemoved});
    //_watcher.EnumerationCompleted({this, &PortManager::onEnumerationCompleted});
    _watcher.Stopped({this, &PortManager::onStopped});
  }

  ~PortManager() {}

  std::future<void> scan() override {
    std::promise<void> promise;
    auto future = promise.get_future();
    winrt::event_token evToken;
    evToken = _watcher.EnumerationCompleted(
      [this, promise = std::move(promise), evToken]
      (auto &&watcher, auto &&inspectable) mutable {
        watcher.EnumerationCompleted(evToken);
        watcher.Stop();
        this->onEnumerationCompleted(watcher, inspectable);
        promise.set_value();
      }
    );
    _ports.clear();
    // can this throw? how do we handle the promise stuff here?
    _watcher.Start();
    return future;
  }

  void startWatching() override {
    Log::warn("Watcher APIs are WIP");
    _watcher.Start();
  }

  void stopWatching() override {
    _watcher.Stop();
  }

  bool hasNext(size_t index) const noexcept override {
    return false;
  }

  size_t size() const noexcept override {
    return _ports.size();
  }

  iterator begin() const noexcept override {
    if (this->empty()) {
      return {this};
    }
    return {this, 0};
  }

  typename P::future_type open(const iterator &it) override {
    typename P::promise_type promise;
    auto future = promise.get_future();
    auto const &id = _ports[this->iteratorIndex(it)].id;
    P::winrt_type::FromIdAsync(id).Completed(
      [promise = std::move(promise)](auto &&sender, auto &&status) mutable {
        promise.set_value(std::make_unique<P>(sender.GetResults()));
      }
    );
    return future;
  }

private:
  void onDeviceAdded(const WDE::DeviceWatcher &,
                     const WDE::DeviceInformation &info) {
    std::lock_guard<std::mutex> lock{this->_mutex};

    _ports.emplace_back(info);

    Log::debug(
      "MIDI {} Added: {} / Id: {} / Kind: {}",
      P::direction, to_utf8(info.Name()), to_utf8(info.Id()),
      (int)info.Kind()
    );
  }

  void onDeviceUpdated(const WDE::DeviceWatcher &,
                       const WDE::DeviceInformationUpdate &info) {
    std::lock_guard<std::mutex> lock{this->_mutex};

    // TODO: What to do here? Anything?

    Log::debug(
      "MIDI {} Update: Id {} / Kind: {}",
      P::direction, to_utf8(info.Id()), (int)info.Kind()
    );
  }

  void onDeviceRemoved(const WDE::DeviceWatcher &,
                       const WDE::DeviceInformationUpdate &info) {
    std::lock_guard<std::mutex> lock{this->_mutex};

    for (auto it = _ports.begin(), end = _ports.end(); it != end; ++it) {
      if (*it == info) {
        _ports.erase(it);
        Log::debug(
          "MIDI {} Removed: Id {} / Kind {}",
          P::direction, to_utf8(info.Id()), (int)info.Kind()
        );
        return;
      }
    }

    Log::warn(
      "MIDI {} No device found to remove for Id {} / Kind {}",
      P::direction, to_utf8(info.Id()), (int)info.Kind()
    );
  }

  void onEnumerationCompleted(const WDE::DeviceWatcher &,
                              const WF::IInspectable &) {
    Log::debug("MIDI {} Enumeration completed", P::direction);
  }

  void onStopped(const WDE::DeviceWatcher &,
                 const WF::IInspectable &) {
    Log::debug("MIDI {} Watcher stopped", P::direction);
  }

  WDE::DeviceWatcher _watcher;
  std::vector<PortInfo> _ports;
};

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi_impl::uwp

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::create<Backend::UWP>() noexcept
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::uwp;
  return util::make_unique_nothrow<impl::PortManager<impl::InputPort>>();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::UWP>() noexcept
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::uwp;
  return util::make_unique_nothrow<impl::PortManager<impl::OutputPort>>();
}

void uwpMidiInitWinRT() {
  winrt::init_apartment();
}

} // namespace a8::midi
