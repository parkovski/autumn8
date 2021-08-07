#ifndef A8_UTIL_RECFUN_H_
#define A8_UTIL_RECFUN_H_

#include <utility>

namespace a8 {

template<typename ...TArgs>
struct recfun {
  using function = recfun (*)(TArgs...);
  using return_type = function;

  recfun() : f(nullptr) {}
  recfun(function f) : f(f) {}
  recfun(const recfun &other) : f(other.f) {}
  recfun &operator=(const recfun &other) {
    f = other.f;
  }

  bool operator==(const recfun &other) const {
    return f == other.f;
  }

  bool operator!=(const recfun &other) const {
    return f != other.f;
  }

  recfun operator()(TArgs &&...args) const {
    return f(std::forward<TArgs>(args)...);
  }

private:
  function f;
};

template<typename T, typename ...TArgs>
struct memrecfun {
  using function = memrecfun (typename T::*)(TArgs...);
  using return_type = function;

  memrecfun() : f(nullptr) {}
  memrecfun(function f) : f(f) {}
  memrecfun(const memrecfun &other) : f(other.f) {}
  memrecfun &operator=(const memrecfun &other) {
    f = other.f;
    return *this;
  }

  bool operator==(const memrecfun &other) const {
    return f == other.f;
  }

  bool operator==(const recfun &other) const {
    return f != other.f;
  }

  memrecfun operator()(T &self, TArgs &&...args) const {
    return self.*f(std::forward<TArgs>(args)...);
  }

private:
  function f;
};

} // namespace a8

#endif /* A8_UTIL_RECFUN_H_ */
