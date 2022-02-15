#ifndef A8_VM_INTERNAL_SCOPE_H_
#define A8_VM_INTERNAL_SCOPE_H_

#include "../scope.h"
#include "../stdsym.h"

#include <vector>
#include <unordered_map>

struct a8_vm_scope {
  std::vector<a8_vm_symid> _symbols;
  std::unordered_map<a8_vm_strid, a8_vm_usize> _names;

  size_t size() const noexcept {
    return _symbols.size();
  }

  // Returns a8_vm_symid_undef on failure.
  a8_vm_usize get_index_for_name(a8_vm_strid name) const noexcept {
    auto it = _names.find(name);
    if (it == _names.end()) {
      return a8_vm_symid_undef;
    }
    return it->second;
  }

  // Returns a8_vm_symid_undef on failure.
  a8_vm_symid get_by_name(a8_vm_strid name) const noexcept {
    auto index = get_index_for_name(name);
    if (index == a8_vm_no_index) {
      return a8_vm_symid_undef;
    }
    return _symbols[index];
  }

  // Unchecked! Requires index < size().
  a8_vm_symid get_by_index(a8_vm_usize index) const noexcept {
    return _symbols[index];
  }

  // If the name exists, returns the index for the name. Otherwise adds the
  // name, pointing to a8_vm_symid_undef.
  a8_vm_usize put_name(a8_vm_strid name) {
    auto [it, inserted] = _names.emplace(name, _symbols.size());
    if (inserted) {
      _symbols.emplace_back(a8_vm_symid_undef);
    }
    return it->second;
  }

  // If the name exists, replaces the symbol at the name, otherwise adds it.
  a8_vm_usize set_name(a8_vm_strid name, a8_vm_symid symbol) {
    auto [it, inserted] = _names.emplace(name, _symbols.size());
    if (inserted) {
      _symbols.emplace_back(symbol);
    } else {
      _symbols[it->second] = symbol;
    }
    return it->second;
  }

  // Unchecked! Replaces the symbol at index. Returns the previous value.
  a8_vm_symid set_index(a8_vm_usize index, a8_vm_symid symbol) {
    a8_vm_symid old = _symbols[index];
    _symbols[index] = symbol;
    return old;
  }
};

#endif // A8_VM_INTERNAL_SCOPE_H_
