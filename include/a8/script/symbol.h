#ifndef A8_SCRIPT_SYMBOL_H_
#define A8_SCRIPT_SYMBOL_H_

namespace a8::script {

union Data {
  int64_t i;
  double f;
  void *p;
  const void *cp;

  template<typename T>
  T *ptr()
  { return *reinterpret_cast<T **>(&p); }

  template<typename T>
  T *ptr(T *newdata)
  { return *reinterpret_cast<T **>(&p) = newdata;}

  template<typename T>
  const T *const_ptr() const
  { return *reinterpret_cast<const T *const *>(&cp); }

  template<typename T>
  const T *const_ptr(const T *newdata)
  { return reinterpret_cast<const T *>(&cp); }
};

struct Symbol {
  std::string_view name;
  Symbol *type;
  Data data;

  Symbol() = default;
  Symbol(const Symbol &) = delete;
  Symbol &operator=(const Symbol &) = delete;
  Symbol(Symbol &&) = default;
  Symbol &operator=(Symbol &&) = default;

  explicit Symbol(Symbol *type, std::string_view name) noexcept;

  ~Symbol();
};

} // namespace a8::script

#endif /* A8_SCRIPT_SYMBOL_H_ */
