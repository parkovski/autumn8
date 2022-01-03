#include "a8/gfx/msgloop.h"
#include "a8/gfx/window.h"
#include "a8/gfx/views/webview.h"
#include "a8/util/router.h"

#include <fstream>
#include <filesystem>

using namespace a8;
using namespace a8::gfx;

class Browser {
  std::filesystem::path _cwd;

public:
  explicit Browser()
    : _cwd{std::filesystem::current_path()}
  {}

  bool go(std::string_view path) {
    std::error_code noone_cares;
    if (std::filesystem::is_directory(path, noone_cares)) {
      _cwd = path;
      return true;
    }
    return false;
  }

  bool goee_up() {
    if (!_cwd.has_parent_path()) {
      return false;
    }
    _cwd = _cwd.parent_path();
    return true;
  }

  bool goee_down(std::string_view dir) {
    auto nd = _cwd / dir;
    std::error_code noone_cares;
    if (std::filesystem::is_directory(nd, noone_cares)) {
      _cwd = nd;
      return true;
    }
    return false;
  }

  bool bee_wobby(WebViewResponse &req) {
    std::error_code err;
    std::filesystem::directory_iterator di(_cwd, err);
    if (err) {
      return false;
    }
    std::filesystem::directory_iterator end;

    req.write("P=").write(_cwd.string());
    if (_cwd.has_parent_path()) {
      req.write("\nD=..");
    }

    while (di != end) {
      auto isdir = di->is_directory(err);
      if (err) {
        continue;
      }
      req.write(isdir ? "\nD=" : "\nF=").write(di->path().filename().string());
      di.increment(err);
    }
    return true;
  }
};

int a8main(int argc, char *argv[]) {
  auto win = new Window("Browser");
  win->resize(0, 0, 500, 500);
  win->quitOnClose();

  Browser browser;

  auto wv = new WebView(win);
  Router<WebViewRouterRequest> router;
  wv->setUserCallback(std::ref(router));

  //router.notFound() = ...
  //router.ise() = ...

  router["/:filename"] = [](auto const &req, auto &res) {
    std::string filename{"browser/"};
    filename.append(req["filename"]);
    res.sendResFile(filename);
  };

  router["/up"] = [&browser](auto const &req, auto &res) {
    if (browser.goee_up()) {
      res.status(a8::http::HTTP_204_No_Content);
    } else {
      res.status(a8::http::HTTP_404_Not_Found);
    }
    res.end();
  };

  router["/cd/:..."] = [&browser](auto const &req, auto &res) {
    std::string path;
    for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
      path.append(1, '/').append(*it);
    }
    if (browser.go(path)) {
      res.status(http::HTTP_204_No_Content);
    } else {
      res.status(http::HTTP_404_Not_Found);
    }
  };

  router["/enter/:subdir"] = [&browser](auto const &req, auto &res) {
    if (browser.goee_down(req["subdir"])) {
      res.status(http::HTTP_204_No_Content);
    } else {
      res.status(http::HTTP_404_Not_Found);
    }
  };

  router["/ls"] = [&browser](auto const &req, auto &res) {
    if (!browser.bee_wobby(res)) {
      res.status(a8::http::HTTP_500_Internal_Server_Error);
      res.end("Something bad happened");
    }
  };

  wv->navigate("a8://app/index.html");

  win->show();
  return runMessageLoop();
}
