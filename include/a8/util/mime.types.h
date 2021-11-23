#ifndef A8_UTIL_MIMETYPES_H_
#define A8_UTIL_MIMETYPES_H_

#include <unordered_map>

namespace a8 {

std::unordered_map<const char *, const char *>
getMimeTypeMap(bool names = true, bool exts = true);

} // namespace a8

#endif // A8_UTIL_MIMETYPES_H_
