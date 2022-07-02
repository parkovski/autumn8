#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"
#include "a8/util/utfconv.h"

namespace a8::platform {
inline namespace Windows {

bool setThreadName(const wchar_t *name) {
  static CallSite<decltype(&SetThreadDescription)> fn(
    L"kernel32.dll", "SetThreadDescription"
  );
  return fn && fn(GetCurrentThread(), name) == S_OK;
}

bool setThreadName(const char *name) {
  return setThreadName(to_utf16(name).c_str());
}

void PlatformState::init(HINSTANCE hInstance, int nCmdShow) noexcept {
  if (this->hInstance) {
    return;
  }

  this->hInstance = hInstance;
  this->nCmdShow = nCmdShow;

  this->dwMainThreadId = GetCurrentThreadId();
  setThreadName(L"Main");

  if (wantsConsole) {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
      AllocConsole();
    }
  }

  gfxinit();
}

void PlatformState::deinit() const noexcept {
  gfxdeinit();
}

PlatformState g_platformState;

} // inline namespace Windows
} // namespace a8::platform
