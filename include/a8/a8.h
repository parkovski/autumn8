#ifndef A8_A8_H_
#define A8_A8_H_

#define A8_PLATFORM_NULL       0
#define A8_PLATFORM_Windows    1
#define A8_PLATFORM_macOS      2
#define A8_PLATFORM_iOS        3
#define A8_PLATFORM_Android    4
#define A8_PLATFORM_Linux      5
#define A8_PLATFORM_Emscripten 6

#define A8_GFX_BACKEND_NULL       0
#define A8_GFX_BACKEND_Win32      1
#define A8_GFX_BACKEND_UWP        2
#define A8_GFX_BACKEND_Cocoa      3
#define A8_GFX_BACKEND_CocoaTouch 4
#define A8_GFX_BACKEND_Android    5
#define A8_GFX_BACKEND_Gtk        6
#define A8_GFX_BACKEND_Qt         7
#define A8_GFX_BACKEND_HTML       8

#if defined(A8_INCLUDE_GFX_BACKEND) && !defined(A8_INCLUDE_PLATFORM)
# define A8_INCLUDE_PLATFORM
#endif

#define A8_IS_PLATFORM(Platform) (A8_PLATFORM == A8_PLATFORM_##Platform)
#ifdef A8_INCLUDE_PLATFORM
# define A8_IS_PLATFORM_IMPL(Platform) A8_IS_PLATFORM(Platform)
#else
# define A8_IS_PLATFORM_IMPL(Platform) 0
#endif

#define A8_IS_GFX(Backend) (A8_GFX_BACKEND == A8_GFX_BACKEND_##Backend)
#ifdef A8_INCLUDE_GFX_BACKEND
# define A8_IS_GFX_IMPL(Backend) A8_IS_GFX(Backend)
#else
# define A8_IS_GFX_IMPL(Backend) 0
#endif

#include "config.h"
#include "compiler.h"

#include <cassert>

#include "util/inline.h"
#ifndef A8_NO_LOG
#include "util/log.h"
#endif

extern "C" int a8main(int argc, char *argv[]);

#ifdef A8_INCLUDE_PLATFORM
# include A8_PLATFORM_HEADER
#endif

#ifdef A8_INCLUDE_GFX_BACKEND
# include A8_GFX_BACKEND_HEADER
#endif

#endif /* A8_A8_H_ */
