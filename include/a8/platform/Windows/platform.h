#ifndef A8_PLATFORM_WINDOWS_H_
#define A8_PLATFORM_WINDOWS_H_

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

#include <optional>

namespace a8::platform {
inline namespace windows {

bool setThreadName(const wchar_t *name);

enum class DpiStatus { Error = 0, Ok, Fallback };
DpiStatus enableThreadDpiAwareness();

LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);

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

extern const wchar_t *A8WindowClass;

extern struct GlobalPlatformInit {
  int nCmdShow = SW_HIDE;
  HINSTANCE hInstance = nullptr;
  DWORD dwMainThreadId = 0;
  DpiStatus mainThreadDpiStatus = DpiStatus::Error;
  bool wantsConsole = false;

  void init(HINSTANCE hInstance, int nCmdShow = SW_NORMAL) noexcept;

  // For DLLs.
  void deinit() const noexcept;
} g_platformInit;

} // inline namespace windows
} // namespace a8::platform

#endif // A8_PLATFORM_WINDOWS_H_
