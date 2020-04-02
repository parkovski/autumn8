#include "a8/midi/midi.h"

namespace a8::midi {

Port::~Port() {
  Log::debug("Port {} async destroy finished.", name());
  _destroyPromise.set_value();
}

template<typename P>
PortManager<P>::~PortManager() {}

template<typename P>
auto PortManager<P>::create(Backend backend) -> std::unique_ptr<PortManager> {
  switch (backend) {

#define BACKEND(b)   \
    case Backend::b: \
      return PortManager<P>::template create<Backend::b>();

    A8_MIDI_BACKENDS(BACKEND)

#undef BACKEND

    default:
      A8_UNREACHABLE;
      break;
  }
}

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi
