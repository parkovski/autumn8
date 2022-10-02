#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

namespace a8::platform {
inline namespace Emscripten {

bool setThreadName(const char *name) {
  return false;
}

void PlatformState::init() noexcept {
  setThreadName("Main");
  gfxinit();
}

void PlatformState::deinit() const noexcept {
  gfxdeinit();
}

} // inline namespace Emscripten
} // namespace a8::platform
