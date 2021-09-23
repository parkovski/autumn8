#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

namespace a8::platform {
inline namespace windows {

const wchar_t *const A8WindowClass = L"Autumn8WndClass";

DpiStatus enableThreadDpiAwareness() {
  static CallSite<decltype(&SetThreadDpiAwarenessContext)> fn(
    L"user32.dll", "SetThreadDpiAwarenessContext"
  );
  if (fn && fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    return DpiStatus::Ok;
  }

  static CallSite<HRESULT (WINAPI *)(int)> fnFallback(
    L"shcore.dll", "SetProcessDpiAwareness"
  );
  if (fnFallback && fnFallback(2 /* PROCESS_PER_MONITOR_DPI_AWARE */)) {
    return DpiStatus::Fallback;
  };
  return DpiStatus::Error;
}

void GlobalPlatformInit::gfxinit() noexcept {
  enableThreadDpiAwareness();

  WNDCLASSEXW cls{};
  cls.cbSize = sizeof(WNDCLASSEXW);
  cls.style = CS_HREDRAW | CS_VREDRAW;
  cls.lpfnWndProc = mainWndProc;
  cls.hInstance = hInstance;
  cls.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  cls.hCursor = LoadCursor(nullptr, IDC_ARROW);
  cls.lpszClassName = A8WindowClass;
  cls.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

  if (!RegisterClassEx(&cls)) {
    Log::error("RegisterClassEx failed.");
  }

  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
}

void GlobalPlatformInit::gfxdeinit() const noexcept {
  CoUninitialize();
  if (!UnregisterClass(A8WindowClass, hInstance)) {
    Log::warn("UnregisterClass failed.");
  }
}

} // inline namespace windows
} // namespace a8::platform
