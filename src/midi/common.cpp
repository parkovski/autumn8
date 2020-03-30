#include "a8/midi/midi.h"

using namespace a8::midi;

InputPort::~InputPort() {}
OutputPort::~OutputPort() {}

template<typename P> PortManager<P>::~PortManager() {}

template<typename P>
auto PortManager<P>::create(Backend backend) -> future_type {
  switch (backend) {
#define BACKEND(b)   \
    case Backend::b: \
      return PortManager<P>::template create<Backend::b>();

# ifdef _WIN32
    BACKEND(Winmm)
    BACKEND(UWP)
#endif

#undef BACKEND
    // default:
    //   throw something...
  }
}

namespace a8::midi {
  template class PortManager<InputPort>;
  template class PortManager<OutputPort>;
}
