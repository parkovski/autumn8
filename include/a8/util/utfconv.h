#ifndef A8_UTIL_UTFCONV_H_
#define A8_UTIL_UTFCONV_H_

#include <string>
#include <string_view>

namespace a8 {

#ifdef _WIN32
// Converts and appends utf16str to utf8str.
// Returns the number of bytes appended.
size_t to_utf8(std::string &utf8str, std::wstring_view utf16str);
// Converts and appends utf8str to utf16str.
// Returns the number of wchar_t's appended.
size_t to_utf16(std::wstring &utf16str, std::string_view utf8str);
// Returns utf16str converted to UTF-8.
std::string to_utf8(std::wstring_view utf16str);
// Returns utf8str converted to UTF-16LE.
std::wstring to_utf16(std::string_view utf8str);
#else
// Converts and appends utf16str to utf8str.
// Returns the number of bytes appended.
size_t to_utf8(std::string &utf8str, std::u16string_view utf16str);
// Converts and appends utf8str to utf16str.
// Returns the number of char16_t's appended.
size_t to_utf16(std::u16string &utf16str, std::string_view utf8str);
// Returns utf16str converted to UTF-8.
std::string to_utf8(std::u16string_view utf16str);
// Returns utf8str converted to UTF-16LE.
std::u16string to_utf16(std::string_view utf8str);
#endif

} // namespace a8

#endif /* A8_UTIL_UTFCONV_H_ */
