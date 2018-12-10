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

namespace a8 {

struct DevInfo {
  std::string name;
  WDE::DeviceInformation winDevInfo;
  int inPort;
  int outPort;

  static constexpr int portDisconnected = -1;
  static constexpr int portInvalid = -2;
  static constexpr bool isPortConnected(int id) {
    return id >= 0;
  }
};

class UwpMidiManager final : public MidiManager {
  WDE::DeviceWatcher _midiInWatcher;
  WDE::DeviceWatcher _midiOutWatcher;

  mutable std::mutex _mutex;
  mutable std::wstring _temp;
  std::vector<DevInfo> _deviceInfo;
  std::unordered_map<std::wstring, size_t> _deviceIdMap;
  std::unordered_multimap<std::wstring, size_t> _deviceNameMap;
  std::vector<WDM::MidiInPort> _inPorts;
  std::vector<WDM::MidiOutPort> _outPorts;
  std::function<void (int, bool)> _callback;

  template<bool In>
  void deviceAdded(const WDE::DeviceWatcher &, const WDE::DeviceInformation &info) {
    int deviceId;
    {
      std::lock_guard<std::mutex> lock(_mutex);

      auto index = _deviceInfo.size();
      auto id = info.Id();
      auto [it, inserted] = _deviceIdMap.emplace(std::wstring_view(id), index);
      auto name = info.Name();
      // std::wcout << L"\nDevice " << name.operator std::wstring_view() << L":\n";
      // std::wcout << L"  " << info.Properties().Size() << L" properties\n";
      auto foo = info.Properties().First();
      while (foo) {
        // std::wcout << foo.Current().Key().operator std::wstring_view() << L": ";
        WF::IPropertyValue bob;
        if (foo.Current().Value() && foo.Current().Value().try_as<WF::IPropertyValue>(bob)) {
          if (bob.Type() == WF::PropertyType::String) {
            // std::wcout << bob.GetString().operator std::wstring_view();
          }
        }
        // std::wcout << L"\n";
        ++foo;
      }
      if (inserted) {
        _deviceInfo.emplace_back(DevInfo {
          ::to_utf8(name),
          info,
          In ? DevInfo::portDisconnected : DevInfo::portInvalid,
          In ? DevInfo::portInvalid : DevInfo::portDisconnected
        });
      } else {
        index = it->second;
        _deviceInfo[index] = DevInfo {
          ::to_utf8(name),
          info,
          In ? DevInfo::portDisconnected : DevInfo::portInvalid,
          In ? DevInfo::portInvalid : DevInfo::portDisconnected
        };
      }
      _deviceNameMap.emplace(std::wstring_view(name), index);
      deviceId = (int)index;
    }
    _callback(deviceId, true);
  }

  template<bool In>
  void deviceRemoved(const WDE::DeviceWatcher &, const WDE::DeviceInformationUpdate &update) {
    int deviceId;
    {
      std::lock_guard<std::mutex> lock(_mutex);

      auto it = _deviceIdMap.find(_temp = update.Id());
      if (it == _deviceIdMap.end()) {
        return;
      }
      auto &di = _deviceInfo[it->second];
      di.inPort = di.outPort = DevInfo::portInvalid;
      auto range = _deviceNameMap.equal_range(to_utf16(di.name));
      for (; range.first != range.second; ++range.first) {
        // Erase the index with the device index as the removed device.
        // it = iterator from unique id, range.first = current element with same name.
        if (range.first->second == it->second) {
          _deviceNameMap.erase(range.first);
          break;
        }
      }
      deviceId = (int)it->second;
    }
    _callback(deviceId, false);
  }

  void deviceUpdated(const WDE::DeviceWatcher &, const WDE::DeviceInformationUpdate &update) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _deviceIdMap.find(_temp = update.Id());
    if (it == _deviceIdMap.end()) {
      return;
    }
    _deviceInfo[it->second].winDevInfo.Update(update);
  }

  template<bool In>
  void enumerationCompleted(const WDE::DeviceWatcher &, const WF::IInspectable &) {
    // When both are done, emit a devices loaded message
  }

  template<bool In>
  void enumerationStopped(const WDE::DeviceWatcher &, const WF::IInspectable &) {
    // Emit a devices loading canceled message.
  }

  static void emptyCallback(int, bool) {}

public:
  UwpMidiManager()
    : _midiInWatcher(WDE::DeviceInformation::CreateWatcher(WDM::MidiInPort::GetDeviceSelector())),
      _midiOutWatcher(WDE::DeviceInformation::CreateWatcher(WDM::MidiOutPort::GetDeviceSelector())),
      _callback(emptyCallback)
  {
    using DW = const WDE::DeviceWatcher &;
    using DI = const WDE::DeviceInformation &;
    using DIU = const WDE::DeviceInformationUpdate &;
    using obj = const WF::IInspectable &;

    _midiInWatcher.Added({ this, &UwpMidiManager::deviceAdded<true> });
    _midiInWatcher.Removed({ this, &UwpMidiManager::deviceRemoved<true> });
    _midiInWatcher.Updated({ this, &UwpMidiManager::deviceUpdated });
    _midiInWatcher.EnumerationCompleted({ this, &UwpMidiManager::enumerationCompleted<true> });
    _midiInWatcher.Stopped({ this, &UwpMidiManager::enumerationStopped<true> });

    _midiOutWatcher.Added({ this, &UwpMidiManager::deviceAdded<false> });
    _midiOutWatcher.Removed({ this, &UwpMidiManager::deviceRemoved<false> });
    _midiOutWatcher.Updated({ this, &UwpMidiManager::deviceUpdated });
    _midiOutWatcher.EnumerationCompleted({ this, &UwpMidiManager::enumerationCompleted<false> });
    _midiOutWatcher.Stopped({ this, &UwpMidiManager::enumerationStopped<false> });
  }

  virtual void watchDevices(std::function<void (int, bool)> callback) override {
    std::lock_guard<std::mutex> lock(_mutex);
    _callback = callback;
    for (auto watcher : {&_midiInWatcher, &_midiOutWatcher}) {
      switch (watcher->Status()) {
      case WDE::DeviceWatcherStatus::Created:
      case WDE::DeviceWatcherStatus::Stopped:
      case WDE::DeviceWatcherStatus::Aborted:
        watcher->Start();
        break;
      default:
        break;
      }
    }
  }

  virtual void stopWatchingDevices() override {
    std::lock_guard<std::mutex> lock(_mutex);
    _callback = emptyCallback;
    for (auto watcher : {&_midiInWatcher, &_midiOutWatcher}) {
      if (watcher->Status() == WDE::DeviceWatcherStatus::Created) {
        continue;
      }
      watcher->Stop();
    }
  }

  virtual bool isDeviceValid(int id) const override {
    if (id < 0 || id >= _deviceInfo.size()) {
      return false;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    return (
      _deviceInfo[id].inPort != DevInfo::portInvalid ||
      _deviceInfo[id].outPort != DevInfo::portInvalid
    );
  }

  virtual bool isInputDevice(int id) const override {
    if (id < 0 || id >= _deviceInfo.size()) {
      return false;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    return _deviceInfo[id].inPort != DevInfo::portInvalid;
  }

  virtual std::string_view deviceName(int id) const override {
    if (id < 0 || id >= _deviceInfo.size()) {
      return std::string_view();
    }
    Sleep(100);
    //std::lock_guard<std::mutex> lock(_mutex);
    return _deviceInfo[id].name;
  }

  virtual int findNamedDevice(std::string_view name) const override {
    auto wname = to_utf16(name);
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _deviceNameMap.find(wname);
    if (it == _deviceNameMap.end()) {
      return -1;
    }
    return (int)it->second;
  }
};

void uwpMidiInitWinRT() {
  winrt::init_apartment();
}

MidiManager *newUwpMidiManager() {
  return new UwpMidiManager;
}

} // namespace a8
