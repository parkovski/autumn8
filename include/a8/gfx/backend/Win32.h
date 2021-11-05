#if !defined(A8_A8_H_) || !defined(A8_INCLUDE_GFX_BACKEND)
# error "Do not include this file directly; use a8/a8.h and A8_INCLUDE_GFX_BACKEND"
#endif

namespace a8::platform {
inline namespace Windows {

enum class DpiStatus { Error = 0, Ok, Fallback };
DpiStatus enableThreadDpiAwareness();

extern const wchar_t *const A8WindowClass;

LRESULT CALLBACK mainWndProc(HWND, UINT, WPARAM, LPARAM);

} // inline namespace Windows
} // namespace a8::platform
