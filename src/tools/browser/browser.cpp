#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"
#include "a8/util/router.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

using namespace a8;
using namespace a8::gfx;

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
  std::filesystem::path _cwd;

public:
  explicit Browser()
    : _cwd{std::filesystem::current_path()}
  {}

  std::string get_path(std::string_view name) {
    auto path = _cwd / name;
    if (std::filesystem::exists(path)) {
      return path.string();
    }
    return {};
  }

  bool go(std::string_view path) {
    std::error_code noone_cares;
    if (std::filesystem::is_directory(path, noone_cares)) {
      _cwd = path;
      return true;
    }
    return false;
  }

  bool go_up() {
    if (!_cwd.has_parent_path()) {
      return false;
    }
    _cwd = _cwd.parent_path();
    return true;
  }

  bool go_down(std::string_view dir) {
    auto nd = _cwd / dir;
    std::error_code noone_cares;
    if (std::filesystem::is_directory(nd, noone_cares)) {
      _cwd = nd;
      return true;
    }
    return false;
  }

  DirList list() const {
    DirList dir;
    std::error_code err;
    std::filesystem::directory_iterator di(_cwd, err);
    if (err) {
      return dir;
    }
    std::filesystem::directory_iterator end;
    dir.name = _cwd.string();

    if (_cwd.has_parent_path()) {
      dir.entries.emplace_back(DirEntry{"..", true});
    }

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

int a8main(int argc, char *argv[]) {
  auto win = new Window("Browser");
  win->resize(0, 0, 800, 600);
  win->quitOnClose();

  Browser browser;
  browser.go("/mnt/hdd1/music/wavetables/AKWF-FREE-master");

  auto wv = new WebView(win);
  wv->devToolsSetEnabled();
  Router<WebViewRouterRequest> router;
  wv->setUserCallback(std::ref(router));

  //router.notFound() = ...
  //router.ise() = ...

  // Temporary replacement for res routes.
  router["/:filename"] = [](auto const &req, auto &res) {
    std::string filename{"browser/"};
    filename.append(req["filename"]);
    res.sendResFile(filename);
  };

  router["/view/:name"] = [&browser](auto const &req, auto &res) {
    std::string filename = browser.get_path(req[0]);
    if (filename.empty()) {
      res.status(http::HTTP_404_Not_Found);
    } else {
      res.sendFile(filename);
    }
  };

  router["/up"] = [&browser](auto const &req, auto &res) {
    if (browser.go_up()) {
      res.contentType("application/json") << json(browser.list());
    } else {
      res.status(a8::http::HTTP_404_Not_Found);
    }
  };

  router["/cd/:..."] = [&browser](auto const &req, auto &res) {
    std::string path;
    for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
      path.append(1, '/').append(*it);
    }
    if (browser.go(path)) {
      res.contentType("application/json") << json(browser.list());
    } else {
      res.status(http::HTTP_404_Not_Found);
    }
  };

  router["/enter/:subdir"] = [&browser](auto const &req, auto &res) {
    if (browser.go_down(req["subdir"])) {
      res.contentType("application/json") << json(browser.list());
    } else {
      res.status(http::HTTP_404_Not_Found);
    }
  };

  router["/ls"] = [&browser](auto const &req, auto &res) {
    auto list = browser.list();
    if (list.name.empty()) {
      res.status(http::HTTP_404_Not_Found);
    } else {
      res.contentType("application/json") << json(browser.list());
    }
  };

  router["/dev/enabled"] = [](auto const &req, auto &res) {
    res << req.webview()->devToolsEnabled();
  };

  router["/dev/visible"] = [](auto const &req, auto &res) {
    res << req.webview()->devToolsVisible();
    res.finish();
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

  wv->navigate("a8://app/index.html");

  win->show();
  using std::string_view_literals::operator""sv;
  if (argc > 1 && argv[1] == "-dev"sv) {
    wv->devToolsSetVisible(true);
  }
  return runMessageLoop();
}
