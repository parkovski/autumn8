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
  Window *mainWindow = new Window;
  mainWindow->resize(0, 0, 500, 500);
  mainWindow->quitOnClose();

  auto icon16 = getfile("C:\\dev\\autumn8\\res\\size16.png");
  auto icon64 = getfile("C:\\dev\\autumn8\\res\\size64.png");
  auto icon256 = getfile("C:\\dev\\autumn8\\res\\size256.png");

  auto *webview = new WebView("http://appusr.a8.parkovski.com/index.html");
  webview->setUserCallback([&](auto *wv, auto const &req, auto &res) -> bool {
    res.status = 200;
    res.reason = "OK";
    res.contentType = "text/html";
    res.content = "<html><head>"
      "<link rel=\"icon\" href=\"/icon.png\"/>"
      "<title>Custom Page</title></head><body>";
    if (a8::ends_with(std::string_view{req.uri}, "/index.html"sv)) {
      res.content
        .append("<a href=\"http://appres.a8.parkovski.com/test.html\">Resource page</a><br>")
        .append("<a href=\"http://appusr.a8.parkovski.com/test.html\">User page</a><br>")
        .append("<a href=\"http://appusr.a8.parkovski.com/test2.html\">User page 2</a><br>");
    } else if (a8::ends_with(std::string_view{req.uri}, ".png"sv)) {
      res.contentType = "image/png";
      res.content.clear();
      if (a8::ends_with(req.uri, "/icon.png")) {
        res.content.assign(icon64.first.get(), icon64.second);
      } else if (a8::ends_with(req.uri, "/icon16.png")) {
        res.content.assign(icon16.first.get(), icon16.second);
      } else if (a8::ends_with(req.uri, "/icon256.png")) {
        res.content.assign(icon256.first.get(), icon256.second);
      } else {
        return false;
      }
      return true;
    } else {
      res.content.append("You navigated to ").append(req.uri);
    }
    res.content.append("</body></html>");

    return true;
  });
  mainWindow->setView(webview);

  if (!mainWindow->open("Minigfx")) {
    return 1;
  }

  auto x = runMessageLoop();
  a8::Log::debug("Message loop returned {}.", x);
  return x;
}
