#include "a8/midi/midi.h"

using namespace a8;

namespace a8::midi {

class NullInputPort final : public InputPort {
  bool _connected = false;

public:
  static constexpr Port::id_t id = 1;
  explicit NullInputPort() noexcept
    : InputPort(id, "NullInputPort")
  {}

  bool deviceConnected() const noexcept override { return true; }
  Connection connection() const noexcept override {
    return _connected
      ? Connection::Connected
      : Connection::Disconnected;
  }
  std::future<Port *> close() override {
    std::promise<Port *> promise;
    promise.set_value(this);
    _connected = false;
    return promise.get_future();
  }
  std::future<InputPort *> open() override {
    std::promise<InputPort *> promise;
    promise.set_value(this);
    _connected = true;
    return promise.get_future();
  }
};

class NullOutputPort final : public OutputPort {
  bool _connected = false;

public:
  static constexpr Port::id_t id = 2;
  explicit NullOutputPort() noexcept
    : OutputPort(id, "NullOutputPort")
  {}

  bool deviceConnected() const noexcept override { return true; }
  Connection connection() const noexcept override {
    return _connected
      ? Connection::Connected
      : Connection::Disconnected;
  }
  std::future<Port *> close() override {
    std::promise<Port *> promise;
    promise.set_value(this);
    _connected = false;
    return promise.get_future();
  }
  std::future<OutputPort *> open() override {
    std::promise<OutputPort *> promise;
    promise.set_value(this);
    _connected = true;
    return promise.get_future();
  }
  void send(const unsigned char *begin, const unsigned char *end,
            Port::timestamp_t timestamp) override {}
};

class NullPortManager final : public PortManager {
public:
  explicit NullPortManager() {
    auto &map = portMap();
    auto input = std::make_shared<NullInputPort>();
    auto output = std::make_shared<NullOutputPort>();
    map.emplace(1, input);
    map.emplace(2, output);
  }
};

template<>
auto PortManager::createInternal<Backend::Null>()
  -> std::unique_ptr<PortManager>
{
  return std::make_unique<NullPortManager>();
}

} // namespace a8::midi
