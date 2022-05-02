#ifndef A8_MIDI_ACCESS_H_
#define A8_MIDI_ACCESS_H_

#include "backend.h"

#include <future>
#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <tuple>

namespace a8::midi {

struct Options {
  bool sysex = false;
};

class Port;
class Input;
class Output;

class Access {
public:
  template<class P>
  using port_map = std::unordered_map<std::string, std::shared_ptr<P>>;

protected:
  explicit Access(Options options)
    : _options{options}
  {}

public:
  template<Backend B>
  static
  std::future<std::shared_ptr<Access>>
  request(Options options = {});

  static
  std::future<std::shared_ptr<Access>>
  request(Backend b, Options options = {});

  static
  std::future<std::shared_ptr<Access>>
  request(Options options = {});

  std::pair<std::shared_lock<std::shared_mutex>, const port_map<Input> &>
  inputsRead() const noexcept {
    std::shared_lock<std::shared_mutex> lock{_inputMutex};
    return std::make_pair(std::move(lock), std::cref(_inputMap));
  }

  std::pair<std::shared_lock<std::shared_mutex>, const port_map<Output> &>
  outputsRead() const noexcept {
    std::shared_lock<std::shared_mutex> lock{_outputMutex};
    return std::make_pair(std::move(lock), std::cref(_outputMap));
  }

  void onStateChange(std::function<void(Access *, Port *)> listener) noexcept {
    _stateChangeListener = std::move(listener);
  }

  bool sysexEnabled() const noexcept {
    return _options.sysex;
  }

protected:
  std::pair<std::unique_lock<std::shared_mutex>, port_map<Input> &>
  inputsWrite() noexcept {
    std::unique_lock<std::shared_mutex> lock{_inputMutex};
    return std::make_pair(std::move(lock), std::ref(_inputMap));
  }

  std::pair<std::unique_lock<std::shared_mutex>, port_map<Output> &>
  outputsWrite() noexcept {
    std::unique_lock<std::shared_mutex> lock{_outputMutex};
    return std::make_pair(std::move(lock), std::ref(_outputMap));
  }

  void emitStateChange(Port *port) noexcept {
    _stateChangeListener(this, port);
  }

private:
  Options _options;
  std::function<void(Access *, Port *)> _stateChangeListener;
  port_map<Input> _inputMap;
  port_map<Output> _outputMap;
  mutable std::shared_mutex _inputMutex;
  mutable std::shared_mutex _outputMutex;
};

} // namespace a8::midi

#endif /* A8_MIDI_ACCESS_H_ */
