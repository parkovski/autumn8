#ifndef A8_VM_STDSYM_H_
#define A8_VM_STDSYM_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * How the metadata works:
 *
 * - Everything is a symbol.
 * - Every symbol has a unique name within its scope, giving it a globally
 *   unique path.
 * - The root scope's name is null.
 * - Paths use the special characters `/` (delimiter), `{}` (subpath), `@:`
 *   (instance value), `\` (escape).
 */

typedef union a8_vm_value {
  char bytes[sizeof(intptr_t)];
  intptr_t i;
  uintptr_t u;
  void *p;
} a8_vm_value;

typedef struct a8_vm_symbol_part {
  uint32_t moff;
  uint32_t voff;
} a8_vm_symbol_part;

const uint32_t A8_VM_SYMBOL_NO_INDEX = (uint32_t)(-1);
typedef struct a8_vm_symbol a8_vm_symbol, *a8_vm_symref;
struct a8_vm_symbol {
  uint32_t count;
  a8_vm_symbol_part parts[1];
};

typedef struct a8_vm_name {
  const char *str;
  size_t len;
} a8_vm_name;

typedef struct a8_vm_scope a8_vm_scope;
typedef struct a8_vm_program a8_vm_program;

// 4 bits is enough to encode up to 8kB in powers of 2. 5 bits can encode
// up to 2GB.
const uint32_t A8_VM_DATA_ALIGN_BITS = 5;
const uint32_t A8_VM_DATA_MAX_ALIGN_VALUE = (1U << A8_VM_DATA_ALIGN_BITS) - 1;
const uint64_t A8_VM_DATA_MAX_ALIGN = 1U << A8_VM_DATA_MAX_ALIGN_VALUE;
typedef struct a8_vm_data {
  size_t size;
  uint32_t align : A8_VM_DATA_ALIGN_BITS;
  uint32_t flags : 32 - A8_VM_DATA_ALIGN_BITS;
} a8_vm_data;

typedef struct a8_vm_function a8_vm_function;
typedef struct a8_vm_code_page a8_vm_code_page;

typedef enum a8_vm_stdsym {
  a8_vm_stdsym_symbol,
  a8_vm_stdsym_symbol_part,
  a8_vm_stdsym_value,
  a8_vm_stdsym_name,
  a8_vm_stdsym_scope,
  a8_vm_stdsym_program,
  a8_vm_stdsym_data,
  a8_vm_stdsym_function,
  a8_vm_stdsym_function_no_params,
  a8_vm_stdsym_code_page,

  a8_vm_stdsym_max__,
} a8_vm_stdsym;

#ifdef __cplusplus
} // extern "C"

namespace a8::vm {
using value = a8_vm_value;
using symbol_part = a8_vm_symbol_part;
using symbol = a8_vm_symbol;
using symref = a8_vm_symref;
using name = a8_vm_name;
using scope = a8_vm_scope;
using program = a8_vm_program;
using data = a8_vm_data;
using stdsym = a8_vm_stdsym;
} // namespace a8::vm
#endif

#endif // A8_VM_STDSYM_H_
