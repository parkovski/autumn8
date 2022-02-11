#ifndef A8_VM_VM_H_
#define A8_VM_VM_H_

#include "stdsym.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// String map {{{
size_t            a8_vm_string_map_alloc_size();
a8_vm_string_map *a8_vm_string_map_init(a8_vm_string_map *map);
a8_vm_string_map *a8_vm_string_map_destroy(a8_vm_string_map *map);

a8_vm_strid       a8_vm_string_map_get_id(a8_vm_string_map *map,
                                          const char *s, size_t length);
a8_vm_string      a8_vm_string_map_get_string(a8_vm_string_map *map,
                                              const char *s, size_t length);
a8_vm_string      a8_vm_string_map_get_string_at(a8_vm_string_map *map,
                                                 a8_vm_strid id);
// String map }}}

// Symbol table {{{
size_t               a8_vm_symbol_table_alloc_size();
a8_vm_symbol_table * a8_vm_symbol_table_init(a8_vm_symbol_table *symtab);
a8_vm_symbol_table * a8_vm_symbol_table_destroy(a8_vm_symbol_table *symtab);

a8_vm_symbol *       a8_vm_symbol_table_get_symbol(a8_vm_symbol_table *symtab,
                                                   a8_vm_symid symid);
a8_vm_symbol_base *  a8_vm_symbol_table_get_symbol_base(a8_vm_symbol_table *st,
                                                        a8_vm_symid symid,
                                                        a8_vm_symid base);
a8_vm_usize          a8_vm_symbol_table_get_size(a8_vm_symbol_table *symtab,
                                                 a8_vm_symid *ids,
                                                 a8_vm_usize count);
a8_vm_symid          a8_vm_symbol_table_add_symbol(a8_vm_symbol_table *symtab,
                                                   a8_vm_symbol *symbol,
                                                   a8_vm_usize size,
                                                   a8_vm_symid *ids,
                                                   a8_vm_usize count);
a8_vm_symid          a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab,
                                                   a8_vm_symid *ids,
                                                   a8_vm_usize count);

a8_vm_symbol_base *  a8_vm_symbol_cast(a8_vm_symbol *symbol, a8_vm_symid base);
a8_vm_symbol_base *  a8_vm_symbol_ensure_cast(a8_vm_symbol *symbol,
                                              a8_vm_symbol_table *symtab,
                                              a8_vm_symid base);
a8_vm_usize          a8_vm_symbol_ensure(a8_vm_symbol *symbol,
                                         a8_vm_symbol_table *symtab,
                                         a8_vm_symid *bases,
                                         a8_vm_usize count);

#define              a8_vm_symbol_base_value(T, base) \
  a8_vm_align_ceil(T*, ((char*)(base)) + sizeof(a8_vm_symbol_base), alignof(T))
// Symbol table }}}

// Scope {{{
size_t       a8_vm_scope_alloc_size();
a8_vm_scope *a8_vm_scope_init(a8_vm_scope *scope);
a8_vm_scope *a8_vm_scope_destroy(a8_vm_scope *scope);

a8_vm_usize  a8_vm_scope_get_child_count(a8_vm_scope *scope);
a8_vm_symid  a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_strid name);
a8_vm_symid  a8_vm_scope_get_child_at(a8_vm_scope *scope, a8_vm_usize index);
a8_vm_usize  a8_vm_scope_get_child_index(a8_vm_scope *scope, a8_vm_strid name);
a8_vm_usize  a8_vm_scope_put_name(a8_vm_scope *scope, a8_vm_strid name);
a8_vm_usize  a8_vm_scope_set_child(a8_vm_scope *scope, a8_vm_strid name,
                                   a8_vm_symid symbol);
a8_vm_symid  a8_vm_scope_set_child_at(a8_vm_scope *scope, a8_vm_usize index,
                                      a8_vm_symid symbol);
// Scope }}}

// Stack {{{
size_t       a8_vm_stack_alloc_size();
a8_vm_stack *a8_vm_stack_init(a8_vm_stack *stack, a8_vm_usize size);
a8_vm_stack *a8_vm_stack_destroy(a8_vm_stack *stack);
a8_vm_usize  a8_vm_stack_offset(a8_vm_stack *stack);
void         a8_vm_stack_push_offset(a8_vm_stack *stack);
void         a8_vm_stack_pop_offset(a8_vm_stack *stack, a8_vm_usize offsetloc);
char *       a8_vm_stack_pointer(a8_vm_stack *stack);
char *       a8_vm_stack_restore(a8_vm_stack *stack, a8_vm_usize offset);
char *       a8_vm_stack_reserve(a8_vm_stack *stack, a8_vm_usize size,
                                 a8_vm_usize align);
char *       a8_vm_stack_release(a8_vm_stack *stack, a8_vm_usize size);
// Stack }}}

// Code buffer {{{
size_t             a8_vm_code_buffer_alloc_size();
a8_vm_code_buffer *a8_vm_code_buffer_init(a8_vm_code_buffer *buf,
                                          a8_vm_usize page_size);
a8_vm_code_buffer *a8_vm_code_buffer_destroy(a8_vm_code_buffer *buf);
// Code buffer }}}

// Program {{{
size_t             a8_vm_program_alloc_size();
a8_vm_program *    a8_vm_program_init(a8_vm_program *program);
a8_vm_program *    a8_vm_program_destroy(a8_vm_program *program);

size_t             a8_vm_program_get_string_id(a8_vm_program *program,
                                               const char *s, size_t length);
a8_vm_string       a8_vm_program_get_string(a8_vm_program *program,
                                            const char *s, size_t length);
a8_vm_string       a8_vm_program_get_string_at(a8_vm_program *p, size_t index);

a8_vm_scope *      a8_vm_program_get_root_scope(a8_vm_program *program);
a8_vm_code_buffer *a8_vm_program_get_code_buffer(a8_vm_program *program);
a8_vm_stack *      a8_vm_program_get_stack(a8_vm_program *program);
// Program }}}

// Data {{{
uint32_t a8_vm_data_get_align(a8_vm_data *data);
bool     a8_vm_data_set_align(a8_vm_data *data, uint32_t align);
// Data }}}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_VM_H_ */
