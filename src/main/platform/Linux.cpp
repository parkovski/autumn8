#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

#include <pthread.h>

namespace a8::platform {
inline namespace Linux {

bool setThreadName(const char *name) {
  return pthread_setname_np(pthread_self(), name) == 0;
}

void PlatformState::init(int argc, char **argv) noexcept {
  this->argc = argc;
  this->argv = argv;

  setThreadName("Main");

  gfxinit();
}

void PlatformState::deinit() const noexcept {
  gfxdeinit();
}

PlatformState g_platformState;

} // inline namespace Linux
} // namespace a8::platform
