#include "a8/vm/internal/symbol_table.h"

a8_vm_usize
a8_vm_symbol_table::base_size(a8_vm_symid *ids, a8_vm_usize count) /*const*/
    noexcept {
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

a8_vm_symid
a8_vm_symbol_table::add(a8_vm_symbol *symbol, a8_vm_usize size) {
  auto id = static_cast<a8_vm_symid>(_symbols.size());
  _symbols.emplace_back(std::unique_ptr<a8_vm_symbol>(symbol));
  symbol->base.meta = a8_vm_symid_symbol;
  symbol->base.self = id;
  symbol->base.size = size;
  return id;
}

a8_vm_symid
a8_vm_symbol_table::add(a8_vm_symbol *symbol, a8_vm_usize size,
                        a8_vm_symid *ids, a8_vm_usize count) {
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

a8_vm_symid
a8_vm_symbol_table::add(a8_vm_symid *ids, a8_vm_usize count) {
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

// C API {{{
size_t
a8_vm_symbol_table_alloc_size() {
  return sizeof(a8_vm_symbol_table);
}

a8_vm_symbol_table *
a8_vm_symbol_table_init(a8_vm_symbol_table *symtab) {
  new (symtab) a8_vm_symbol_table;
  return symtab;
}

a8_vm_symbol_table *
a8_vm_symbol_table_destroy(a8_vm_symbol_table *symtab) {
  symtab->~a8_vm_symbol_table();
  return symtab;
}

a8_vm_symbol *
a8_vm_symbol_table_get_symbol(a8_vm_symbol_table *symtab, a8_vm_symid symid) {
  return symtab->_symbols[symid].get();
}

a8_vm_symbol_base *
a8_vm_symbol_table_get_symbol_base(a8_vm_symbol_table *st, a8_vm_symid symid,
                                   a8_vm_symid base) {
  return a8_vm_symbol_cast(a8_vm_symbol_table_get_symbol(st, symid), base);
}

a8_vm_usize
a8_vm_symbol_table_get_size(a8_vm_symbol_table *symtab, a8_vm_symid *ids,
                            a8_vm_usize count) {
  return symtab->base_size(ids, count);
}

a8_vm_symid
a8_vm_symbol_table_add_symbol(a8_vm_symbol_table *symtab, a8_vm_symbol *symbol,
                              a8_vm_usize size, a8_vm_symid *ids,
                              a8_vm_usize count) {
  return symtab->add(symbol, size, ids, count);
}

a8_vm_symid
a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab, a8_vm_symid *ids,
                              a8_vm_usize count) {
  return symtab->add(ids, count);
}

a8_vm_symbol_base *
a8_vm_symbol_cast(a8_vm_symbol *symbol, a8_vm_symid base) {
  // Check static bases.
  // Alignment of p is guaranteed via static_assert.
  char *p = reinterpret_cast<char *>(&symbol->base) + sizeof(a8_vm_symbol);
  for (a8_vm_usize n = symbol->static_count; n; --n) {
    auto *basep = reinterpret_cast<a8_vm_symbol_base *>(p);
    if (basep->self == base) {
      return basep;
    }
    p += sizeof(a8_vm_symbol_base) + basep->size;
    p = a8_vm_std_align_ceil(char *, p);
  }

  // Check dynamic bases.
  for (a8_vm_usize n = 0; n < symbol->dynamic_count; ++n) {
    auto *basep = symbol->dynamic_base[n];
    if (basep->self == base) {
      return basep;
    }
  }

  // Were they trying to be funny?
  // Important - this keeps people from adding symbols to the dynamic base
  // list, which would be confusing.
  if (base == a8_vm_symid_symbol) {
    return &symbol->base;
  }

  // Nothing found.
  return nullptr;
}

a8_vm_symbol_base *
a8_vm_symbol_ensure_cast(a8_vm_symbol *symbol, a8_vm_symbol_table *symtab,
                         a8_vm_symid base) {
  // Easy way out - it already exists.
  auto *basep = a8_vm_symbol_cast(symbol, base);
  if (basep) {
    return basep;
  }

  // We can only instantiate things that describe their data layout.
  // Also require the max_size flag to be sure no one intends to overflow
  // this data.
  // TODO: If max_size is not set, see if the symbol has a function that
  // can calculate it.
  auto *data_base =
    a8_vm_symbol_table_get_symbol_base(symtab, symbol->base.meta,
                                       a8_vm_symid_data);
  auto *datap = a8_vm_symbol_base_value(a8_vm_data, data_base);
  // Alignment guaranteed via static_assert.
  if (!datap ||
      (datap->flags & a8_vm_data_flag_max_size) != a8_vm_data_flag_max_size) {
    return nullptr;
  }

  // Make sure there's room to add another dynamic base.
  if (symbol->dynamic_alloc <= symbol->dynamic_count) {
    uint16_t new_size;
    if (symbol->dynamic_alloc < 4) {
      new_size = 4;
    } else {
      new_size = a8_vm_align_ceil(uint16_t, symbol->dynamic_alloc, 8);
    }
    auto **new_array = new a8_vm_symbol_base *[new_size];
    for (uint16_t i = 0; i < symbol->dynamic_alloc; ++i) {
      new_array[i] = symbol->dynamic_base[i];
    }
    delete [] symbol->dynamic_base;
    symbol->dynamic_base = new_array;
    symbol->dynamic_alloc = new_size;
  }

  // Calculate the total size and allocate space.
  // TODO: Handle alignment. Need to store in base header.
  //size_t align = a8_vm_data_get_align(datap);
  size_t size = sizeof(a8_vm_symbol_base);
  //size = a8_vm_align_ceil(size_t, size, align) + datap->size;
  size += datap->size;
  basep =
    symbol->dynamic_base[symbol->dynamic_count] =
    static_cast<a8_vm_symbol_base *>(::operator new(size));
  ++symbol->dynamic_count;

  // Set up the metadata. The data is not cleared - this is the caller's
  // responsibility.
  basep->self = symbol->base.self;
  basep->meta = base;
  // TODO: Size : align
  basep->size = size;
  return basep;
}

a8_vm_usize
a8_vm_symbol_ensure(a8_vm_symbol *symbol, a8_vm_symbol_table *symtab,
                    a8_vm_symid *bases, a8_vm_usize count) {
  // TODO: There are much better ways to do this, but best is to have some
  // scratch area - either the VM stack or the native stack with alloca.
  a8_vm_usize ok = 0;
  for (a8_vm_usize i = 0; i < count; ++i) {
    if (a8_vm_symbol_ensure_cast(symbol, symtab, bases[i])) {
      ++ok;
    }
  }
  return ok;
}
// C API }}}
