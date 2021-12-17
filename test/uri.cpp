#include "a8/util/uri.h"
#include <catch2/catch_test_macros.hpp>
#include <sstream>

namespace {
  const char *const g_uris[] = {
    "mailto:me@me.com",
    "abc",
    "abc/def",
    "/abc",
    "//",
    "///",
    "./",
    "./foo",
    "foo:bar",
    "foo:/bar",
    "foo:bar/baz",
    "foo:/bar/baz/",
    "file:///etc/passwd",
    "https://me@www.foobar.com",
    "https://localhost:8080/?a=b&c=d#fragment",
    "a?b",
    "?query_only",
    "#fragment_only",
  };
  const size_t g_uris_size = sizeof(g_uris) / sizeof(g_uris[0]);

  const char *const g_paths[g_uris_size] = {
    "me@me.com",
    "abc",
    "abc/def",
    "/abc",
    nullptr,
    "/",
    "./",
    "./foo",
    "bar",
    "/bar",
    "bar/baz",
    "/bar/baz/",
    "/etc/passwd",
    nullptr,
    "/",
    "a",
    nullptr,
    nullptr,
  };

  const char *const g_paths_by_component[g_uris_size] = {
    "1:me@me.com",
    "1:abc",
    "2:abc:def",
    "1:abc",
    "0",
    "1:",
    "2:.:",
    "2:.:foo",
    "1:bar",
    "1:bar",
    "2:bar:baz",
    "3:bar:baz:",
    "2:etc:passwd",
    "0",
    "1:",
    "1:a",
    "0",
    "0",
  };
}

TEST_CASE("URI path check", "[uri]") {
  for (size_t i = 0; i < g_uris_size; ++i) {
    a8::Uri uri{g_uris[i]};
    REQUIRE(static_cast<bool>(uri));
    if (g_paths[i]) {
      REQUIRE(uri.path().str() == g_paths[i]);
    } else {
      REQUIRE(uri.path().str().empty());
    }
  }
}

TEST_CASE("URI path component sum", "[uri]") {
  for (size_t i = 0; i < g_uris_size; ++i) {
    a8::Uri uri{g_uris[i]};
    auto path = uri.path();
    auto it = path.cbegin();
    auto end = path.cend();
    int size = 0;
    while (it != end) {
      ++size;
      ++it;
    }
    std::stringstream s;
    s << size;
    it = path.cbegin();
    while (it != end) {
      s << ":" << *it++;
    }
    REQUIRE(s.str() == g_paths_by_component[i]);
  }
}
