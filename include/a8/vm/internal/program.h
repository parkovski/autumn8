#ifndef A8_VM_INTERNAL_PROGRAM_H_
#define A8_VM_INTERNAL_PROGRAM_H_

#include "string_map.h"
#include "symbol_table.h"
#include "scope.h"
#include "ibuf.h"
#include "stack.h"

struct a8_vm_program {
  a8_vm_string_map _strings;
  a8_vm_symbol_table _symtab;
  a8_vm_scope _root;
  a8_vm_ibuf _buffer;
  a8_vm_stack _stack;

  explicit a8_vm_program() {
    // TODO: Find ideal number.
    _symtab._symbols.reserve(256);
  }

  a8_vm_strid get_strid(std::string_view str) {
    return _strings[str];
  }

  a8_vm_string get_string(std::string_view str) {
    return _strings.string(str);
  }

  a8_vm_string get_string(a8_vm_strid id) const noexcept {
    return _strings.string(id);
  }
};

#endif // A8_VM_INTERNAL_PROGRAM_H_
