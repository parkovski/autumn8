#include "a8/vm/internal/symbol.h"
#include "a8/vm/internal/symbol_table.h"
#include "a8/vm/data.h"
#include <new>

a8_vm_symbol *
a8_vm_symbol_get(a8_vm_symid id, const a8_vm_symbol_table *symtab) {
  return a8_vm_symbol_table_get_symbol(symtab, id);
}

/*void *
a8_vm_symbol_get_value(a8_vm_symid id, const a8_vm_symbol_table *symtab) {
  auto sym = a8_vm_symbol_table_get_symbol(symtab, symid);
  if (sym == nullptr) {
    return nullptr;
  }
}*/

a8_vm_meta_object_s *
a8_vm_symbol_get_meta_object(a8_vm_symid id, const a8_vm_symbol_table *symtab)
{
  auto sym = a8_vm_symbol_table_get_symbol(symtab, id);
  if (sym == nullptr || sym->meta == a8_vm_symid_undef) {
    return nullptr;
  }
  return reinterpret_cast<a8_vm_meta_object_s *>(
    a8_vm_symbol_table_get_symbol(symtab, sym->meta)
  );
}

a8_vm_usize
a8_vm_meta_object_size(const a8_vm_symid *eids, a8_vm_usize count,
                       const a8_vm_symbol_table *symtab) {
  // TODO: Alignment
  return a8_vm_usize(sizeof(a8_vm_meta_object_m)) +
    symtab->meta_entry_size(eids, count);
}

a8_vm_meta_entry_s *
a8_vm_meta_object_cast(a8_vm_meta_object *m, a8_vm_symid eid) {
  // Not likely, but generic code might want to do this.
  if (eid == a8_vm_symid_symbol) {
    return reinterpret_cast<a8_vm_meta_entry_s *>(
      reinterpret_cast<char *>(m) - sizeof(a8_vm_meta_entry_s)
    );
  }

  // Check static bases.
  // Alignment of p is guaranteed via static_assert.
  for (a8_vm_usize n = m->static_count; n; --n) {
    auto *pent = &a8_vm_meta_object_static_entry(m, n);
    if (pent->base.meta == eid) {
      return pent;
    }
  }

  // Check dynamic bases.
  for (a8_vm_usize n = 0; n < m->dynamic_count; ++n) {
    auto *pent = &a8_vm_meta_object_dynamic_entry(m, n);
    if (pent->base.meta == eid) {
      return pent;
    }
  }

  // Nothing found.
  return nullptr;
}

a8_vm_meta_entry_s *
a8_vm_meta_object_cast_append(a8_vm_meta_object *m,
                              const a8_vm_symbol_table *symtab,
                              a8_vm_symid eid) {
  // Easy way out - it already exists.
  auto *pent = a8_vm_meta_object_cast(m, eid);
  if (pent) {
    return pent;
  }

  auto *ms = reinterpret_cast<a8_vm_meta_object_m *>(
    reinterpret_cast<char *>(m) - sizeof(a8_vm_meta_entry)
  );
  // We can only instantiate things that describe their data layout.
  // Also require the max_size flag to be sure no one intends to overflow
  // this data.
  // TODO: If max_size is not set, see if the symbol has a function that
  // can calculate it.
  auto *ebase = a8_vm_symbol_get_meta_entry(ms->base.id,
                                            a8_vm_symid_data, symtab);
  if (!ebase) {
    return nullptr;
  }
  auto *datap = a8_vm_meta_entry_value(a8_vm_data, ebase);
  if ((datap->flags & a8_vm_data_flag_max_size) != a8_vm_data_flag_max_size) {
    return nullptr;
  }

  // Make sure there's room to add another dynamic base.
  if (ms->self.dynamic_alloc == ms->self.dynamic_count) {
    uint16_t new_size;
    if (ms->self.dynamic_alloc < 4) {
      new_size = 4;
    } else {
      new_size = a8_vm_align_ceil(uint16_t, ms->self.dynamic_alloc, 8);
    }
    auto **old_array = ms->self.entries;
    auto **new_array = new a8_vm_meta_entry_s *[new_size];
    for (uint16_t i = 0; i < ms->self.dynamic_alloc; ++i) {
      new_array[i] = old_array[i];
    }
    delete [] old_array;
    ms->self.entries = new_array;
    ms->self.dynamic_alloc = new_size;
  }

  // Calculate the total size and allocate space.
  // TODO: Handle alignment. Need to store in base header.
  //size_t align = a8_vm_data_get_align(datap);
  size_t size = sizeof(a8_vm_meta_entry_s);
  //size = a8_vm_align_ceil(size_t, size, align) + datap->size;
  size += datap->size;
  pent =
    ms->self.entries[ms->self.dynamic_count] =
    static_cast<a8_vm_meta_entry_s *>(::operator new(size));
  ++ms->self.dynamic_count;

  // Set up the metadata. The data is not cleared - this is the caller's
  // responsibility.
  pent->base.meta = eid;
  pent->self.id = ms->base.id;
  // TODO: Size : align
  return pent;
}

/*
a8_vm_usize
a8_vm_meta_object_append_all(a8_vm_meta_object *m, const a8_vm_symid *eids,
                             a8_vm_usize count,
                             const a8_vm_symbol_table *symtab) {
  // TODO: There are much better ways to do this, but best is to have some
  // scratch area - either the VM stack or the native stack with alloca.
  a8_vm_usize ok = 0;
  for (a8_vm_usize i = 0; i < count; ++i) {
    if (a8_vm_meta_object_cast_append(m, symtab, eids[i])) {
      ++ok;
    }
  }
  return ok;
}
*/

a8_vm_meta_entry_s *
a8_vm_symbol_get_meta_entry(a8_vm_symid id, a8_vm_symid eid,
                            const a8_vm_symbol_table *symtab) {
  auto m = a8_vm_symbol_get_meta_object(id, symtab);
  if (m == nullptr) {
    return nullptr;
  }
  return a8_vm_meta_object_cast(&m->self, eid);
}

a8_vm_meta_entry_s *
a8_vm_meta_entry_cast(a8_vm_meta_entry *e, a8_vm_symid eid,
                      const a8_vm_symbol_table *symtab) {
  auto *m = reinterpret_cast<a8_vm_meta_object_s *>(
    a8_vm_symbol_table_get_symbol(symtab, e->id)
  );
  return a8_vm_meta_object_cast(&m->self, eid);
}
