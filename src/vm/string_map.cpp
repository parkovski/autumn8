#include "a8/vm/internal/string_map.h"

a8_vm_string_map::a8_vm_string_map() {
  // TODO: Find ideal numbers for these.
  _strings.reserve(256);
  _stringsIndex.reserve(256);
  // Put the empty string into the map.
  auto p = new (0) a8_vm_intern_string("", 0);
  _strings.emplace_back(std::unique_ptr<a8_vm_intern_string>(p));
  _stringsIndex.emplace(std::string_view{p->data(), 0}, 0);
}

a8_vm_strid a8_vm_string_map::operator[](std::string_view sv) {
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

// C API {{{
size_t
a8_vm_string_map_alloc_size() {
  return sizeof(a8_vm_string_map);
}

a8_vm_string_map *
a8_vm_string_map_init(a8_vm_string_map *map) {
  new (map) a8_vm_string_map;
  return map;
}

a8_vm_string_map *
a8_vm_string_map_destroy(a8_vm_string_map *map) {
  map->~a8_vm_string_map();
  return map;
}

a8_vm_strid
a8_vm_string_map_get_id(a8_vm_string_map *map, const char *s, size_t length) {
  std::string_view sv;
  if (length) {
    sv = {s, length};
  } else {
    sv = s;
  }
  return (*map)[sv];
}

a8_vm_string
a8_vm_string_map_get_string(a8_vm_string_map *map, const char *s,
                            size_t length) {
  std::string_view sv;
  if (length) {
    sv = {s, length};
  } else {
    sv = s;
  }
  return map->string(sv);
}

a8_vm_string
a8_vm_string_map_get_string_at(a8_vm_string_map *map, a8_vm_strid id) {
  return map->string(id);
}
// C API }}}
