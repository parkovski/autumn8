#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/msgloop.h"
#include "a8/util/router.h"

#include <string_view>
#include <filesystem>

using namespace std::string_view_literals;
namespace stdfs = std::filesystem;

using namespace a8;
using namespace a8::gfx;

namespace {

void configureRouter(Router<WebViewRouterRequest> &r) {
  r["/:..."] = [](auto const &req, auto &res) {
    stdfs::path path{"../../build/web/play"};
    for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
      path /= *it;
    }
    res.sendResFile(path.string());
  };
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

  webview->navigate("a8://app/index.html");
  win->show();

  if (argc > 1 && argv[1] == "-dev"sv) {
    webview->devToolsSetVisible(true);
  }

  return runMessageLoop();
}
