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
  webview->setUserCallback([&](auto const &req, auto &res) {
    auto path = req.uri().path().str();
    if (a8::ends_with(path, ".png")) {
      a8::Log::debug("requested png {}", path);
      if (a8::ends_with(path, "/icon.png")) {
        res.contentType("image/png");
        res << std::string_view{icon64.first.get(), icon64.second};
      } else if (a8::ends_with(path, "/icon16.png")) {
        res.contentType("image/png");
        res << std::string_view{icon16.first.get(), icon16.second};
      } else if (a8::ends_with(path, "/icon256.png")) {
        res.contentType("image/png");
        res << std::string_view{icon256.first.get(), icon256.second};
      } else {
        res.status(a8::http::HTTP_404_Not_Found);
      }
      return;
    }

    res.status(a8::http::HTTP_200_OK).contentType("text/html");
    res << "<html><head>"
      "<link rel=\"icon\" href=\"/icon.png\"/>"
      "<title>Custom Page</title></head><body>";
    if (a8::ends_with(path, "/index.html"sv)) {
      res
        << "<a href=\"a8://res/test.html\">Resource page</a><br>"
        << "<a href=\"a8://app/test.html\">User page</a><br>"
        << "<a href=\"a8://app/test2.html\">User page 2</a><br>";
    } else {
      res << "You navigated to " << path;
    }
    res << "</body></html>";
  });
  webview->navigate("a8://app/index.html");

  mainWindow->show(true);

  auto x = runMessageLoop();
  a8::Log::debug("Message loop returned {}.", x);
  return x;
}
