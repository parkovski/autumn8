#ifndef A8_A8_H_
#define A8_A8_H_

#if defined(A8_INCLUDE_GFX_BACKEND) && !defined(A8_INCLUDE_PLATFORM)
# define A8_INCLUDE_PLATFORM
#endif

#include "config.h"
#include "compiler.h"

#ifdef A8_INCLUDE_PLATFORM
# include A8_PLATFORM_HEADER
#endif

#ifdef A8_INCLUDE_GFX_BACKEND
# include A8_GFX_BACKEND_HEADER
#endif

#include <cassert>

#include "util/inline.h"
#ifndef A8_NO_LOG
#include "util/log.h"
#endif

extern "C" int a8main(int argc, char *argv[]);

#endif /* A8_A8_H_ */
