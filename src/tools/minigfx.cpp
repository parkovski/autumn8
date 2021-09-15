#include "a8/a8.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"
//#include "a8/gfx/view.h"
//#include "a8/gfx/renderer.h"

using namespace a8::gfx;

int a8main(int argc, char *argv[]) {
  Window mainWindow;
  mainWindow.resize(0, 0, 500, 500);
  mainWindow.connect<SystemEvent>([](auto &&w, auto &&e) -> bool {
    a8::Log::print("systemevent {}\n", e);
    return true;
  });

  mainWindow.setView(new WebView);

  if (!mainWindow.open("MiniGfx")) {
    return 1;
  }

  return runMessageLoop();
}
