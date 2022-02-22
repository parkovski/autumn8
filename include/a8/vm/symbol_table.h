#ifndef A8_VM_SYMBOL_TABLE_H_
#define A8_VM_SYMBOL_TABLE_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t
a8_vm_symbol_table_alloc_size();

a8_vm_symbol_table *
a8_vm_symbol_table_init(a8_vm_symbol_table *symtab);

a8_vm_symbol_table *
a8_vm_symbol_table_destroy(a8_vm_symbol_table *symtab);

/**
 * Returns the symbol with id `symid`, or null if it doesn't exist.
 */
a8_vm_symbol *
a8_vm_symbol_table_get_symbol(const a8_vm_symbol_table *symtab,
                              a8_vm_symid id);

/**
 * Returns the ID of a new symbol with `size` bytes available after the header,
 * aligned to `align`, a power of 2.
 */
a8_vm_symid
a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab, a8_vm_symid meta,
                              a8_vm_usize size, a8_vm_usize align,
                              a8_vm_symbol **psym);

a8_vm_symid
a8_vm_symbol_table_new_meta_object(a8_vm_symbol_table *symtab,
                                   const a8_vm_symid *eids, a8_vm_usize count,
                                   a8_vm_meta_object_s **pmo);

/// @returns The number of children for symbol `id`.
a8_vm_usize
a8_vm_symbol_table_child_count(const a8_vm_symbol_table *symtab,
                               a8_vm_symid id);

/// @returns The value of `id[name]` or `undef` if `name` is not a child of
/// `id`.
a8_vm_symid
a8_vm_symbol_table_get_child(const a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name);

/// @returns The value of `id[name]` after the put operation: If it is a new
/// entry, this is `cid`. Otherwise, `cid` is ignored and the value is not
/// changed.
a8_vm_symid
a8_vm_symbol_table_put_child(a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name, a8_vm_symid cid);

/// @returns The previous value of `id[name]`, or `undef` if it is a new
/// entry.
a8_vm_symid
a8_vm_symbol_table_set_child(a8_vm_symbol_table *symtab, a8_vm_symid id,
                             a8_vm_strid name, a8_vm_symid cid);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_SYMBOL_TABLE_H_ */
