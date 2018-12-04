#ifndef A8_UTIL_UTFCONV_H_
#define A8_UTIL_UTFCONV_H_

#include <string>
#include <string_view>

namespace a8::util::utfconv {

std::string to_utf8(std::wstring_view utf16str);
std::wstring to_utf16(std::string_view utf8str);

} // namespace a8::util::utfconv

#endif /* A8_UTIL_UTFCONV_H_ */

