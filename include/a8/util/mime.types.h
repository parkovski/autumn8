#ifndef A8_UTIL_MIMETYPES_H_
#define A8_UTIL_MIMETYPES_H_

#include "cstring_view.h"
#include <unordered_map>

namespace a8 {

std::unordered_map<cstring_view, cstring_view>
getMimeTypeMap(bool names = true, bool exts = true);

} // namespace a8

#endif // A8_UTIL_MIMETYPES_H_
