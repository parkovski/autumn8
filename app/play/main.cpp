#include "a8/window/window.h"
#include "a8/window/msgloop.h"
#include "a8/webview/webview.h"
#include "a8/util/router.h"

#include <string_view>
#include <filesystem>

using namespace std::string_view_literals;
//namespace stdfs = std::filesystem;

using namespace a8;
using namespace a8::win;
using namespace a8::wv;

namespace {

void configureRouter(Router<WebViewRouterRequest> &r) {
}

} // anonymous namespace

int a8main(int argc, char *argv[]) {
  auto win = new Window("Autumn8 Play");
  win->resize(0, 0, 800, 600);
  win->quitOnClose();

  auto webview = new WebView(win);
  webview->devToolsSetEnabled();

  Router<WebViewRouterRequest> router;
  configureRouter(router);
  webview->setUserCallback(std::ref(router));

  webview->navigate("a8://res/index.html");
  win->show();

  if (argc > 1 && argv[1] == "-dev"sv) {
    webview->devToolsSetVisible(true);
  }

  return runMessageLoop();
}
