#include "a8/vm/internal/symbol.h"
#include "a8/vm/internal/symbol_table.h"
#include "a8/vm/data.h"

a8_vm_usize
a8_vm_symbol_table::meta_entry_size(const a8_vm_symid *eids, a8_vm_usize count)
    const noexcept {
  a8_vm_usize size = 0;
  for (a8_vm_usize i = 0; i < count; ++i) {
    auto *pent =
      a8_vm_symbol_get_meta_entry(eids[i], a8_vm_symid_data, this);
    if (!pent) {
      // TODO How to handle this?
      return 0;
    }
    auto *datap = a8_vm_meta_entry_s_value(a8_vm_data, pent);
    if ((datap->flags & a8_vm_data_flag_max_size) != a8_vm_data_flag_max_size)
    {
      // TODO How to handle this?
      return 0;
    }
    //
    size = a8_vm_std_align_ceil(a8_vm_usize, size)
      + sizeof(a8_vm_meta_entry_s);
    // TODO: handle data alignment. This needs to be stored in the header for
    // quick access.
    size += datap->size;
  }
  return size;
}

std::pair<a8_vm_symid, a8_vm_symbol *>
a8_vm_symbol_table::add(a8_vm_symid meta, a8_vm_usize size, a8_vm_usize align)
{
  size += a8_vm_usize(sizeof(a8_vm_symbol));
  // TODO: Align
  auto id = static_cast<a8_vm_symid>(_symbols.size());
  auto *psym = _symbols.emplace_back(
    static_cast<a8_vm_symbol *>(::operator new(size))).get();
  psym->meta = meta;
  return {id, psym};
}

std::pair<a8_vm_symid, a8_vm_meta_object_s *>
a8_vm_symbol_table::add_meta(const a8_vm_symid *eids, a8_vm_usize count) {
  // Size of
  auto size = meta_entry_size(eids, count);
  if (size == 0) {
    return {a8_vm_symid_undef, nullptr};
  }
  size += sizeof(a8_vm_meta_entry_s *) * count;

  auto id = static_cast<a8_vm_symid>(_symbols.size());
  auto *sym = reinterpret_cast<a8_vm_meta_object_s *>(
    _symbols.emplace_back(
      static_cast<a8_vm_symbol *>(::operator new(size))).get()
  );
  sym->base.base.meta = a8_vm_symid_meta_object;
  sym->base.self.id = id;
  sym->self.name = a8_vm_strid_empty;
  sym->self.parent = a8_vm_symid_undef;
  sym->self.flags = 0;
  sym->self.dynamic_alloc = 0;
  sym->self.dynamic_count = 0;
  sym->self.static_count = uint16_t(count);
  sym->self.entries = nullptr;

  auto **static_entries =
    reinterpret_cast<a8_vm_meta_entry_s **>(&sym->self.entries) + 1;
  char *p = reinterpret_cast<char *>(static_entries + count);
  for (a8_vm_usize i = 0; i < count; ++i) {
    auto *me = reinterpret_cast<a8_vm_meta_entry_s *>(p);
    me->base.meta = eids[i];
    me->self.id = id;
    static_entries[i] = me;

    auto *pent = a8_vm_symbol_get_meta_entry(eids[i], a8_vm_symid_data, this);
    assert(pent);
    auto *pdata = a8_vm_meta_entry_s_value(a8_vm_data, pent);
    p = a8_vm_align_ceil(char *, p + pdata->size, pdata->align);
  }

  return {id, sym};
}

a8_vm_usize
a8_vm_symbol_table::children(a8_vm_symid id) const noexcept {
  auto it = _children.find(id);
  if (it == _children.end()) {
    return 0;
  }
  return a8_vm_usize(it->second.size());
}

a8_vm_symid
a8_vm_symbol_table::child(a8_vm_symid id, a8_vm_strid name) const noexcept {
  auto it = _children.find(id);
  if (it == _children.end()) {
    return a8_vm_symid_undef;
  }
  auto it2 = it->second.find(name);
  if (it2 == it->second.end()) {
    return a8_vm_symid_undef;
  }
  return it2->second;
}

a8_vm_symid
a8_vm_symbol_table::put_child(a8_vm_symid id, a8_vm_strid name,
                              a8_vm_symid cid) {
  if (id >= _symbols.size()) {
    return a8_vm_symid_undef;
  }
  auto it = _children.find(id);
  if (it == _children.end()) {
    _children.emplace(id, std::map<a8_vm_strid, a8_vm_symid>{}).first
      ->second.emplace(name, cid);
    return cid;
  }
  auto [it2, inserted] = it->second.emplace(name, cid);
  if (inserted) {
    return cid;
  }
  return it2->second;
}

a8_vm_symid
a8_vm_symbol_table::set_child(a8_vm_symid id, a8_vm_strid name,
                              a8_vm_symid cid) {
  if (id >= _symbols.size()) {
    return a8_vm_symid_undef;
  }
  auto it = _children.find(id);
  if (it == _children.end()) {
    _children.emplace(id, std::map<a8_vm_strid, a8_vm_symid>{}).first
      ->second.emplace(name, cid);
    return a8_vm_symid_undef;
  }
  auto it2 = it->second.find(name);
  if (it2 == it->second.end()) {
    it->second.emplace(name, cid);
    return a8_vm_symid_undef;
  }
  auto prev_id = it2->second;
  it2->second = cid;
  return prev_id;
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
a8_vm_symbol_table_get_symbol(a8_vm_symbol_table *symtab, a8_vm_symid id) {
  return symtab->_symbols[id].get();
}

a8_vm_symid
a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab, a8_vm_symid meta,
                              a8_vm_usize size, a8_vm_usize align,
                              a8_vm_symbol **psym) {
  auto pair = symtab->add(meta, size, align);
  if (psym) {
    *psym = pair.second;
  }
  return pair.first;
}

a8_vm_symid
a8_vm_symbol_table_new_meta_object(a8_vm_symbol_table *symtab,
                                   const a8_vm_symid *eids, a8_vm_usize count,
                                   a8_vm_meta_object_s **pmo) {
  auto pair = symtab->add_meta(eids, count);
  if (pmo) {
    *pmo = pair.second;
  }
  return pair.first;
}

a8_vm_usize
a8_vm_symbol_table_child_count(const a8_vm_symbol_table *symtab,
                               a8_vm_symid id) {
  return symtab->children(id);
}

a8_vm_symid
a8_vm_symbol_table_get_child(const a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name) {
  return symtab->child(id, name);
}

a8_vm_symid
a8_vm_symbol_table_put_child(a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name, a8_vm_symid cid) {
  return symtab->put_child(id, name, cid);
}

a8_vm_symid
a8_vm_symbol_table_set_child(a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name, a8_vm_symid cid) {
  return symtab->set_child(id, name, cid);
}
// C API }}}
