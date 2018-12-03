#ifndef _a8_util_utfconv_h_
#define _a8_util_utfconv_h_

#include <string>
#include <string_view>

namespace a8::util::utfconv {

std::string to_utf8(std::wstring_view utf16str);
std::wstring to_utf16(std::string_view utf8str);

} // namespace a8::util::utfconv

#endif // _a8_util_utfconv_h_
