#ifndef A8_UTIL_INLINE_H_
#define A8_UTIL_INLINE_H_

#include <memory>
#include <string_view>
#include <type_traits>
#include <exception>

namespace a8 {

template<class F>
class ScopeExit {
  F _function;
  bool _active;

public:
  explicit ScopeExit(F function)
    noexcept(std::is_nothrow_move_constructible_v<F>)
    : _function{std::move(function)},
      _active{true}
  {}

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit &operator=(const ScopeExit &) = delete;

  ~ScopeExit() {
    if (_active) {
      _function();
    }
  }

  void release() noexcept {
    _active = false;
  }
};

template<class F>
class ScopeSuccess : public ScopeExit<F> {
public:
  using ScopeExit<F>::ScopeExit;

  ~ScopeSuccess() {
    if (std::current_exception()) {
      this->release();
    }
  }
};

template<class F>
class ScopeFail : public ScopeExit<F> {
public:
  using ScopeExit<F>::ScopeExit;

  ~ScopeFail() {
    if (!std::current_exception()) {
      this->release();
    }
  }
};

template<class F>
ScopeExit<F> scope_exit(F function)
             noexcept(std::is_nothrow_move_constructible_v<F>) {
  return ScopeExit<F>(std::move(function));
}

template<class F>
ScopeSuccess<F> scope_success(F function)
             noexcept(std::is_nothrow_move_constructible_v<F>) {
  return ScopeSuccess<F>(std::move(function));
}

template<class F>
ScopeFail<F> scope_fail(F function)
             noexcept(std::is_nothrow_move_constructible_v<F>) {
  return ScopeFail<F>(std::move(function));
}

namespace detail {
  struct ScopeExitHelper {};
  template<class F>
  ScopeExit<F> operator^(ScopeExitHelper, F function)
               noexcept(std::is_nothrow_move_constructible_v<F>) {
    return ScopeExit<F>(std::move(function));
  }

  template<class F>
  ScopeSuccess<F> operator&(ScopeExitHelper, F function)
                  noexcept(std::is_nothrow_move_constructible_v<F>) {
    return ScopeSuccess<F>(std::move(function));
  }

  template<class F>
  ScopeFail<F> operator|(ScopeExitHelper, F function)
               noexcept(std::is_nothrow_move_constructible_v<F>) {
    return ScopeFail<F>(std::move(function));
  }
}

#define A8_SCOPE_EXIT auto _scope_exit_##__LINE__ = ::a8::detail::ScopeExitHelper{} ^ [&]()
#define A8_SCOPE_SUCCESS auto _scope_exit_##__LINE__ = ::a8::detail::ScopeExitHelper{} & [&]()
#define A8_SCOPE_FAIL auto _scope_exit_##__LINE__ = ::a8::detail::ScopeExitHelper{} | [&]()

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
