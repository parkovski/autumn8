#ifndef a8_vm_string_map_h_
#define a8_vm_string_map_h_

#include "a8/vm/vm.h"

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

  explicit a8_vm_string_map() {
    // TODO: Find ideal numbers for these.
    _strings.reserve(256);
    _stringsIndex.reserve(256);
    // Put the empty string into the map.
    auto p = new (0) a8_vm_intern_string("", 0);
    _strings.emplace_back(std::unique_ptr<a8_vm_intern_string>(p));
    _stringsIndex.emplace(std::string_view{p->data(), 0}, 0);
  }

  size_t size() const noexcept {
    return static_cast<a8_vm_usize>(_strings.size());
  }

  a8_vm_strid operator[](std::string_view sv) {
    auto it = _stringsIndex.find(sv);
    if (it != _stringsIndex.end()) {
      return it->second;
    }
    auto nextIndex = static_cast<a8_vm_strid>(_strings.size());
    auto p = new (sv.length()) a8_vm_intern_string(sv.data(), sv.length());
    _stringsIndex.emplace(std::string_view{p->data(), p->size}, nextIndex);
    _strings.emplace_back(std::unique_ptr<a8_vm_intern_string>(p));
    return nextIndex;
  }

  a8_vm_string string(a8_vm_strid strid) const noexcept {
    return *_strings[strid];
  }

  a8_vm_string string(std::string_view sv) {
    return string((*this)[sv]);
  }
};

#endif // a8_vm_string_map_h_
