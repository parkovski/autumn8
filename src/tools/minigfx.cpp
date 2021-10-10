#include "a8/a8.h"
#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <sstream>

using namespace a8::gfx;

int a8main(int argc, char *argv[]) {
  Window *mainWindow = new Window;
  mainWindow->resize(0, 0, 500, 500);
  mainWindow->quitOnClose();

  auto *webview = new WebView("http://apprc.parkovski.com/test.html");
  mainWindow->setView(webview);

  if (!mainWindow->open("Minigfx")) {
    return 1;
  }

  auto x = runMessageLoop();
  a8::Log::debug("Message loop returned {}.", x);
  return x;
}
