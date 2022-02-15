#ifndef A8_VM_SCOPE_H_
#define A8_VM_SCOPE_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct a8_vm_scope a8_vm_scope;

size_t
a8_vm_scope_alloc_size();

a8_vm_scope *
a8_vm_scope_init(a8_vm_scope *scope);

a8_vm_scope *
a8_vm_scope_destroy(a8_vm_scope *scope);

a8_vm_usize
a8_vm_scope_get_child_count(a8_vm_scope *scope);

a8_vm_symid
a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_strid name);

a8_vm_symid
a8_vm_scope_get_child_at(a8_vm_scope *scope, a8_vm_usize index);

a8_vm_usize
a8_vm_scope_get_child_index(a8_vm_scope *scope, a8_vm_strid name);

a8_vm_usize
a8_vm_scope_put_name(a8_vm_scope *scope, a8_vm_strid name);

a8_vm_usize
a8_vm_scope_set_child(a8_vm_scope *scope, a8_vm_strid name,
                      a8_vm_symid symbol);

a8_vm_symid
a8_vm_scope_set_child_at(a8_vm_scope *scope, a8_vm_usize index,
                         a8_vm_symid symbol);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_SCOPE_H_ */
