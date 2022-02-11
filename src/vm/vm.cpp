#include "program.h"

// String map {{{
size_t a8_vm_string_map_alloc_size() {
  return sizeof(a8_vm_string_map);
}

a8_vm_string_map *a8_vm_string_map_init(a8_vm_string_map *map) {
  new (map) a8_vm_string_map;
  return map;
}

a8_vm_string_map *a8_vm_string_map_destroy(a8_vm_string_map *map) {
  map->~a8_vm_string_map();
  return map;
}

a8_vm_strid a8_vm_string_map_get_id(a8_vm_string_map *map,
                                    const char *s, size_t length) {
  std::string_view sv;
  if (length) {
    sv = {s, length};
  } else {
    sv = s;
  }
  return (*map)[sv];
}

a8_vm_string a8_vm_string_map_get_string(a8_vm_string_map *map,
                                         const char *s, size_t length) {
  std::string_view sv;
  if (length) {
    sv = {s, length};
  } else {
    sv = s;
  }
  return map->string(sv);
}

a8_vm_string a8_vm_string_map_get_string_at(a8_vm_string_map *map,
                                            a8_vm_strid id) {
  return map->string(id);
}
// String map }}}

// Symbol table {{{
size_t a8_vm_symbol_table_alloc_size() {
  return sizeof(a8_vm_symbol_table);
}

a8_vm_symbol_table *a8_vm_symbol_table_init(a8_vm_symbol_table *symtab) {
  new (symtab) a8_vm_symbol_table;
  return symtab;
}

a8_vm_symbol_table *a8_vm_symbol_table_destroy(a8_vm_symbol_table *symtab) {
  symtab->~a8_vm_symbol_table();
  return symtab;
}

a8_vm_symbol *a8_vm_symbol_table_get_symbol(a8_vm_symbol_table *symtab,
                                            a8_vm_symid symid) {
  return symtab->_symbols[symid].get();
}

a8_vm_symbol_base *a8_vm_symbol_table_get_symbol_base(a8_vm_symbol_table *st,
                                                      a8_vm_symid symid,
                                                      a8_vm_symid base) {
  return a8_vm_symbol_cast(a8_vm_symbol_table_get_symbol(st, symid), base);
}

a8_vm_usize a8_vm_symbol_table_get_size(a8_vm_symbol_table *symtab,
                                        a8_vm_symid *ids, a8_vm_usize count) {
  return symtab->base_size(ids, count);
}

a8_vm_symid a8_vm_symbol_table_add_symbol(a8_vm_symbol_table *symtab,
                                          a8_vm_symbol *symbol,
                                          a8_vm_usize size,
                                          a8_vm_symid *ids,
                                          a8_vm_usize count) {
  return symtab->add(symbol, size, ids, count);
}

a8_vm_symid a8_vm_symbol_table_new_symbol(a8_vm_symbol_table *symtab,
                                          a8_vm_symid *ids,
                                          a8_vm_usize count) {
  return symtab->add(ids, count);
}

a8_vm_symbol_base *a8_vm_symbol_cast(a8_vm_symbol *symbol, a8_vm_symid base) {
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

a8_vm_symbol_base *a8_vm_symbol_ensure_cast(a8_vm_symbol *symbol,
                                            a8_vm_symbol_table *symtab,
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

a8_vm_usize a8_vm_symbol_ensure(a8_vm_symbol *symbol,
                                a8_vm_symbol_table *symtab,
                                a8_vm_symid *bases,
                                a8_vm_usize count) {
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
// Symbol table }}}

// Scope {{{
size_t a8_vm_scope_alloc_size() {
  return sizeof(a8_vm_scope);
}

a8_vm_scope *a8_vm_scope_init(a8_vm_scope *scope) {
  new (scope) a8_vm_scope;
  return scope;
}

a8_vm_scope *a8_vm_scope_destroy(a8_vm_scope *scope) {
  scope->~a8_vm_scope();
  return scope;
}

a8_vm_usize a8_vm_scope_get_child_count(a8_vm_scope *scope) {
  return static_cast<a8_vm_usize>(scope->size());
}

a8_vm_symid a8_vm_scope_get_child(a8_vm_scope *scope, a8_vm_strid name) {
  return scope->get_by_name(name);
}

a8_vm_symid  a8_vm_scope_get_child_at(a8_vm_scope *scope, a8_vm_usize index) {
  return scope->get_by_index(index);
}

a8_vm_usize  a8_vm_scope_get_child_index(a8_vm_scope *scope,
                                         a8_vm_strid name) {
  return scope->get_index_for_name(name);
}

a8_vm_usize a8_vm_scope_put_name(a8_vm_scope *scope, a8_vm_strid name) {
  return scope->put_name(name);
}

a8_vm_usize  a8_vm_scope_set_child(a8_vm_scope *scope, a8_vm_strid name,
                                   a8_vm_symid symbol) {
  return scope->set_name(name, symbol);
}

a8_vm_symid  a8_vm_scope_set_child_at(a8_vm_scope *scope, a8_vm_usize index,
                                      a8_vm_symid symbol) {
  return scope->set_index(index, symbol);
}
// Scope }}}

// Stack {{{
size_t a8_vm_stack_alloc_size() {
  return sizeof(a8_vm_stack);
}

a8_vm_stack *a8_vm_stack_init(a8_vm_stack *stack, a8_vm_usize size) {
  new (stack) a8_vm_stack(size);
  return stack;
}

a8_vm_stack *a8_vm_stack_destroy(a8_vm_stack *stack) {
  stack->~a8_vm_stack();
  return stack;
}

a8_vm_usize a8_vm_stack_offset(a8_vm_stack *stack) {
  return stack->offset();
}

void a8_vm_stack_push_offset(a8_vm_stack *stack) {
  stack->pushoff();
}

void a8_vm_stack_pop_offset(a8_vm_stack *stack, a8_vm_usize offsetloc) {
  stack->popoff(offsetloc);
}

char *a8_vm_stack_pointer(a8_vm_stack *stack) {
  return stack->pointer();
}

char *a8_vm_stack_restore(a8_vm_stack *stack, a8_vm_usize offset) {
  stack->restore(offset);
  return stack->pointer();
}

char *a8_vm_stack_reserve(a8_vm_stack *stack, a8_vm_usize size,
                          a8_vm_usize align) {
  if (stack->reserve(size, align)) {
    return stack->pointer();
  }
  return nullptr;
}

char *a8_vm_stack_release(a8_vm_stack *stack, a8_vm_usize size) {
  char *p = stack->pointer();
  return stack->release(size) ? p : nullptr;
}
// Stack }}}

// Code buffer {{{
size_t a8_vm_code_buffer_alloc_size() {
  return sizeof(a8_vm_code_buffer);
}

a8_vm_code_buffer *a8_vm_code_buffer_init(a8_vm_code_buffer *buf,
                                          a8_vm_usize page_size) {
  new (buf) a8_vm_code_buffer(page_size);
  return buf;
}

a8_vm_code_buffer *a8_vm_code_buffer_destroy(a8_vm_code_buffer *buf) {
  buf->~a8_vm_code_buffer();
  return buf;
}
// Code buffer }}}

// Program {{{
size_t a8_vm_program_alloc_size() {
  return sizeof(a8_vm_program);
}

a8_vm_program *a8_vm_program_init(a8_vm_program *program) {
  new (program) a8_vm_program();
  return program;
}

a8_vm_program *a8_vm_program_destroy(a8_vm_program *program) {
  program->~a8_vm_program();
  return program;
}

a8_vm_strid a8_vm_program_get_string_id(a8_vm_program *program,
                                        const char *s, size_t length) {
  return a8_vm_string_map_get_id(&program->_strings, s, length);
}

a8_vm_string a8_vm_program_get_string(a8_vm_program *program, const char *s,
                                      size_t length) {
  return a8_vm_string_map_get_string(&program->_strings, s, length);
}

a8_vm_string a8_vm_program_get_string_at(a8_vm_program *program,
                                         a8_vm_strid id) {
  return a8_vm_string_map_get_string_at(&program->_strings, id);
}

a8_vm_scope *a8_vm_program_get_root_scope(a8_vm_program *program) {
  return &program->_root;
}
// Program }}}

// Data {{{
uint32_t a8_vm_data_get_align(a8_vm_data *data) {
  return 1 << data->align;
}

bool a8_vm_data_set_align(a8_vm_data *data, uint32_t align) {
  for (uint32_t i = 0; i <= A8_VM_DATA_MAX_ALIGN_VALUE; ++i) {
    if (align == (1 << i)) {
      data->align = i;
      return true;
    }
  }
  return false;
}
// Data }}}
