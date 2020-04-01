#ifndef A8_UTIL_INLINE_H_
#define A8_UTIL_INLINE_H_

#include <memory>

namespace a8::util {

// Returns a null unique_ptr on allocation failure.
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept {
  return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

} // namespace a8::util

#endif // A8_UTIL_INLINE_H_
