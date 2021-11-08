#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/msgloop.h"

using namespace a8::platform::Linux;

namespace a8::gfx {

static int s_messageLoopExitCode = 0;

int runMessageLoop() {
  gtk_main();
  return s_messageLoopExitCode;
}

int runConstantFPSMessageLoop(void (*callback)(), int fps) {
  // TODO
  gtk_main();
  return s_messageLoopExitCode;
}

void postQuitMessage(int exitCode) {
  s_messageLoopExitCode = exitCode;
  gtk_main_quit();
}

} // namespace a8::gfx
