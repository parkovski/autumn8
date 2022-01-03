#include <catch2/catch_test_macros.hpp>

#include "a8/util/router.h"
#include <map>

using namespace a8;

struct Request : RouterBase::RouteParams {
  std::string_view path;
  explicit Request(std::string_view path)
    : path{path}
  {}
};

struct RouterTraits {
  using callback_type = std::function<void(const Request &)>;

  static void call(const callback_type &callback, Request &req, Request &) {
    callback(req);
  }

  static Request &get_request(Request &req) { return req; }
  static std::string_view get_route_path(Request &req) { return req.path; }
  static RouterBase::RouteParams &get_route_params(Request &req) { return req; }
};

TEST_CASE("Router", "[router]") {
  Router<Request, ::RouterTraits> r;
  std::string s;
  auto add = [&](std::string_view route) {
    r[route] = [&s, route](auto const &req) {
      s = route;
      if (req.named_size()) {
        // Put params in an ordered map for reliable test output.
        std::map<std::string_view, std::string_view> map{req.named_begin(),
                                                         req.named_end()};
        for (auto it = map.begin(); it != map.end(); ++it) {
          s.append("; ").append(it->first).append(":").append(it->second);
        }
      }
      if (req.unnamed_size()) {
        for (auto it = req.unnamed_begin(); it != req.unnamed_end(); ++it) {
          s.append("; ").append(*it);
        }
      }
    };
  };
  auto call = [&](std::string_view path) -> std::string_view {
    Request req{path};
    if (!r(req)) {
      return {};
    }
    return s;
  };

  add("/");
  add("/foo");
  add("/foo/bar");
  add("/foo/:bar");
  add("/foo/bar/bar");
  add("/foo/:p0/bar");
  add("/foo/bar/:p1/:p2");
  add("/foo/:p1/bar/:p2");
  add("/:p1/bar/:p2/bar");
  add("/:p0/bar/bar/foo");
  add("/bar/bar/bar/xyz");
  add("/all/:...");
  add("/:p0/all/:...");

  REQUIRE(call("/")                == "/");
  REQUIRE(call("/foo")             == "/foo");
  REQUIRE(call("/foo/bar")         == "/foo/bar");
  REQUIRE(call("/foo/foo")         == "/foo/:bar; bar:foo");
  REQUIRE(call("/foo/bar/bar")     == "/foo/bar/bar");
  REQUIRE(call("/foo/foo/bar")     == "/foo/:p0/bar; p0:foo");
  REQUIRE(call("/foo/bar/bar/foo") == "/foo/bar/:p1/:p2; p1:bar; p2:foo");
  REQUIRE(call("/foo/bar/foo/bar") == "/foo/bar/:p1/:p2; p1:foo; p2:bar");
  REQUIRE(call("/foo/foo/bar/bar") == "/foo/:p1/bar/:p2; p1:foo; p2:bar");
  REQUIRE(call("/bar/bar/bar/bar") == "/:p1/bar/:p2/bar; p1:bar; p2:bar");
  REQUIRE(call("/bar/bar/foo/bar") == "/:p1/bar/:p2/bar; p1:bar; p2:foo");
  REQUIRE(call("/bar/bar/bar/foo") == "/:p0/bar/bar/foo; p0:bar");
  REQUIRE(call("/bar/bar/foo/foo").empty());
  REQUIRE(call("/bar/bar/bar/abc").empty());
  REQUIRE(call("/all")             == "/all/:...");
  REQUIRE(call("/all/foo/bar")     == "/all/:...; foo; bar");
  REQUIRE(call("/abc/all/foo/bar") == "/:p0/all/:...; p0:abc; foo; bar");
}
