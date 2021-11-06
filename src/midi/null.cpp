#include "a8/midi/midi.h"

using namespace a8;

namespace a8::midi_impl::null_be {

class InputPort final : public midi::InputPort {
public:
  using base = midi::InputPort;
  using typename base::promise_type;
  using typename base::future_type;

  explicit InputPort() : base("") {}

  void start() override {}
  void stop() override {}
};

class OutputPort final : public midi::OutputPort {
public:
  using base = midi::OutputPort;
  using typename base::promise_type;
  using typename base::future_type;

  explicit OutputPort() : base("") {}
};

template<typename P>
class PortManager final : public midi::PortManager<typename P::base> {
public:
  using base = midi::PortManager<typename P::base>;
  using typename base::promise_type;
  using typename base::future_type;
  using typename base::iterator;

  explicit PortManager() {}
  ~PortManager() {}

  std::future<void> scan() override {
    std::promise<void> promise;
    promise.set_value();
    return promise.get_future();
  }
  void startWatching() override {}
  void stopWatching() override {}

  size_t size() const noexcept override {
    return 0;
  }

  typename P::future_type open(const iterator &it) override {
    typename P::promise_type promise;
    // TODO: promise.set_value(???);
    return promise.get_future();
  }
};

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi_impl::null_be

namespace a8::midi {

template<> template<>
auto PortManager<InputPort>::createInternal<Backend::Null>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::null_be;
  return std::make_unique<impl::PortManager<impl::InputPort>>();
}

template<> template<>
auto PortManager<OutputPort>::createInternal<Backend::Null>()
  -> std::unique_ptr<PortManager>
{
  namespace impl = midi_impl::null_be;
  return std::make_unique<impl::PortManager<impl::OutputPort>>();
}

} // namespace a8::midi
