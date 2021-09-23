#include "a8/util/utfconv.h"
#include <catch2/catch_all.hpp>

// If these tests fail on Windows, the compiler probably didn't receive the
// /utf-8 option.

#ifdef _WIN32
const char *const    hello_en_s =  "Hello world, it's good to see you 😃!";
const wchar_t *const hello_en_w = L"Hello world, it's good to see you 😃!";
const char *const    hello_ru_s =  "Привет мир, я очень рад тебя видеть 😃!";
const wchar_t *const hello_ru_w = L"Привет мир, я очень рад тебя видеть 😃!";

TEST_CASE("UTF8 to UTF16 conversion", "[utfconv]") {
  SetConsoleOutputCP(CP_UTF8);
  std::wstring w;
  a8::to_utf16(w, hello_en_s);
  REQUIRE(w == hello_en_w);
  a8::to_utf16(w, hello_ru_s);
  REQUIRE(w == hello_ru_w);
}

TEST_CASE("UTF16 to UTF8 conversion", "[utfconv]") {
  SetConsoleOutputCP(CP_UTF8);
  std::string s;
  a8::to_utf8(s, hello_en_w);
  REQUIRE(s == hello_en_s);
  a8::to_utf8(s, hello_ru_w);
  REQUIRE(s == hello_ru_s);
}
#endif
