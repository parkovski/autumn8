#include "a8/a8.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

int a8main(int argc, char *argv[]) {
  Window mainWindow{"MiniGfx", {0, 0}, {500, 500}};
  if (!mainWindow.isValid()) {
    return 1;
  }

  return runMessageLoop(mainWindow);
}

