#include "a8/midi/midi.h"

using namespace a8;

namespace a8::midi {

class NullInputPort final : public InputPort {
  bool _connected = false;

public:
  explicit NullInputPort(boost::uuids::uuid id)
    : InputPort(id, "null")
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
  explicit NullOutputPort(boost::uuids::uuid id)
    : OutputPort(id, "null")
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
    auto input = std::make_shared<NullInputPort>(nextId());
    auto output = std::make_shared<NullOutputPort>(nextId());
    map.emplace(input->id(), input);
    map.emplace(output->id(), output);
  }
};

template<>
auto PortManager::createInternal<Backend::Null>()
  -> std::unique_ptr<PortManager>
{
  return std::make_unique<NullPortManager>();
}

} // namespace a8::midi
