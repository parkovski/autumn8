#ifndef A8_VM_INTERNAL_SYMBOL_TABLE_H_
#define A8_VM_INTERNAL_SYMBOL_TABLE_H_

#include "a8/vm/vm.h"

#include <vector>
#include <memory>

struct a8_vm_symbol_table {
  std::vector<std::unique_ptr<a8_vm_symbol>> _symbols;

  // Unchecked!
  a8_vm_symbol *at(a8_vm_symid symid) const noexcept {
    return _symbols[symid].get();
  }

  // Checked.
  a8_vm_symbol *operator[](a8_vm_symid symid) const noexcept {
    if (symid >= _symbols.size()) {
      return nullptr;
    }
    return at(symid);
  }

  // Sum the sizes of a list of symbol bases, including any required
  // padding/alignment.
  a8_vm_usize base_size(a8_vm_symid *ids, a8_vm_usize count) /*const*/ noexcept;

  // Add a preallocated symbol. Set up base fields only.
  a8_vm_symid add(a8_vm_symbol *symbol, a8_vm_usize size);

  // Add a preallocated symbol. Set up all symbol fields and all static base
  // headers.
  a8_vm_symid add(a8_vm_symbol *symbol, a8_vm_usize size, a8_vm_symid *ids,
                  a8_vm_usize count);

  // Allocate and set up a symbol from a list of base IDs.
  a8_vm_symid add(a8_vm_symid *ids, a8_vm_usize count);
};

#endif // A8_VM_INTERNAL_SYMBOL_TABLE_H_
