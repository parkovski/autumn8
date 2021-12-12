#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

int a8main(int argc, char *argv[]) {
  auto win = new Window("Browser");
  win->resize(0, 0, 500, 500);
  win->quitOnClose();
  win->show();
  return runMessageLoop();
}
