#ifndef A8_VM_EXECUTION_CONTEXT_H_
#define A8_VM_EXECUTION_CONTEXT_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

const a8_vm_usize A8_VM_DEFAULT_STACK_SIZE =
    sizeof(a8_vm_usize) == 8 ? 0xF6000 // 984kB
  : sizeof(a8_vm_usize) == 4 ? 0x7B000 // 492kB
  : sizeof(a8_vm_usize) == 2 ? 0x3D800 // 246kB
  : 0;

const a8_vm_usize a8_vm_reg_fp = 29;
const a8_vm_usize a8_vm_reg_sp = 29;
const a8_vm_usize a8_vm_reg_pc = 29;

typedef struct a8_vm_execution_context {
  a8_vm_usize reg[32];
  char *stk;
  struct a8_vm_symbol_table *symtab;
} a8_vm_execution_context;

void a8_vm_execution_context_symbol_table_init(a8_vm_execution_context *ctx);

void a8_vm_execution_context_run(a8_vm_execution_context *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_EXECUTION_CONTEXT_H_ */
