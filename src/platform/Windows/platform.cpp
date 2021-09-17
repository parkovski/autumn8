#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

namespace a8::platform {
inline namespace windows {

bool setThreadName(const wchar_t *name) {
  static CallSite<decltype(&SetThreadDescription)> fn(
    L"kernel32.dll", "SetThreadDescription"
  );
  return fn && fn(GetCurrentThread(), name) == S_OK;
}

void GlobalPlatformInit::init(HINSTANCE hInstance, int nCmdShow) noexcept {
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

void GlobalPlatformInit::deinit() const noexcept {
  gfxdeinit();
}

GlobalPlatformInit g_platformInit;

} // inline namespace windows
} // namespace a8::platform
