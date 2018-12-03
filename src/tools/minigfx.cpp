#include "a8/a8.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

int a8main(int argc, char *argv[]) {
  Window mainWindow{"MiniGfx"};
  if (!mainWindow.isValid()) {
    return 1;
  }

  return runMessageLoop(mainWindow);
}

