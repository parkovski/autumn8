#include "routes.h"
#include "browser.h"

#include "a8/window/window.h"
#include "a8/window/msgloop.h"
#include "a8/webview/webview.h"
#include "a8/util/router.h"

#include <filesystem>

using namespace a8;
using namespace a8::win;
using namespace a8::wv;
using namespace a8::app::browser;

int a8main(int argc, char *argv[]) {
  auto win = new Window("Browser");
  win->resize(0, 0, 800, 600);
  win->quitOnClose();

  auto wv = new WebView(win);
  wv->devToolsSetEnabled();

  Browser browser;
  browser.go_to("/mnt/hdd1/music/wavetables/AKWF-FREE-master/AKWF");

  Router<WebViewRouterRequest> router;
  Routes routes(router, browser);
  wv->setUserCallback(std::ref(router));

  wv->navigate("a8://res/index.html");

  win->show();

  using std::string_view_literals::operator""sv;
  if (argc > 1 && argv[1] == "-dev"sv) {
    wv->devToolsSetVisible(true);
  }

  return runMessageLoop();
}
