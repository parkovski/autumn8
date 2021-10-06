#ifndef A8_UTIL_INLINE_H_
#define A8_UTIL_INLINE_H_

#include <memory>
#include <string_view>

namespace a8 {

// Returns a null unique_ptr on allocation failure.
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique_nothrow(Args &&...args) noexcept {
  return std::unique_ptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
}

template<class Char>
bool starts_with(Char const *str, std::basic_string_view<Char> start)
                noexcept {
  auto start_iter = start.begin();
  auto start_end = start.end();
  while (true) {
    if (start_iter == start_end) {
      return true;
    }
    if (!*str) {
      return false;
    }
    if (*str != *start_iter) {
      return false;
    }
    ++str;
    ++start_iter;
  }
}

template<class Char>
bool starts_with(std::basic_string_view<Char> str,
                 std::basic_string_view<Char> start) noexcept {
  if (str.length() < start.length()) {
    return false;
  }
  return str.substr(0, start.length()) == start;
}

template<class Char>
bool ends_with(std::basic_string_view<Char> str,
               std::basic_string_view<Char> end) noexcept {
  if (str.length() < end.length()) {
    return false;
  }
  return str.substr(str.length() - end.length()) == end;
}

} // namespace a8

#endif // A8_UTIL_INLINE_H_
