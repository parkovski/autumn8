#ifndef A8_GFX_PLATFORM_WINDOWS_H_
#define A8_GFX_PLATFORM_WINDOWS_H_

#include <ostream>
#include <Windows.h>

namespace a8::platform::windows {

bool setThreadName(const wchar_t *name);

enum class DpiStatus { Error = 0, Ok, Fallback };
inline std::ostream &operator<<(std::ostream &os, DpiStatus status) {
  switch (status) {
  case DpiStatus::Error: os << "DpiStatus::Error"; break;
  case DpiStatus::Ok: os << "DpiStatus::Ok"; break;
  case DpiStatus::Fallback: os << "DpiStatus::Fallback"; break;
  default: os << "DpiStatus::???"; break;
  }
  return os;
}
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

  CallSite(const CallSite<F> &other) noexcept
    : _cachedFunction(other._cachedFunction)
  {}

  CallSite<F> &operator=(const CallSite<F> &other) noexcept {
    _cachedFunction = other._cachedFunction;
    return *this;
  }

  bool operator==(const CallSite<F> &other) noexcept {
    return _cachedFunction == other._cachedFunction;
  }

  bool operator!=(const CallSite<F> &other) noexcept {
    return !(*this == other);
  }

  operator bool() const {
    return !!_cachedFunction;
  }

  template<typename... Args>
  decltype(auto) operator()(Args &&...args) const {
    return _cachedFunction(std::forward<Args>(args)...);
  }

private:
  F _cachedFunction;
};

extern struct GlobalPlatformInit {
  int nCmdShow;
  HINSTANCE hInstance;
  DWORD dwMsgThreadId;
} g_platformInit;

extern const wchar_t *A8WindowClass;

} // namespace a8::platform::windows

#endif // A8_GFX_PLATFORM_WINDOWS_H_
