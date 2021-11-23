#if !defined(A8_A8_H_) || !defined(A8_INCLUDE_PLATFORM)
# error "Do not include this file directly; use a8/a8.h and A8_INCLUDE_PLATFORM"
#endif

#include "common.h"
#include <string>

namespace a8::platform {
inline namespace Linux {

std::string getExePath();
bool setThreadName(const char *name);

extern struct PlatformState : PlatformStateCommon {
  int argc;
  char **argv;

  void init(int argc, char **argv) noexcept;
  void deinit() const noexcept;

private:
  // TODO: Move these to a8::gfx
#ifdef A8_HAS_GUI
  void gfxinit() noexcept;
  void gfxdeinit() const noexcept;
#else
  void gfxinit() noexcept {}
  void gfxdeinit() const noexcept {}
#endif
} g_platformState;

} // inline namespace Linux
} // namespace a8::platform