#if !defined(A8_A8_H_) || !defined(A8_INCLUDE_PLATFORM)
# error "Do not include this file directly; use a8/a8.h and A8_INCLUDE_PLATFORM"
#endif

namespace a8::plat {

struct PlatformStateCommon {
  int argc = 0;
  char **argv = nullptr;
};

} // namespace a8::plat