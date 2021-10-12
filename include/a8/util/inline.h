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

template<class Char, class String>
bool starts_with(std::basic_string_view<Char> str, String &&start_) noexcept {
  std::basic_string_view<Char> start{std::forward<String>(start_)};
  if (str.length() < start.length()) {
    return false;
  }
  return str.substr(0, start.length()) == start;
}

template<class Char, class String>
bool starts_with(Char const *str, String &&start_) noexcept {
  std::basic_string_view<Char> start{std::forward<String>(start_)};
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

template<class Char, class String>
bool starts_with(const std::basic_string<Char> &str, String &&start) noexcept {
  return starts_with<Char>(std::basic_string_view<Char>{str},
                           std::forward<String>(start));
}

template<class Char, class String>
bool ends_with(std::basic_string_view<Char> str, String &&end_) noexcept {
  std::basic_string_view<Char> end{std::forward<String>(end_)};
  if (str.length() < end.length()) {
    return false;
  }
  return str.substr(str.length() - end.length()) == end;
}

template<class Char, class String>
bool ends_with(const std::basic_string<Char> &str, String &&end) noexcept {
  return ends_with<Char>(std::basic_string_view<Char>{str},
                         std::forward<String>(end));
}

template<class Char, class String>
bool ends_with(const Char *str, String &&end) noexcept {
  return ends_with<Char>(std::basic_string_view<Char>{str},
                         std::forward<String>(end));
}

} // namespace a8

#endif // A8_UTIL_INLINE_H_
