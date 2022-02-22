#ifndef A8_VM_INTERNAL_SYMBOL_TABLE_H_
#define A8_VM_INTERNAL_SYMBOL_TABLE_H_

#include "../symbol_table.h"
#include "a8/vm/internal/symbol.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>

struct a8_vm_symbol_table {
  using symbol_list_t = std::vector<std::unique_ptr<a8_vm_symbol>>;
  using child_map_t = std::unordered_map<
    a8_vm_symid, std::map<a8_vm_strid, a8_vm_symid>>;

  symbol_list_t _symbols;
  child_map_t _children;

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

  // Sum the sizes of a list of meta entries, including any required
  // padding/alignment.
  a8_vm_usize
  meta_entry_size(const a8_vm_symid *ids, a8_vm_usize count) const noexcept;

  // Allocate a new symbol and returns its ID.
  std::pair<a8_vm_symid, a8_vm_symbol *>
  add(a8_vm_symid meta, a8_vm_usize size, a8_vm_usize align = a8_vm_std_align);

  template<class T>
  std::pair<a8_vm_symid, a8_vm_symbol_t<T> *>
  add(a8_vm_symid meta) {
    decltype(auto) pair = add(meta, sizeof(T), alignof(T));
    return {
      pair.first, static_cast<a8_vm_symbol_t<T> *>(pair.second)
    };
  }

  // Allocate and set up a meta object from a list of base IDs.
  std::pair<a8_vm_symid, a8_vm_meta_object_s *>
  add_meta(const a8_vm_symid *eids, a8_vm_usize count);

  /// @returns The number of children for symbol `id`.
  a8_vm_usize children(a8_vm_symid id) const noexcept;

  /// @returns The value of `id[name]` or `undef` if `name` is not a child of
  /// `id`.
  a8_vm_symid child(a8_vm_symid id, a8_vm_strid name) const noexcept;

  /// @returns The value of `id[name]` after the put operation: If it is a new
  /// entry, this is `cid`. Otherwise, `cid` is ignored and the value is not
  /// changed.
  a8_vm_symid put_child(a8_vm_symid id, a8_vm_strid name, a8_vm_symid cid);

  /// @returns The previous value of `id[name]`, or `undef` if it is a new
  /// entry.
  a8_vm_symid set_child(a8_vm_symid id, a8_vm_strid name, a8_vm_symid cid);
};

#endif // A8_VM_INTERNAL_SYMBOL_TABLE_H_
