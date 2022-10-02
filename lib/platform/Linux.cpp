#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

#include <pthread.h>
#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>

namespace a8::platform {
inline namespace Linux {

std::string getExePath() {
  std::string path;
  size_t size = 0;
  ssize_t readsize;
  do {
    size += PATH_MAX;
    path.resize(size);
    readsize = readlink("/proc/self/exe", path.data(), size);
    if (readsize == -1) {
      readsize = 0;
      break;
    }
  } while (static_cast<size_t>(readsize) == size);

  path.resize(static_cast<size_t>(readsize));
  return path;
}

std::string getExeDir() {
  std::string fullPath = getExePath();
  fullPath.c_str();
  return dirname(fullPath.data());
}

std::string getExeName() {
  std::string fullPath = getExePath();
  fullPath.c_str();
  return basename(fullPath.data());
}

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
