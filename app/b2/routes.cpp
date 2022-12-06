#include "routes.h"
#include "browser.h"

#include <nlohmann/json.hpp>

#include <functional>

using namespace a8::app::browser;
using namespace nlohmann;

Routes::Routes(Router<Request> &r, Browser &browser)
  : browser{browser}
{
  using namespace std::placeholders;

#define ROUTE(Path, Fn) r[Path] = std::bind(&Routes::Fn, this, _1, _2)

  ROUTE("/view/:name", viewFile);
  ROUTE("/cwd", cwd);
  ROUTE("/ls", ls);
  ROUTE("/go/:...", go);
  ROUTE("/dev/enabled", devEnabled);
  ROUTE("/dev/visible", devVisible);
  ROUTE("/dev/:show", devShow);
  ROUTE("/log/:level/:message", log);

#undef ROUTE
}

void Routes::viewFile(const Request &req, Response &res) {
  std::filesystem::path path = browser.curdir() / req[0];
  std::error_code noone_cares;
  if (!std::filesystem::is_regular_file(path, noone_cares)) {
    res.status(http::HTTP_404_Not_Found);
    return;
  }
  res.sendFile(path.string());
}

void Routes::cwd(const Request &req, Response &res) {
  res.contentType("text/plain") << browser.curdir().string();
}

void Routes::go(const Request &req, Response &res) {
  std::filesystem::path path{"/"};
  for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
    path /= *it;
  }
  if (browser.go_to(path)) {
    res.contentType("application/json") << json(browser.list());
  } else {
    res.status(http::HTTP_404_Not_Found);
  }
}

void Routes::ls(const Request &req, Response &res) {
  auto list = browser.list();
  if (list.name.empty()) {
    res.status(http::HTTP_404_Not_Found);
    return;
  }
  res.contentType("application/json") << json(browser.list());
}

void Routes::devEnabled(const Request &req, Response &res) {
  res << req.webview()->devToolsEnabled();
}

void Routes::devVisible(const Request &req, Response &res) {
  res << req.webview()->devToolsVisible();
}

void Routes::devShow(const Request &req, Response &res) {
  bool show;
  if (req[0] == "show") {
    show = true;
  } else if (req[0] == "hide") {
    show = false;
  } else if (req[0] == "toggle") {
    show = !req.webview()->devToolsVisible();
  } else {
    res.status(http::HTTP_400_Bad_Request);
    return;
  }

  if (req.webview()->devToolsSetVisible(show)) {
    res.status(http::HTTP_200_OK) << (show ? "true" : "false");
  } else {
    res.status(http::HTTP_403_Forbidden);
  }
}

void Routes::log(const Request &req, Response &res) {
  spdlog::level::level_enum level = spdlog::level::off;
  auto level_str = req[0];
  if (level_str == "critical") {
    level = spdlog::level::critical;
  } else if (level_str == "error") {
    level = spdlog::level::err;
  } else if (level_str == "warn") {
    level = spdlog::level::warn;
  } else if (level_str == "info") {
    level = spdlog::level::info;
  } else if (level_str == "debug") {
    level = spdlog::level::debug;
  } else if (level_str == "trace") {
    level = spdlog::level::trace;
  } else if (level_str == "print") {
    A8_PRINT("{}", req[1]);
    return;
  } else if (level_str == "eprint") {
    A8_EPRINT("{}", req[1]);
    return;
  }

  if (level != spdlog::level::off) {
    A8_LOGGER.log(level, req[1]);
  }
}
