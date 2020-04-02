#include "a8/midi/midi.h"
#include "a8/util/utfconv.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string>
#include <string_view>
#include <sstream>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Midi.h>

#pragma comment(lib, "windowsapp")

namespace WF = winrt::Windows::Foundation;
namespace WDM = winrt::Windows::Devices::Midi;
namespace WDE = winrt::Windows::Devices::Enumeration;

using namespace a8::util::utfconv;

namespace a8::midi_impl::uwp {

static std::string to_string(const WF::IPropertyValue &pv) {
  if (!pv) {
    return "<null>";
  }
  switch (pv.Type()) {
    case WF::PropertyType::String:
      return to_utf8(pv.GetString());
    case WF::PropertyType::Guid:
      return to_utf8(winrt::to_hstring(pv.GetGuid()));
    case WF::PropertyType::Boolean:
      return pv.GetBoolean() ? "true" : "false";
    default:
      return "???";
  }
}

class InputPort : public midi::InputPort {
public:
  using base = midi::InputPort;
  using typename base::promise_type;
  using typename base::future_type;
  using winrt_type = WDM::MidiInPort;
  static constexpr const char *direction = "In";
  static constexpr const char *baseName = "MIDIIN";

  explicit InputPort(std::string name, WDM::MidiInPort port)
    : midi::InputPort(std::move(name)),
      _port(std::move(port))
  {
    _port.MessageReceived({this, &InputPort::onMessage});
    Log::debug("Initialized input port for {}", this->name());
  }

  void onMessage(const WDM::MidiInPort &port,
                 const WDM::MidiMessageReceivedEventArgs &args) {
    Log::trace("MIDI Message: {}: {}", name(), (int)args.Message().Type());
  }

private:
  WDM::MidiInPort _port;
};

class OutputPort : public midi::OutputPort {
public:
  using base = midi::OutputPort;
  using typename base::promise_type;
  using typename base::future_type;
  using winrt_type = WDM::MidiOutPort;
  static constexpr const char *direction = "Out";
  static constexpr const char *baseName = "MIDIOUT";

  explicit OutputPort(std::string name, WDM::IMidiOutPort port)
    : midi::OutputPort(std::move(name)),
      _port(std::move(port))
  {
    Log::debug("Initialized output port for {}", this->name());
  }

private:
  WDM::IMidiOutPort _port;
};

struct PortInfo {
  explicit PortInfo(
    const WDE::DeviceInformation &info,
    const winrt::guid &containerGuid,
    std::string name
  )
    : id{info.Id()},
      kind{info.Kind()},
      containerGuid{containerGuid},
      name{std::move(name)}
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
  winrt::guid containerGuid;
  std::string name;
};

struct ContainerInfo {
  // This is the container GUID for the GS Wavetable Synth.
  static constexpr const winrt::guid wackyGuid = {
    0, 0, 0, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
  };
  ContainerInfo(std::string name, size_t firstDeviceIndex)
    : name{std::move(name)},
      devices{{firstDeviceIndex}}
  {
  }

  // Create a winmm compatible name.
  template<typename P>
  std::string nextDeviceName() const {
    std::stringstream name;
    name << P::baseName << (devices.size() + 1)
         << " (" << this->name << ")";
    return name.str();
  }

  std::string name;
  std::vector<size_t> devices;
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

  size_t size() const noexcept override {
    return _ports.size();
  }

  typename P::future_type open(const iterator &it) override {
    typename P::promise_type promise;
    auto future = promise.get_future();
    auto const &pi = _ports[this->iteratorIndex(it)];
    auto const &id = pi.id;
    P::winrt_type::FromIdAsync(id).Completed(
      [promise = std::move(promise), name = pi.name]
      (auto &&sender, auto &&status) mutable {
        promise.set_value(std::make_unique<P>(std::move(name),
                                              sender.GetResults()));
      }
    );
    return future;
  }

private:
  void onDeviceAdded(const WDE::DeviceWatcher &,
                     const WDE::DeviceInformation &info) {
    std::lock_guard<std::mutex> lock{this->_mutex};

    auto containerIdProp
      = info.Properties().Lookup(L"System.Devices.ContainerId");
    if (containerIdProp) {
      auto const &containerId =
        containerIdProp.as<WF::IPropertyValue>().GetGuid();

      if (containerId == ContainerInfo::wackyGuid) {
        _ports.emplace_back(info, containerId, to_utf8(info.Name()));
      } else if (auto it = _containers.find(containerId);
                 it != _containers.end()) {
        _ports.emplace_back(info, containerId, it->second.nextDeviceName<P>());
        it->second.devices.push_back(_ports.size() - 1);
      } else {
        auto const &dev = WDE::DeviceInformation::CreateFromIdAsync(
          winrt::to_hstring(containerId),
          nullptr,
          WDE::DeviceInformationKind::DeviceContainer
        ).get();
        auto containerName = to_utf8(dev.Name());
        _ports.emplace_back(info, containerId, containerName);
        _containers.try_emplace(containerId, std::move(containerName),
                            _ports.size() - 1);
      }
    }

    Log::debug(
      "MIDI {} Added: {} / Name: {} / Id: {} / Kind: {}",
      P::direction, _ports.back().name, to_utf8(info.Name()),
      to_utf8(info.Id()), (int)info.Kind()
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
        // TODO: Fix container references.
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

  struct guid_hash {
    size_t operator()(const winrt::guid &value) const noexcept {
      return winrt::impl::hash_data(&value, sizeof(value));
    }
  };

  WDE::DeviceWatcher _watcher;
  std::vector<PortInfo> _ports;
  std::unordered_map<winrt::guid, ContainerInfo, guid_hash> _containers;
};

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi_impl::uwp

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::create<Backend::UWP>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::uwp;
  return std::make_unique<impl::PortManager<impl::InputPort>>();
}

template<> template<>
auto PortManager<OutputPort>::create<Backend::UWP>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::uwp;
  return std::make_unique<impl::PortManager<impl::OutputPort>>();
}

void uwpMidiInitWinRT() {
  winrt::init_apartment();
}

} // namespace a8::midi
