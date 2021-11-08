#if !defined(A8_A8_H_) || !defined(A8_INCLUDE_PLATFORM)
# error "Do not include this file directly; use a8/a8.h and A8_INCLUDE_PLATFORM"
#endif

#include "common.h"

#ifndef _UNICODE
# define _UNICODE
#endif

#ifndef UNICODE
# define UNICODE
#endif

#ifndef WIN32
# define WIN32
#endif

#ifndef _WINDOWS
# define _WINDOWS
#endif

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <Windows.h>

namespace a8::platform {
inline namespace Windows {

bool setThreadName(const wchar_t *name);
bool setThreadName(const char *name);

template<typename F>
class CallSite {
public:
  explicit CallSite(const wchar_t *dll, const char *fname) noexcept {
    auto dllHandle = GetModuleHandleW(dll);
    if (dllHandle) {
      _cachedFunction = reinterpret_cast<F>(GetProcAddress(dllHandle, fname));
    }
  }

  bool operator==(const CallSite<F> &other) noexcept {
    return _cachedFunction == other._cachedFunction;
  }

  bool operator!=(const CallSite<F> &other) noexcept {
    return !(*this == other);
  }

  explicit operator bool() const noexcept {
    return !!_cachedFunction;
  }

  template<typename... Args>
  decltype(auto) operator()(Args &&...args) const
    noexcept(noexcept(_cachedFunction(std::forward<Args>(args)...)))
  {
    return _cachedFunction(std::forward<Args>(args)...);
  }

private:
  F _cachedFunction;
};

extern struct PlatformState : PlatformStateCommon {
  int nCmdShow = SW_HIDE;
  HINSTANCE hInstance = nullptr;
  DWORD dwMainThreadId = 0;
  bool wantsConsole = false;

public:
  void init(HINSTANCE hInstance, int nCmdShow = SW_NORMAL) noexcept;

  // For DLLs.
  void deinit() const noexcept;

private:
#ifdef A8_HAS_GUI
  void gfxinit() noexcept;
  void gfxdeinit() const noexcept;
#else
  void gfxinit() noexcept {}
  void gfxdeinit() const noexcept {}
#endif
} g_platformState;

} // inline namespace Windows
} // namespace a8::platform
