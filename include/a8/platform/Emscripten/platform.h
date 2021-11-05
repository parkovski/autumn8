#if !defined(A8_A8_H_) || !defined(A8_INCLUDE_PLATFORM)
# error "Do not include this file directly; use a8/a8.h and A8_INCLUDE_PLATFORM"
#endif

namespace a8::platform {
inline namespace Emscripten {

bool setThreadName(const char *name);

extern struct GlobalPlatformInit {
  void init() noexcept;
  void deinit() const noexcept;

private:
  void gfxinit() noexcept;
  void gfxdeinit() const noexcept;
} g_platformInit;

} // inline namespace Emscripten
} // namespace a8::platform