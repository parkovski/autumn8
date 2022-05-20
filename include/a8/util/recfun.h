#ifndef A8_UTIL_RECFUN_H_
#define A8_UTIL_RECFUN_H_

#include <utility>
#include <type_traits>

namespace a8 {

template<typename ...TArgs>
struct recfun {
  using function = recfun (*)(TArgs...);
  using return_type = function;

  constexpr recfun() noexcept : f(nullptr) {}
  constexpr recfun(std::nullptr_t) noexcept : f(nullptr) {}
  constexpr recfun(function f) noexcept : f(f) {}
  constexpr recfun(const recfun &other) noexcept : f(other.f) {}
  constexpr recfun &operator=(const recfun &other) noexcept {
    f = other.f;
  }

  constexpr bool operator==(const recfun &other) const noexcept {
    return f == other.f;
  }

  constexpr bool operator!=(const recfun &other) const noexcept {
    return f != other.f;
  }

  constexpr recfun operator()(TArgs &&...args) const
      noexcept(std::is_nothrow_invocable_v<function, TArgs...>) {
    return f(std::forward<TArgs>(args)...);
  }

  constexpr explicit operator bool() const noexcept {
    return f != nullptr;
  }

private:
  function f;
};

template<typename T, typename ...TArgs>
struct memrecfun {
  using function = memrecfun (T::*)(TArgs...);
  using return_type = function;

  constexpr memrecfun() noexcept : f(nullptr) {}
  constexpr memrecfun(std::nullptr_t) noexcept : f(nullptr) {}
  constexpr memrecfun(function f) noexcept : f(f) {}
  constexpr memrecfun(const memrecfun &other) noexcept : f(other.f) {}
  constexpr memrecfun &operator=(const memrecfun &other) noexcept {
    f = other.f;
    return *this;
  }

  constexpr bool operator==(const memrecfun &other) const noexcept {
    return f == other.f;
  }

  constexpr bool operator!=(const memrecfun &other) const noexcept {
    return f != other.f;
  }

  constexpr memrecfun operator()(const T &self, TArgs &&...args) const
      noexcept(std::is_nothrow_invocable_v<function, const T &, TArgs...>) {
    return self.*f(std::forward<TArgs>(args)...);
  }

  constexpr memrecfun operator()(T &self, TArgs &&...args) const
      noexcept(std::is_nothrow_invocable_v<function, T &, TArgs...>) {
    return self.*f(std::forward<TArgs>(args)...);
  }

  constexpr explicit operator bool() const noexcept {
    return f != nullptr;
  }

private:
  function f;
};

} // namespace a8

#endif /* A8_UTIL_RECFUN_H_ */
