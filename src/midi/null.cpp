#include "a8/midi/access.h"
#include "a8/midi/port.h"

namespace a8::midi {

class NullPort : public virtual Port {
public:
  std::future<Port *> open() noexcept override final {
    std::promise<Port *> promise;
    promise.set_value(this);
    if (!_isConnected) {
      _isConnected = true;
      _stateChangeListener(this);
    }
    return promise.get_future();
  }

  std::future<Port *> close() noexcept override final {
    std::promise<Port *> promise;
    promise.set_value(this);
    if (_isConnected) {
      _isConnected = false;
      _stateChangeListener(this);
    }
    return promise.get_future();
  }

  DeviceState state() const noexcept override final {
    return DeviceState::Connected;
  }

  ConnectionState connection() const noexcept override final {
    return _isConnected ? ConnectionState::Open : ConnectionState::Closed;
  }

protected:
  bool _isConnected = false;
};

class NullInput final : public NullPort, public Input {
public:
  NullInput()
    : Port("null", "null", "null", "0")
  {}
};

class NullOutput final : public NullPort, public Output {
public:
  NullOutput()
    : Port("null", "null", "null", "0")
  {}

  void send(size_t length, const unsigned char *data, uint32_t time = 0)
            override {}
  void clear() override {}
};

class NullAccess : public Access {
public:
  explicit NullAccess(Options options)
    : Access(options)
  {
    auto [inputLock, inputs] = inputsWrite();
    auto [outputLock, outputs] = outputsWrite();

    inputs.emplace("null", std::make_shared<NullInput>());
    outputs.emplace("null", std::make_shared<NullOutput>());
  }
};

template<>
std::future<std::shared_ptr<Access>>
Access::request<Backend::Null>(Options options) {
  std::promise<std::shared_ptr<Access>> promise;
  promise.set_value(std::make_shared<NullAccess>(options));
  return promise.get_future();
}

} // namespace a8::midi
