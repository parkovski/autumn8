#include <catch2/catch_test_macros.hpp>

#include "a8/util/router.h"
#include <map>

using namespace a8;

TEST_CASE("Router", "[router]") {
  Router r;
  std::string s;
  auto add = [&](std::string_view route) {
    r[route] = [&s, route](auto const &params) {
      s = route;
      if (!params.empty()) {
        // Put params in an ordered map for reliable test output.
        std::map<std::string_view, std::string_view> map{params.begin(),
                                                         params.end()};
        for (auto it = map.begin(); it != map.end(); ++it) {
          s.append("; ").append(it->first).append(":").append(it->second);
        }
      }
    };
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

  REQUIRE(r("/"));
  REQUIRE(s == "/");

  REQUIRE(r("/foo"));
  REQUIRE(s == "/foo");

  REQUIRE(r("/foo/bar"));
  REQUIRE(s == "/foo/bar");

  REQUIRE(r("/foo/foo"));
  REQUIRE(s == "/foo/:bar; bar:foo");

  REQUIRE(r("/foo/bar/bar"));
  REQUIRE(s == "/foo/bar/bar");

  REQUIRE(r("/foo/foo/bar"));
  REQUIRE(s == "/foo/:p0/bar; p0:foo");

  REQUIRE(r("/foo/bar/bar/foo"));
  REQUIRE(s == "/foo/bar/:p1/:p2; p1:bar; p2:foo");

  REQUIRE(r("/foo/bar/foo/bar"));
  REQUIRE(s == "/foo/bar/:p1/:p2; p1:foo; p2:bar");

  REQUIRE(r("/foo/foo/bar/bar"));
  REQUIRE(s == "/foo/:p1/bar/:p2; p1:foo; p2:bar");

  REQUIRE(r("/bar/bar/bar/bar"));
  REQUIRE(s == "/:p1/bar/:p2/bar; p1:bar; p2:bar");

  REQUIRE(r("/bar/bar/foo/bar"));
  REQUIRE(s == "/:p1/bar/:p2/bar; p1:bar; p2:foo");

  REQUIRE(r("/bar/bar/bar/foo"));
  REQUIRE(s == "/:p0/bar/bar/foo; p0:bar");

  REQUIRE(!r("/bar/bar/foo/foo"));

  REQUIRE(!r("/bar/bar/bar/abc"));
}
