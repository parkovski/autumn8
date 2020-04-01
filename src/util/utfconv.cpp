#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "a8/util/utfconv.h"

#include <codecvt>

namespace a8::util::utfconv {

std::string to_utf8(std::wstring_view utf16str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .to_bytes(&utf16str.front(), &utf16str.back() + 1);
}

void to_utf8(std::wstring_view utf16str, std::string &utf8str) {
  // TODO: Make this write directly to utf8str.
  utf8str = to_utf8(utf16str);
}

std::wstring to_utf16(std::string_view utf8str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .from_bytes(&utf8str.front(), &utf8str.back() + 1);
}

void to_utf16(std::string_view utf8str, std::wstring &utf16str) {
  // TODO: Make this write directly to utf16str.
  utf16str = to_utf16(utf8str);
}

} // namespace a8::util::utfconv

