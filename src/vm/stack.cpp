#include "a8/vm/internal/stack.h"

// C API {{{
size_t
a8_vm_stack_alloc_size() {
  return sizeof(a8_vm_stack);
}

a8_vm_stack *
a8_vm_stack_init(a8_vm_stack *stack, a8_vm_usize size) {
  new (stack) a8_vm_stack(size);
  return stack;
}

a8_vm_stack *
a8_vm_stack_destroy(a8_vm_stack *stack) {
  stack->~a8_vm_stack();
  return stack;
}

a8_vm_usize
a8_vm_stack_offset(a8_vm_stack *stack) {
  return stack->offset();
}

void
a8_vm_stack_push_offset(a8_vm_stack *stack) {
  stack->pushoff();
}

void
a8_vm_stack_pop_offset(a8_vm_stack *stack, a8_vm_usize offsetloc) {
  stack->popoff(offsetloc);
}

char *
a8_vm_stack_pointer(a8_vm_stack *stack) {
  return stack->pointer();
}

char *
a8_vm_stack_restore(a8_vm_stack *stack, a8_vm_usize offset) {
  stack->restore(offset);
  return stack->pointer();
}

char *
a8_vm_stack_reserve(a8_vm_stack *stack, a8_vm_usize size, a8_vm_usize align) {
  if (stack->reserve(size, align)) {
    return stack->pointer();
  }
  return nullptr;
}

char *
a8_vm_stack_release(a8_vm_stack *stack, a8_vm_usize size) {
  char *p = stack->pointer();
  return stack->release(size) ? p : nullptr;
}
// C API }}}
