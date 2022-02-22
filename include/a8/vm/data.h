#ifndef A8_VM_DATA_H_
#define A8_VM_DATA_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 4 bits is enough to encode up to 8kB in powers of 2. 5 bits can encode
 * up to 2GB.
 */
const unsigned A8_VM_DATA_ALIGN_BITS = 5;
const unsigned A8_VM_DATA_MAX_ALIGN_POWER_OF_2 =
  (1U << A8_VM_DATA_ALIGN_BITS) - 1;
const unsigned A8_VM_DATA_MAX_ALIGN = 1U << A8_VM_DATA_MAX_ALIGN_POWER_OF_2;

const unsigned A8_VM_DATA_FLAGS_BITS = 32 - A8_VM_DATA_ALIGN_BITS;
const unsigned A8_VM_DATA_FLAGS_MAX_FLAG = 1U << (A8_VM_DATA_FLAGS_BITS - 1);
const unsigned A8_VM_DATA_FLAGS_MAX_VALUE =
  ~(-2U << (A8_VM_DATA_FLAGS_BITS - 1));

typedef enum a8_vm_data_flag {
  /* The data can be modified. */
  a8_vm_data_flag_mutable         = 0x1,
  /* The data can be moved by a generic memcpy. */
  a8_vm_data_flag_move            = 0x2,
  /* The data can be copied by a generic memcpy. */
  a8_vm_data_flag_copy            = 0x4,
  /* The data can be swapped with a generic algorithm. */
  a8_vm_data_flag_swap            = 0x8,
  /* The data does not contain any pointers to itself. */
  a8_vm_data_flag_no_self_link    = 0x10,
  /* The data does not contain any pointers outside itself. */
  a8_vm_data_flag_no_foreign_link = 0x20,
  /* The data does not contain any pointers. */
  a8_vm_data_flag_no_link         =
    a8_vm_data_flag_no_self_link | a8_vm_data_flag_no_foreign_link,
  /* The size of the data is not greater than the size field. */
  a8_vm_data_flag_max_size        = 0x40,
  /* The size of the data is not less than the size field. */
  a8_vm_data_flag_min_size        = 0x80,
  /* The exact size of the data is known. */
  a8_vm_data_flag_const_size      =
    a8_vm_data_flag_min_size | a8_vm_data_flag_max_size,
  /* Ok to reference this data from other threads. */
  a8_vm_data_thread_safe          = 0x100,
  /* Ok to move/copy this data to another thread. */
  a8_vm_data_thread_safe_copy     = 0x200,
} a8_vm_data_flag;

typedef struct a8_vm_data {
  a8_vm_usize size;
  uint32_t align : A8_VM_DATA_ALIGN_BITS;
  uint32_t flags : A8_VM_DATA_FLAGS_BITS;
} a8_vm_data;
a8_vm_assert_size_std_aligned(a8_vm_data);

unsigned a8_vm_data_get_align(a8_vm_data *data);
void a8_vm_data_set_align(a8_vm_data *data, unsigned align);
a8_vm_data_flag a8_vm_data_get_flags(a8_vm_data *data);
void a8_vm_data_set_flags(a8_vm_data *data, a8_vm_data_flag flags);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_DATA_H_ */
