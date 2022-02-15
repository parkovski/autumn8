#include "a8/vm/internal/program.h"

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
