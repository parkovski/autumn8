#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

namespace a8::plat {
inline namespace Linux {

void PlatformState::gfxinit() noexcept {
  gtk_init(&argc, &argv);
}

void PlatformState::gfxdeinit() const noexcept {
}

} // inline namespace Linux
} // namespace a8::plat
