#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "a8/util/utfconv.h"

#include <codecvt>

namespace a8::util::utfconv {

std::string to_utf8(std::wstring_view utf16str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .to_bytes(&utf16str.front(), &utf16str.back() + 1);
}

std::wstring to_utf16(std::string_view utf8str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .from_bytes(&utf8str.front(), &utf8str.back() + 1);
}

} // namespace a8::util::utfconv

