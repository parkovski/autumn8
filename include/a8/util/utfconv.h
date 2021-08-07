#ifndef A8_UTIL_UTFCONV_H_
#define A8_UTIL_UTFCONV_H_

#include <string>
#include <string_view>

namespace a8 {

std::string to_utf8(std::wstring_view utf16str);
void to_utf8(std::wstring_view utf16str, std::string &utf8str);
std::wstring to_utf16(std::string_view utf8str);
void to_utf16(std::string_view utf8str, std::wstring &utf16str);

} // namespace a8

#endif /* A8_UTIL_UTFCONV_H_ */
