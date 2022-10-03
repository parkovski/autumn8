#include "a8/window/window.h"
#include "a8/window/msgloop.h"
#include "a8/webview/webview.h"
#include "a8/util/router.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>
#include <vector>

using namespace a8;
using namespace a8::window;
using namespace a8::webview;

using nlohmann::json;

struct DirEntry {
  std::string name;
  bool isDir;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirEntry, name, isDir)
};

struct DirList {
  std::string name;
  std::vector<DirEntry> entries;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirList, name, entries)
};

class Browser {
  std::vector<std::filesystem::path> _stack;
  size_t _stackIndex;

public:
  using const_iterator = std::vector<std::filesystem::path>::const_iterator;
  using const_reverse_iterator
    = std::vector<std::filesystem::path>::const_reverse_iterator;

  explicit Browser()
    : _stack{std::filesystem::current_path()},
      _stackIndex{0}
  {}

  const std::filesystem::path &curdir() const noexcept {
    return _stack[_stackIndex];
  }

  std::pair<const_reverse_iterator, const_reverse_iterator>
  get_back_range() const noexcept {
    return std::make_pair(const_reverse_iterator{_stack.begin() + _stackIndex},
                          _stack.rend());
  }

  std::pair<const_iterator, const_iterator>
  get_forward_range() const noexcept {
    return std::make_pair(_stack.begin() + (_stackIndex + 1), _stack.end());
  }

  bool can_go_back(size_t count = 1) const noexcept {
    return _stackIndex + 1 > count;
  }

  bool can_go_forward(size_t count = 1) const noexcept {
    return _stackIndex + count < _stack.size();
  }

  bool go_back(size_t count = 1) noexcept {
    if (!can_go_back(count)) {
      return false;
    }
    _stackIndex -= count;
    return true;
  }

  bool go_forward(size_t count = 1) noexcept {
    if (!can_go_forward(count)) {
      return false;
    }
    _stackIndex += count;
    return true;
  }

  /// \param overwrite Replace the current history entry instead of appending.
  bool go_to(std::filesystem::path path, bool overwrite = false) {
    std::error_code noone_cares;
    if (!std::filesystem::is_directory(path, noone_cares)) {
      return false;
    }
    if (overwrite) {
      _stack[_stackIndex] = std::move(path);
      return true;
    }
    if (can_go_forward()) {
      // Pushing a new path overwrites the forward history.
      _stack.erase(_stack.cbegin() + _stackIndex + 1, _stack.cend());
    }
    _stack.emplace_back(std::move(path));
    ++_stackIndex;
    return true;
  }

  bool can_go_up() const noexcept {
    return curdir().has_parent_path();
  }

  bool go_up() {
    if (!can_go_up()) {
      return false;
    }
    return go_to(curdir().parent_path());
  }

  DirList list() const {
    DirList dir;
    std::error_code err;
    std::filesystem::directory_iterator di(curdir(), err);
    if (err) {
      return dir;
    }
    std::filesystem::directory_iterator end;
    dir.name = curdir().string();

    while (di != end) {
      bool isdir = di->is_directory(err);
      if (err) {
        continue;
      }
      dir.entries.emplace_back(DirEntry{di->path().filename().string(), isdir});

      di.increment(err);
    }
    return dir;
  }
};

struct NavState {
  bool canGoBack;
  bool canGoForward;
  bool canGoUp;

  explicit NavState(const Browser &browser) noexcept
    : canGoBack{browser.can_go_back()},
      canGoForward{browser.can_go_forward()},
      canGoUp{browser.can_go_up()}
  {}

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(NavState, canGoBack, canGoForward, canGoUp)
};

int a8main(int argc, char *argv[]) {
  auto win = new Window("Browser");
  win->resize(0, 0, 800, 600);
  win->quitOnClose();

  Browser browser;
  browser.go_to("/mnt/hdd1/music/wavetables/AKWF-FREE-master", true);

  auto wv = new WebView(win);
  wv->devToolsSetEnabled();
  Router<WebViewRouterRequest> router;
  wv->setUserCallback(std::ref(router));

  //router.notFound() = ...
  //router.ise() = ...

  router["/view/:name"] = [&browser](auto const &req, auto &res) {
    std::filesystem::path path = browser.curdir() / req[0];
    std::error_code noone_cares;
    if (!std::filesystem::is_regular_file(path, noone_cares)) {
      res.status(http::HTTP_404_Not_Found);
      return;
    }
    res.sendFile(path.string());
  };

  router["/go/back"] = [&browser](auto const &req, auto &res) {
    if (!browser.can_go_back()) {
      res.status(http::HTTP_400_Bad_Request);
      return;
    }
    browser.go_back();
    res.contentType("application/json") << json(browser.list());
  };

  // TODO: /go/back/:?count
  router["/go/back/:count"] = [&browser](auto const &req, auto &res) {
    //auto count = req.get<size_t>(0);
    //if (!browser.can_go_back(count)) {
      res.status(http::HTTP_400_Bad_Request);
      return;
    //}
    //browser.go_back(count);
    //res.contentType("application/json") << json(browser.list());
  };

  router["/go/forward"] = [&browser](auto const &req, auto &res) {
    if (!browser.can_go_forward()) {
      res.status(http::HTTP_400_Bad_Request);
      return;
    }
    browser.go_forward();
    res.contentType("application/json") << json(browser.list());
  };

  router["/go/forward/:count"] = [&browser](auto const &req, auto &res) {
    //auto count = req.get<size_t>(0);
    //if (!browser.can_go_forward(count)) {
      res.status(http::HTTP_400_Bad_Request);
      return;
    //}
    //browser.go_forward(count);
    //res.contentType("application/json") << json(browser.list());
  };

  router["/go/up"] = [&browser](auto const &req, auto &res) {
    if (!browser.can_go_up()) {
      res.status(a8::http::HTTP_400_Bad_Request);
      return;
    }
    browser.go_up();
    res.contentType("application/json") << json(browser.list());
  };

  router["/go/to/:..."] = [&browser](auto const &req, auto &res) {
    std::filesystem::path path{"/"};
    for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
      path /= *it;
    }
    if (browser.go_to(path)) {
      res.contentType("application/json") << json(browser.list());
    } else {
      res.status(http::HTTP_404_Not_Found);
    }
  };

  router["/go/in/:subdir"] = [&browser](auto const &req, auto &res) {
    if (!browser.go_to(browser.curdir() / req[0])) {
      res.status(http::HTTP_404_Not_Found);
      return;
    }
    res.contentType("application/json") << json(browser.list());
  };

  router["/ls"] = [&browser](auto const &req, auto &res) {
    auto list = browser.list();
    if (list.name.empty()) {
      res.status(http::HTTP_404_Not_Found);
      return;
    }
    res.contentType("application/json") << json(browser.list());
  };

  router["/history/back"] = [&browser](auto const &req, auto &res) {
    auto [it, end] = browser.get_back_range();
    json::array_t j;
    std::for_each(it, end, [&](auto const &dir) { j.emplace_back(dir); });
    res.contentType("application/json") << j;
  };

  router["/history/forward"] = [&browser](auto const &req, auto &res) {
    auto [it, end] = browser.get_forward_range();
    json::array_t j;
    std::for_each(it, end, [&](auto const &dir) { j.emplace_back(dir); });
    res.contentType("application/json") << j;
  };

  router["/navstate"] = [&browser](auto const &req, auto &res) {
    res.contentType("application/json") << json(NavState{browser});
  };

  router["/dev/enabled"] = [](auto const &req, auto &res) {
    res << req.webview()->devToolsEnabled();
  };

  router["/dev/visible"] = [](auto const &req, auto &res) {
    res << req.webview()->devToolsVisible();
  };

  router["/dev/:show"] = [](auto const &req, auto &res) {
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
      res.status(http::HTTP_200_OK) << (show ? "show" : "hide");
    } else {
      res.status(http::HTTP_403_Forbidden);
    }
  };

  wv->navigate("a8://res/index.html");

  win->show();
  using std::string_view_literals::operator""sv;
  if (argc > 1 && argv[1] == "-dev"sv) {
    wv->devToolsSetVisible(true);
  }
  return runMessageLoop();
}
