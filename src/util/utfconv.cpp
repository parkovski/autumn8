#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "a8/util/utfconv.h"

#include <codecvt>

namespace a8 {

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void to_utf8(std::string &utf8str, std::wstring_view utf16str) {
  auto size = WideCharToMultiByte(CP_UTF8, 0, utf16str.data(),
                                  (int)utf16str.size(), nullptr, 0, nullptr,
                                  nullptr);
  if (!size) {
    utf8str.clear();
    return;
  }
  utf8str.resize(size);
  WideCharToMultiByte(CP_UTF8, 0, utf16str.data(), (int)utf16str.size(),
                      utf8str.data(), size, nullptr, nullptr);
}

void to_utf16(std::wstring &utf16str, std::string_view utf8str) {
  auto size = MultiByteToWideChar(CP_UTF8, 0, utf8str.data(),
                                  (int)utf8str.size(), nullptr, 0);
  if (!size) {
    utf16str.clear();
    return;
  }
  utf16str.resize(size);
  MultiByteToWideChar(CP_UTF8, 0, utf8str.data(), (int)utf8str.size(),
                      utf16str.data(), size);
}

std::string to_utf8(std::wstring_view utf16str) {
  std::string s;
  to_utf8(s, utf16str);
  return s;
}

std::wstring to_utf16(std::string_view utf8str) {
  std::wstring s;
  to_utf16(s, utf8str);
  return s;
}

#else // _WIN32

std::string to_utf8(std::u16string_view utf16str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
            .to_bytes(&utf16str.front(), &utf16str.back() + 1);
}

void to_utf8(std::string &utf8str, std::u16string_view utf16str) {
  // TODO: Make this write directly to utf8str.
  utf8str = to_utf8(utf16str);
}

std::u16string to_utf16(std::string_view utf8str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
            .from_bytes(&utf8str.front(), &utf8str.back() + 1);
}

void to_utf16(std::u16string &utf16str, std::string_view utf8str) {
  // TODO: Make this write directly to utf16str.
  utf16str = to_utf16(utf8str);
}

#endif // _WIN32

} // namespace a8
