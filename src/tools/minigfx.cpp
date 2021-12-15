#include "a8/a8.h"
#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace a8::gfx;
using namespace std::string_view_literals;

static
std::pair<std::unique_ptr<char[]>, size_t>
getfile(std::filesystem::path path) noexcept {
  std::fstream file{path, std::ios::in | std::ios::binary};
  if (!file.good()) {
    return {nullptr, 0};
  }
  file.seekg(0);
  auto p0 = file.tellg();
  file.seekg(0, std::ios::end);
  auto p1 = file.tellg();
  auto size = p1 - p0;
  auto buf = std::make_unique<char[]>(size);
  file.seekg(0);
  file.read(buf.get(), size);
  return {std::move(buf), size};
}

int a8main(int argc, char *argv[]) {
  auto mainWindow = new Window("Minigfx");
  mainWindow->resize(0, 0, 500, 500);
  mainWindow->quitOnClose();

  auto icon16 = getfile("/home/parker/Documents/dev/autumn8/res/size16.png");
  auto icon64 = getfile("/home/parker/Documents/dev/autumn8/res/size64.png");
  auto icon256 = getfile("/home/parker/Documents/dev/autumn8/res/size256.png");

  auto webview = new WebView(mainWindow);
  //webview->navigate("http://appusr.a8.parkovski.com/index.html");
  //webview->navigate("https://www.coinmarketcap.com/");
  webview->setUserCallback([&](auto *wv, auto const &req, auto &res) -> bool {
    res.status(a8::http::HTTP_200_OK).contentType("text/html");

    if (a8::ends_with(req.uri, ".png")) {
      a8::Log::debug("requested png {}", req.uri);
      res.contentType("image/png");
      if (a8::ends_with(req.uri, "/icon.png")) {
        res.write(std::string_view{icon64.first.get(), icon64.second});
      } else if (a8::ends_with(req.uri, "/icon16.png")) {
        res.write(std::string_view{icon16.first.get(), icon16.second});
      } else if (a8::ends_with(req.uri, "/icon256.png")) {
        res.write(std::string_view{icon256.first.get(), icon256.second});
      } else {
        return false;
      }
      return true;
    }

    res.write("<html><head>"
      "<link rel=\"icon\" href=\"/icon.png\"/>"
      "<title>Custom Page</title></head><body>");
    if (a8::ends_with(req.uri, "/index.html"sv)) {
      res
        .write("<a href=\"a8://res/test.html\">Resource page</a><br>")
        .write("<a href=\"a8://app/test.html\">User page</a><br>")
        .write("<a href=\"a8://app/test2.html\">User page 2</a><br>");
    } else {
      res.write("You navigated to ").write(req.uri);
    }
    res.write("</body></html>");

    return true;
  });
  webview->navigate("a8://app/index.html");

  mainWindow->show(true);

  auto x = runMessageLoop();
  a8::Log::debug("Message loop returned {}.", x);
  return x;
}
