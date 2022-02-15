#include "a8/vm/internal/scope.h"

// C API {{{
size_t
a8_vm_scope_alloc_size() {
  return sizeof(a8_vm_scope);
}

a8_vm_scope *
a8_vm_scope_init(a8_vm_scope *scope) {
  new (scope) a8_vm_scope;
  return scope;
}

a8_vm_scope *
a8_vm_scope_destroy(a8_vm_scope *scope) {
  scope->~a8_vm_scope();
  return scope;
}

a8_vm_usize
a8_vm_scope_get_child_count(a8_vm_scope *scope) {
  return static_cast<a8_vm_usize>(scope->size());
}

a8_vm_symid
a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_strid name) {
  return scope->get_by_name(name);
}

a8_vm_symid
a8_vm_scope_get_child_at(a8_vm_scope *scope, a8_vm_usize index) {
  return scope->get_by_index(index);
}

a8_vm_usize
a8_vm_scope_get_child_index(a8_vm_scope *scope, a8_vm_strid name) {
  return scope->get_index_for_name(name);
}

a8_vm_usize
a8_vm_scope_put_name(a8_vm_scope *scope, a8_vm_strid name) {
  return scope->put_name(name);
}

a8_vm_usize
a8_vm_scope_set_child(a8_vm_scope *scope, a8_vm_strid name,
                      a8_vm_symid symbol) {
  return scope->set_name(name, symbol);
}

a8_vm_symid
a8_vm_scope_set_child_at(a8_vm_scope *scope, a8_vm_usize index,
                         a8_vm_symid symbol) {
  return scope->set_index(index, symbol);
}
// C API }}}
