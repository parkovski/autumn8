#include "a8/vm/vm.h"
#include <cstdint>
#include <new>

#include <vector>
#include <memory>
#include <string_view>
#include <unordered_map>

struct a8_vm_scope {
  std::unordered_map<std::string_view, std::unique_ptr<a8_vm_symbol>> children;
};

struct a8_vm_program {
  std::vector<std::unique_ptr<const char []>> _strings;
  std::unordered_map<std::string_view, size_t> _stringsIndex;

  a8_vm_symbol *_std[a8_vm_stdsym_max__];
};

// Program {{{
a8_vm_program *
a8_vm_program_new() {
  return new (std::nothrow) a8_vm_program;
}

void
a8_vm_program_free(a8_vm_program *program) {
  delete program;
}

a8_vm_name
a8_vm_program_get_name(a8_vm_program *program, const char *name, size_t length) {
  std::string_view name_sv;
  if (length) {
    name_sv = {name, length};
  } else {
    name_sv = name;
    length = name_sv.size();
  }
  auto nextIndex = program->_strings.size();
  auto [it, inserted] = program->_stringsIndex.emplace(name_sv, nextIndex);
  if (inserted) {
    auto string = std::make_unique<char[]>(length + 1);
    std::char_traits<char>::copy(string.get(), name_sv.data(), length);
    string[length] = 0;
    *const_cast<std::string_view *>(&it->first) = {string.get(), length};
    program->_strings.emplace_back(std::move(string));
  }
  return {program->_strings[it->second].get(), length};
}

a8_vm_symref
a8_vm_program_get_stdsym(a8_vm_program *program, a8_vm_stdsym id) {
  return program->_std[id];
}
// Program }}}

// Symbol {{{
uint32_t a8_vm_symbol_get_part_count(a8_vm_symref sym) {
  return sym->count;
}

uint32_t a8_vm_symbol_get_part_index(a8_vm_symref sym, a8_vm_symref part) {
  for (uint32_t i = 0; i < sym->count; ++i) {
    if (a8_vm_symbol_get_meta_at(sym, i) == part) {
      return i;
    }
  }
  return A8_VM_SYMBOL_NO_INDEX;
}

a8_vm_symref a8_vm_symbol_get_meta_at(a8_vm_symref sym, uint32_t index) {
  return reinterpret_cast<a8_vm_symbol *>(
    reinterpret_cast<char *>(sym) + sym->parts[index].moff);
}

void *a8_vm_symbol_get_value_at(a8_vm_symref sym, uint32_t index) {
  return reinterpret_cast<char *>(sym) + sym->parts[index].voff;
}

void *a8_vm_symbol_get_value(a8_vm_symref sym, a8_vm_symref part) {
  auto index = a8_vm_symbol_get_part_index(sym, part);
  if (index == A8_VM_SYMBOL_NO_INDEX) {
    return nullptr;
  }
  return a8_vm_symbol_get_value_at(sym, index);
}
// Symbol }}}

// Scope {{{
uint32_t a8_vm_scope_get_child_count(a8_vm_scope *scope) {
  return scope->children.size();
}

a8_vm_symref a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_name name) {
  auto it = scope->children.find(std::string_view{name.str, name.len});
  if (it == scope->children.end()) {
    return nullptr;
  }
  return it->second.get();
}

a8_vm_symref a8_vm_scope_add_child(a8_vm_scope *scope, a8_vm_name name,
                                   a8_vm_symref *parts) {
  // TODO
  return nullptr;
}
// Scope }}}

// Data {{{
uint32_t a8_vm_data_get_align(a8_vm_data *data) {
  return 1 << data->align;
}

bool a8_vm_data_set_align(a8_vm_data *data, uint32_t align) {
  for (uint32_t i = 0; i <= A8_VM_DATA_MAX_ALIGN_VALUE; ++i) {
    if (align == (1 << i)) {
      data->align = i;
      return true;
    }
  }
  return false;
}
// Data }}}
