#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "a8/util/utfconv.h"

#include <codecvt>
#include <locale>

namespace a8 {

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

size_t to_utf8(std::string &utf8str, std::wstring_view utf16str) {
  auto size = WideCharToMultiByte(CP_UTF8, 0, utf16str.data(),
                                  (int)utf16str.size(), nullptr, 0, nullptr,
                                  nullptr);
  if (!size) {
    return 0;
  }
  utf8str.resize(utf8str.size() + size);
  WideCharToMultiByte(CP_UTF8, 0, utf16str.data(), (int)utf16str.size(),
                      &utf8str.back() + 1 - size, size, nullptr, nullptr);
  return size;
}

size_t to_utf16(std::wstring &utf16str, std::string_view utf8str) {
  auto size = MultiByteToWideChar(CP_UTF8, 0, utf8str.data(),
                                  (int)utf8str.size(), nullptr, 0);
  if (!size) {
    return 0;
  }
  utf16str.resize(utf16str.size() + size);
  MultiByteToWideChar(CP_UTF8, 0, utf8str.data(), (int)utf8str.size(),
                      &utf16str.back() + 1 - size, size);
  return size;
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

size_t to_utf8(std::string &utf8str, std::u16string_view utf16str) {
  // TODO: Make this write directly to utf8str.
  auto initsize = utf8str.size();
  utf8str += to_utf8(utf16str);
  return utf8str.size() - initsize;
}

std::u16string to_utf16(std::string_view utf8str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
            .from_bytes(&utf8str.front(), &utf8str.back() + 1);
}

size_t to_utf16(std::u16string &utf16str, std::string_view utf8str) {
  // TODO: Make this write directly to utf16str.
  auto initsize = utf16str.size();
  utf16str = to_utf16(utf8str);
  return utf16str.size() - initsize;
}

#endif // _WIN32

} // namespace a8
