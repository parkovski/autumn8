#include "a8/midi/midi.h"

namespace a8::midi {

InputPort::~InputPort() {}
OutputPort::~OutputPort() {}

template<typename P> PortManager<P>::~PortManager() {}

template<typename P>
auto PortManager<P>::create(Backend backend) -> std::unique_ptr<PortManager> {
  switch (backend) {
#define BACKEND(b)   \
    case Backend::b: \
      return PortManager<P>::template create<Backend::b>();

# ifdef _WIN32
    BACKEND(Winmm)
    BACKEND(UWP)
#endif

#undef BACKEND
    default:
      throw BackendException(backend);
  }
}

template class PortManager<InputPort>;
template class PortManager<OutputPort>;

} // namespace a8::midi
