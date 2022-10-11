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
  ROUTE("/go/back", goBack);
  ROUTE("/go/back/:count", goBack);
  ROUTE("/go/forward", goForward);
  ROUTE("/go/forward/:count", goForward);
  ROUTE("/go/up", goUp);
  ROUTE("/go/to/:...", goTo);
  ROUTE("/go/in/:subdir", goIn);
  ROUTE("/ls", ls);
  ROUTE("/history/back", historyBack);
  ROUTE("/history/forward", historyForward);
  ROUTE("/navstate", navstate);
  ROUTE("/dev/enabled", devEnabled);
  ROUTE("/dev/visible", devVisible);
  ROUTE("/dev/:show", devShow);

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

void Routes::goBack(const Request &req, Response &res) {
  auto n = req["count"];
  A8_DEBUG("/go/back/{}", n);

  if (!browser.can_go_back()) {
    res.status(http::HTTP_400_Bad_Request);
    return;
  }
  browser.go_back();
  //res.contentType("application/json") << json(browser.list());
}

void Routes::goForward(const Request &req, Response &res) {
  auto n = req["count"];
  A8_DEBUG("/go/forward/{}", n);

  if (!browser.can_go_forward()) {
    res.status(http::HTTP_400_Bad_Request);
    return;
  }
  browser.go_forward();
  //res.contentType("application/json") << json(browser.list());
}

void Routes::goUp(const Request &req, Response &res) {
  if (!browser.can_go_up()) {
    res.status(a8::http::HTTP_400_Bad_Request);
    return;
  }
  browser.go_up();
  //res.contentType("application/json") << json(browser.list());
}

void Routes::goTo(const Request &req, Response &res) {
  std::filesystem::path path{"/"};
  for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
    path /= *it;
  }
  if (browser.go_to(path)) {
    //res.contentType("application/json") << json(browser.list());
  } else {
    res.status(http::HTTP_404_Not_Found);
  }
}

void Routes::goIn(const Request &req, Response &res) {
  if (!browser.go_to(browser.curdir() / req[0])) {
    res.status(http::HTTP_404_Not_Found);
    return;
  }
  //res.contentType("application/json") << json(browser.list());
}

void Routes::ls(const Request &req, Response &res) {
#if 0
  auto list = browser.list();
  if (list.name.empty()) {
    res.status(http::HTTP_404_Not_Found);
    return;
  }
  res.contentType("application/json") << json(browser.list());
#endif
}

void Routes::historyBack(const Request &req, Response &res) {
  auto [it, end] = browser.get_back_range();
  json::array_t j;
  std::for_each(it, end, [&](auto const &dir) { j.emplace_back(dir); });
  res.contentType("application/json") << j;
}

void Routes::historyForward(const Request &req, Response &res) {
  auto [it, end] = browser.get_forward_range();
  json::array_t j;
  std::for_each(it, end, [&](auto const &dir) { j.emplace_back(dir); });
  res.contentType("application/json") << j;
}

void Routes::navstate(const Request &req, Response &res) {
  //res.contentType("application/json") << json(NavState{browser});
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
