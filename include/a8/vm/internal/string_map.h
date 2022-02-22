#ifndef A8_VM_INTERNAL_STRING_MAP_H_
#define A8_VM_INTERNAL_STRING_MAP_H_

#include "../string_map.h"
#include "../stdsym.h"

#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>

// Contains size+1 characters - always null terminated.
struct a8_vm_intern_string {
  a8_vm_usize size;

  a8_vm_intern_string() = delete;

  explicit a8_vm_intern_string(const char *str, a8_vm_usize length)
    : size{length}
  {
    std::char_traits<char>::copy(data(), str, size);
    data()[size] = 0;
  }

  void *operator new(size_t size, a8_vm_usize length) noexcept {
    auto *p =
      reinterpret_cast<a8_vm_intern_string *>(new char[size + length + 1]);
    return p;
  }

  void operator delete(void *p) noexcept {
    delete [] static_cast<char *>(p);
  }

  const char *data() const noexcept {
    return reinterpret_cast<const char *>(this) + sizeof(*this);
  }

  char *data() noexcept {
    return reinterpret_cast<char *>(this) + sizeof(*this);
  }

  operator std::string_view() const noexcept {
    return {data(), size};
  }

  operator a8_vm_string() const noexcept {
    return {data(), size};
  }

  bool operator==(const a8_vm_intern_string &str) const noexcept {
    return this == &str;
  }

  bool operator!=(const a8_vm_intern_string &str) const noexcept {
    return this != &str;
  }

  bool operator<(const a8_vm_intern_string &str) const noexcept {
    return std::string_view{*this} < std::string_view{str};
  }
};

template<>
struct std::hash<a8_vm_intern_string> {
  size_t operator()(const a8_vm_intern_string &str) {
    return hash<std::string_view>{}(str);
  }
};

struct a8_vm_string_map {
  std::vector<std::unique_ptr<a8_vm_intern_string>> _strings;
  std::unordered_map<std::string_view, a8_vm_strid> _stringsIndex;

  explicit a8_vm_string_map();

  size_t size() const noexcept {
    return static_cast<a8_vm_usize>(_strings.size());
  }

  a8_vm_strid operator[](std::string_view sv);
  a8_vm_strid get(std::string_view sv) {
    return (*this)[sv];
  }

  a8_vm_string string(a8_vm_strid strid) const noexcept {
    return *_strings[strid];
  }

  a8_vm_string string(std::string_view sv) {
    return string((*this)[sv]);
  }
};

#endif // A8_VM_INTERNAL_STRING_MAP_H_
