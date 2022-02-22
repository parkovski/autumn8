#ifndef A8_VM_INTERNAL_SYMBOL_H_
#define A8_VM_INTERNAL_SYMBOL_H_

#include "../symbol.h"

/**
 * Say foo is a meta object that implements bar and baz:
 * symbol
 * |_ meta_entry
 *    |_ meta_object
 *       |_ foo
 *          |_ bar
 *          |_ baz
 *
 * Ideally we'd want something like this:
 * symbol = {meta}
 * symbol<T> = symbol + T
 * meta_entry_self = {self}
 * meta_entry = symbol<meta_entry_self>
 * meta_entry_self<T> = meta_entry_self + T
 * meta_entry<T> = symbol<meta_entry_self<T>>
 *               & meta_entry + T
 * meta_object_self = {...}
 * meta_object = meta_entry<meta_object_self>
 * meta_object_self<T> = meta_object_self + T
 * meta_object<T> = symbol<meta_entry_self<meta_object_self<T>>>
 *                & meta_object + T
 *
 * However there are strange rules around unions in C++ that I'd rather not
 * mess with.
 */

template<class T>
struct a8_vm_symbol_t : a8_vm_symbol {
  template<class... Args>
  constexpr explicit
  a8_vm_symbol_t(a8_vm_symid meta, Args &&...args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    : a8_vm_symbol{meta},
      value{std::forward<Args>(args)...}
  {}

  const T *get() const noexcept { return &value; }
  T *get() noexcept { return &value; }

  const T &operator*() const noexcept { return value; }
  T &operator*() noexcept { return value; }

  const T *operator->() const noexcept { return get(); }
  T *operator->() noexcept { return get(); }

private:
  T value;
};

template<class T>
struct a8_vm_meta_entry_t : a8_vm_meta_entry_s {
  constexpr explicit a8_vm_meta_entry_t(a8_vm_symid meta) noexcept
    : a8_vm_meta_entry_s{{meta}, a8_vm_symid_undef}
  {}

  static const a8_vm_meta_entry_t &
  from(const a8_vm_meta_entry &entry) noexcept {
    return *static_cast<const a8_vm_meta_entry_t *>(
      reinterpret_cast<const a8_vm_meta_entry_s *>(
        reinterpret_cast<const char *>(&entry) - sizeof(a8_vm_symbol)
      )
    );
  }

  static a8_vm_meta_entry_t &
  from(a8_vm_meta_entry &entry) noexcept {
    return *static_cast<a8_vm_meta_entry_t *>(
      reinterpret_cast<a8_vm_meta_entry_s *>(
        reinterpret_cast<char *>(&entry) - sizeof(a8_vm_symbol)
      )
    );
  }

  const T *get() const noexcept { return &value; }
  T *get() noexcept { return &value; }

  const T &operator*() const noexcept { return value; }
  T &operator*() noexcept { return value; }

  const T *operator->() const noexcept { return get(); }
  T *operator->() noexcept { return get(); }

  operator const a8_vm_symbol_t<a8_vm_meta_entry> &() const noexcept {
    return static_cast<const a8_vm_symbol_t<a8_vm_meta_entry> &>(this->base);
  }

  operator a8_vm_symbol_t<a8_vm_meta_entry> &() noexcept {
    return static_cast<a8_vm_symbol_t<a8_vm_meta_entry> &>(this->base);
  }

private:
  T value;
};

template<class... Ts>
struct a8_vm_meta_object_t;

template<>
struct a8_vm_meta_object_t<> : a8_vm_meta_object_s {
  constexpr explicit a8_vm_meta_object_t() noexcept
    : a8_vm_meta_object_s{a8_vm_symid_meta_object}
  {}

  static const a8_vm_meta_object_t &
  from(const a8_vm_meta_object &obj) noexcept {
    return *static_cast<const a8_vm_meta_object_t *>(
      reinterpret_cast<const a8_vm_meta_object_s *>(
        reinterpret_cast<const char *>(&obj) - sizeof(a8_vm_meta_entry_s)
      )
    );
  }

  static a8_vm_meta_object_t &
  from(a8_vm_meta_object &obj) noexcept {
    return *static_cast<a8_vm_meta_object_t *>(
      reinterpret_cast<a8_vm_meta_object_s *>(
        reinterpret_cast<char *>(&obj) - sizeof(a8_vm_meta_entry_s)
      )
    );
  }

  const a8_vm_meta_entry_s *static_entry(a8_vm_usize n) const noexcept {
    return &a8_vm_meta_object_static_entry(&this->self, n);
  }

  a8_vm_meta_entry_s *static_entry(a8_vm_usize n) noexcept {
    return &a8_vm_meta_object_static_entry(&this->self, n);
  }

  const a8_vm_meta_entry_s *dynamic_entry(a8_vm_usize n) const noexcept {
    return &a8_vm_meta_object_dynamic_entry(&this->self, n);
  }

  a8_vm_meta_entry_s *dynamic_entry(a8_vm_usize n) noexcept {
    return &a8_vm_meta_object_dynamic_entry(&this->self, n);
  }

  operator const a8_vm_symbol_t<a8_vm_meta_object_m> &() const noexcept {
    return static_cast<const a8_vm_symbol_t<a8_vm_meta_object_m> &>(
      this->base.base
    );
  }

  operator a8_vm_symbol_t<a8_vm_meta_object_m> &() noexcept {
    return static_cast<a8_vm_symbol_t<a8_vm_meta_object_m> &>(this->base.base);
  }

  operator const a8_vm_meta_entry_t<a8_vm_meta_object> &() const noexcept {
    return static_cast<const a8_vm_meta_entry_t<a8_vm_meta_object> &>(
      this->base
    );
  }

  operator a8_vm_meta_entry_t<a8_vm_meta_object> &() noexcept {
    return static_cast<a8_vm_meta_entry_t<a8_vm_meta_object> &>(this->base);
  }
};

template<class... Ts>
struct a8_vm_meta_object_t : a8_vm_meta_object_t<> {
  a8_vm_meta_object_t() = delete;
  /*
  std::integer_sequence?
  template<class... Args>
  constexpr explicit
  a8_vm_meta_object_t(std::array<sizeof...(Ts), a8_vm_symid> ids,
                      Args &&...args)
    noexcept(std::is_nothrow_constructible_v<
              std::tuple<a8_vm_meta_entry_t<Ts>...>,
              Args...
             >)
    : a8_vm_meta_object_t<>(meta),
      entries(std::forward<Args>(args)...)
  {}
  */

  static const a8_vm_meta_object_t &
  from(const a8_vm_meta_object &obj) noexcept {
    return *static_cast<const a8_vm_meta_object_t *>(
      reinterpret_cast<const a8_vm_meta_object_s *>(
        reinterpret_cast<const char *>(&obj) - sizeof(a8_vm_meta_entry_s)
      )
    );
  }

  static a8_vm_meta_object_t &
  from(a8_vm_meta_object &obj) noexcept {
    return *static_cast<a8_vm_meta_object_t *>(
      reinterpret_cast<a8_vm_meta_object_s *>(
        reinterpret_cast<char *>(&obj) - sizeof(a8_vm_meta_entry_s)
      )
    );
  }

  template<size_t I>
  auto const *static_entry() const noexcept {
    return &std::get<I>(this->entries);
  }

  template<size_t I>
  auto *static_entry() noexcept {
    return &std::get<I>(this->entries);
  }

private:
  a8_vm_meta_entry_s *static_entries[sizeof...(Ts)];
  std::tuple<a8_vm_meta_entry_t<Ts>...> entries;
};

#endif /* A8_VM_INTERNAL_SYMBOL_H_ */
