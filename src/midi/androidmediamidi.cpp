#include "a8/midi/midi.h"

using namespace a8;

namespace a8::midi {

class NullPortManager final : public PortManager {
public:
  explicit NullPortManager();
};

template<>
auto PortManager::createInternal<Backend::AndroidMediaMidi>()
  -> std::unique_ptr<PortManager>
{
  return std::make_unique<NullPortManager>();
}

} // namespace a8::midi
