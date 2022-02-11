#ifndef a8_vm_symbol_table_h_
#define a8_vm_symbol_table_h_

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
  a8_vm_usize base_size(a8_vm_symid *ids, a8_vm_usize count) /*const*/ noexcept {
    a8_vm_usize size = 0;
    for (a8_vm_usize i = 0; i < count; ++i) {
      auto *data_base =
        a8_vm_symbol_table_get_symbol_base(this, ids[i], a8_vm_symid_data);
      auto *datap = a8_vm_symbol_base_value(a8_vm_data, data_base);
      if (!datap
        || (datap->flags & a8_vm_data_flag_max_size)
          != a8_vm_data_flag_max_size) {
        // TODO How to handle this?
        return 0;
      }
      //
      size = a8_vm_std_align_ceil(a8_vm_usize, size)
        + sizeof(a8_vm_symbol_base);
      // TODO: handle data alignment. This needs to be stored in the header for
      // quick access.
      size += datap->size;
    }
    return size;
  }

  // Add a preallocated symbol. Set up base fields only.
  a8_vm_symid add(a8_vm_symbol *symbol, a8_vm_usize size) {
    auto id = static_cast<a8_vm_symid>(_symbols.size());
    _symbols.emplace_back(std::unique_ptr<a8_vm_symbol>(symbol));
    symbol->base.meta = a8_vm_symid_symbol;
    symbol->base.self = id;
    symbol->base.size = size;
    return id;
  }

  // Add a preallocated symbol. Set up all symbol fields and all static base
  // headers.
  a8_vm_symid add(a8_vm_symbol *symbol, a8_vm_usize size, a8_vm_symid *ids,
                  a8_vm_usize count) {
    auto id = add(symbol, size);
    symbol->name = a8_vm_strid_empty;
    symbol->parent = a8_vm_symid_undef;
    symbol->dynamic_base = nullptr;
    symbol->dynamic_alloc = 0;
    symbol->dynamic_count = 0;
    symbol->static_count = static_cast<uint16_t>(count);

    char *p = reinterpret_cast<char *>(&symbol->base) + sizeof(a8_vm_symbol);
    for (a8_vm_usize i = 0; i < count; ++i) {
      auto *data_base =
        a8_vm_symbol_table_get_symbol_base(this, ids[i], a8_vm_symid_data);
      auto *datap = a8_vm_symbol_base_value(a8_vm_data, data_base);
      auto *base = reinterpret_cast<a8_vm_symbol_base *>(p);
      base->self = symbol->base.self;
      base->meta = ids[i];
      base->size = datap->size;
    }

    return id;
  }

  // Allocate and set up a symbol from a list of base IDs.
  a8_vm_symid add(a8_vm_symid *ids, a8_vm_usize count) {
    size_t size = sizeof(a8_vm_symbol);
    if (count > 0) {
      if (auto basesz = base_size(ids, count)) {
        size += basesz;
      } else {
        // TODO Error handling
        return a8_vm_symid_undef;
      }
    }
    auto symbol = static_cast<a8_vm_symbol *>(::operator new (size));
    return add(symbol, static_cast<a8_vm_usize>(size), ids, count);
  }
};

#endif // a8_vm_symbol_table_h_
