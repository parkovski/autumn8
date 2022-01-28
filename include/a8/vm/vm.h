#ifndef A8_VM_VM_H_
#define A8_VM_VM_H_

#include "stdsym.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Program {{{
a8_vm_program *a8_vm_program_new();
void           a8_vm_program_free(a8_vm_program *program);
a8_vm_name     a8_vm_program_get_name(a8_vm_program *program, const char *name,
                                      size_t length);
a8_vm_symref   a8_vm_program_get_stdsym(a8_vm_program *program,
                                        a8_vm_stdsym id);
a8_vm_scope   *a8_vm_program_get_scope(a8_vm_program *program);
// Program }}}

// Symbol {{{
uint32_t a8_vm_symbol_get_part_count(a8_vm_symref sym);
uint32_t a8_vm_symbol_get_part_index(a8_vm_symref sym, a8_vm_symref part);
a8_vm_symref a8_vm_symbol_get_meta_at(a8_vm_symref sym, uint32_t index);
void *a8_vm_symbol_get_value_at(a8_vm_symref sym, uint32_t index);
void *a8_vm_symbol_get_value(a8_vm_symref sym, a8_vm_symref part);
// Symbol }}}

// Scope {{{
uint32_t a8_vm_scope_get_child_count(a8_vm_scope *scope);
uint32_t a8_vm_scope_get_child_index(a8_vm_scope *scope, a8_vm_name name);
a8_vm_symref a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_name name);
a8_vm_symref a8_vm_scope_get_child_at(a8_vm_scope *scope, uint32_t index);
a8_vm_symref a8_vm_scope_add_child(a8_vm_scope *scope, a8_vm_name name,
                                   a8_vm_symref *parts);
// Scope }}}

// Data {{{
uint32_t a8_vm_data_get_align(a8_vm_data *data);
bool a8_vm_data_set_align(a8_vm_data *data, uint32_t align);
// Data }}}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // A8_VM_VM_H_
