#ifndef A8_VM_STACK_H_
#define A8_VM_STACK_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct a8_vm_stack a8_vm_stack;

size_t
a8_vm_stack_alloc_size();

a8_vm_stack *
a8_vm_stack_init(a8_vm_stack *stack, a8_vm_usize size);

a8_vm_stack *
a8_vm_stack_destroy(a8_vm_stack *stack);

a8_vm_usize
a8_vm_stack_offset(a8_vm_stack *stack);

void
a8_vm_stack_push_offset(a8_vm_stack *stack);

void
a8_vm_stack_pop_offset(a8_vm_stack *stack, a8_vm_usize offsetloc);

char *
a8_vm_stack_pointer(a8_vm_stack *stack);

char *
a8_vm_stack_restore(a8_vm_stack *stack, a8_vm_usize offset);

char *
a8_vm_stack_reserve(a8_vm_stack *stack, a8_vm_usize size, a8_vm_usize align);

char *
a8_vm_stack_release(a8_vm_stack *stack, a8_vm_usize size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_STACK_H_ */
