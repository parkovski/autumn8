#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

namespace a8::platform {
inline namespace Linux {

bool setThreadName(const char *name) {
  return false;
}

void GlobalPlatformInit::init() noexcept {
  setThreadName("Main");
  gfxinit();
}

void GlobalPlatformInit::deinit() const noexcept {
  gfxdeinit();
}

} // inline namespace Linux
} // namespace a8::platform
