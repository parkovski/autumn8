// Have to include this first or we get winsock redefinition errors.
#include <uwebsockets/App.h>
#include <uwebsockets/PerMessageDeflate.h>

#include "a8/a8.h"

#include <fmt/ostream.h>

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <sstream>

struct Options {
  explicit Options() {}

  bool ok() const {
    if (port < 1024) {
      return false;
    }

    if (baseDir.empty()) {
      return false;
    }
    if (!std::filesystem::exists(baseDir)) {
      fmt::print(stderr, "Directory {} does not exist\n", baseDir);
      return false;
    }
    if (!std::filesystem::is_directory(baseDir)) {
      fmt::print(stderr, "Path {} is not a directory\n", baseDir);
      return false;
    }

    return true;
  }

  bool parse(int argc, const char *const *argv) {
    for (int i = 1; i < argc; ++i) {
      if (argv[i][0] == '-' && argv[i][1] == 'p' && argv[i][2] == 0) {
        if (++i == argc) {
          return false;
        }
        char *endp;
        unsigned long ul = strtoul(argv[i], &endp, 10);
        if (endp == argv[i] || ul < 1024 || ul > UINT16_MAX) {
          fmt::print(stderr, "Bad port number {}\n", ul);
          return false;
        }
        port = uint16_t(ul);
      } else {
        if (!baseDir.empty()) {
          fmt::print(stderr, "Redefinition of base directory\n");
          return false;
        }
        baseDir = argv[i];
      }
    }

    return true;
  }

  uint16_t port = 8090;
  std::filesystem::path baseDir;
};

int a8main(int argc, char **argv) {
  Options opts;
  if (!opts.parse(argc, argv) || !opts.ok()) {
    fmt::print(stderr, "Usage: {} <dir>.\n", argv[0]);
    return 1;
  }

  auto &dir = opts.baseDir;
  uWS::App().get("/*", [&dir](auto *res, auto *req) {
    std::string_view url = req->getUrl().substr(1);
    std::filesystem::path path{dir};
    if (!url.empty()) {
      path /= url;
      if (!std::filesystem::is_directory(path)) {
        res->end("not a directory");
        return;
      }
    }
    std::stringstream ss;
    std::filesystem::directory_iterator di{path}, end;
    for (; di != end; ++di) {
      ss << di->path() << "\n";
    }
    res->end(ss.str());
  }).listen(opts.port, [](auto *) {
    fmt::print("ok\n");
  }).run();

  return 0;
}
