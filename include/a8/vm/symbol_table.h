#ifndef A8_VM_SYMBOL_TABLE_H_
#define A8_VM_SYMBOL_TABLE_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct a8_vm_symbol_table a8_vm_symbol_table;

size_t
a8_vm_symbol_table_alloc_size();

a8_vm_symbol_table *
a8_vm_symbol_table_init(a8_vm_symbol_table *symtab);

a8_vm_symbol_table *
a8_vm_symbol_table_destroy(a8_vm_symbol_table *symtab);

a8_vm_symbol *
a8_vm_symbol_table_get_symbol(a8_vm_symbol_table *symtab, a8_vm_symid symid);

a8_vm_symbol_base *
a8_vm_symbol_table_get_symbol_base(a8_vm_symbol_table *st, a8_vm_symid symid,
                                   a8_vm_symid base);

a8_vm_usize
a8_vm_symbol_table_get_size(a8_vm_symbol_table *symtab, a8_vm_symid *ids,
                            a8_vm_usize count);

a8_vm_symid
a8_vm_symbol_table_add_symbol(a8_vm_symbol_table *symtab, a8_vm_symbol *symbol,
                              a8_vm_usize size, a8_vm_symid *ids,
                              a8_vm_usize count);

a8_vm_symid
a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab, a8_vm_symid *ids,
                              a8_vm_usize count);

a8_vm_symbol_base *
a8_vm_symbol_cast(a8_vm_symbol *symbol, a8_vm_symid base);

a8_vm_symbol_base *
a8_vm_symbol_ensure_cast(a8_vm_symbol *symbol, a8_vm_symbol_table *symtab,
                         a8_vm_symid base);

a8_vm_usize
a8_vm_symbol_ensure(a8_vm_symbol *symbol, a8_vm_symbol_table *symtab,
                    a8_vm_symid *bases, a8_vm_usize count);

#define a8_vm_symbol_base_value(T, base) \
  a8_vm_align_ceil(T*, ((char*)(base)) + sizeof(a8_vm_symbol_base), alignof(T))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_SYMBOL_TABLE_H_ */
