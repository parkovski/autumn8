#ifndef A8_UTIL_UTFCONV_H_
#define A8_UTIL_UTFCONV_H_

#include <string>
#include <string_view>

namespace a8 {

#ifdef _WIN32
std::string to_utf8(std::wstring_view utf16str);
void to_utf8(std::string &utf8str, std::wstring_view utf16str);
std::wstring to_utf16(std::string_view utf8str);
void to_utf16(std::wstring &utf16str, std::string_view utf8str);
#else
std::string to_utf8(std::u16string_view utf16str);
void to_utf8(std::string &utf8str, std::u16string_view utf16str);
std::u16string to_utf16(std::string_view utf8str);
void to_utf16(std::u16string &utf16str, std::string_view utf8str);
#endif

} // namespace a8

#endif /* A8_UTIL_UTFCONV_H_ */
