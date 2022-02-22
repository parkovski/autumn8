#include "a8/vm/data.h"

unsigned a8_vm_data_get_align(a8_vm_data *data) {
  return 1 << data->align;
}

void a8_vm_data_set_align(a8_vm_data *data, unsigned align) {
  assert(align > 0 && align <= A8_VM_DATA_MAX_ALIGN);
#if defined(__has_builtin) && __has_builtin(__builtin_ctz)
  assert(align == 1 << __builtin_ctz(align) && "Align must be a power of 2.");
  data->align = (uint32_t)(__builtin_ctz(align));
#else
  for (unsigned i = 0; i <= A8_VM_DATA_MAX_ALIGN_POWER_OF_2; ++i) {
    if (align == (1U << i)) {
      data->align = i;
      return;
    }
  }
  assert(!"Align value must be a power of 2 <= A8_VM_DATA_MAX_ALIGN.");
#endif
}

a8_vm_data_flag a8_vm_data_get_flags(a8_vm_data *data) {
  return (a8_vm_data_flag)data->flags;
}

void a8_vm_data_set_flags(a8_vm_data *data, a8_vm_data_flag flags) {
  data->flags = (uint32_t)flags;
}
