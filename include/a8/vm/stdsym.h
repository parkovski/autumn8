#ifndef A8_VM_STDSYM_H_
#define A8_VM_STDSYM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef A8_VM_USIZE_T
# define A8_VM_USIZE_T size_t
#endif
#ifndef A8_VM_SSIZE_T
# define A8_VM_SSIZE_T ptrdiff_t
#endif

typedef A8_VM_USIZE_T a8_vm_usize;
typedef A8_VM_SSIZE_T a8_vm_ssize;
typedef A8_VM_USIZE_T a8_vm_symid;
typedef A8_VM_USIZE_T a8_vm_strid;

const a8_vm_usize a8_vm_no_index = (a8_vm_usize)(-1);
const a8_vm_strid a8_vm_strid_empty = 0;

const a8_vm_usize a8_vm_std_align = alignof(a8_vm_usize);

#define a8_vm_align_ceil(T, p, alignval) \
  ((T)((((size_t)(p)) + ((alignval) - 1)) & (-(alignval))))

#define a8_vm_align_floor(T, p, alignval) \
  ((T)((((size_t)(p)) & (-(alignval)))))

#define a8_vm_std_align_ceil(T, p) a8_vm_align_ceil(T, p, a8_vm_std_align)
#define a8_vm_std_align_floor(T, p) a8_vm_align_floor(T, p, a8_vm_std_align)

#define a8_vm_assert_size_aligned(T, a)       \
  static_assert((sizeof(T) & ((a) - 1)) == 0, \
                "Size of '" #T "' does not meet alignment requirement.")

#define a8_vm_assert_size_std_aligned(T) \
  a8_vm_assert_size_aligned(T, a8_vm_std_align)

typedef struct a8_vm_symbol a8_vm_symbol;

typedef struct a8_vm_symbol_base {
  /*
   * ID of the symbol that describes the content of this base. For all symbols,
   * that is, the base header of a8_vm_symbol, this is a8_vm_symid_symbol. For
   * all other bases (sub-bases), this is the ID of that symbol.
   */
  a8_vm_symid meta;

  /*
   * ID of the symbol whose value this base is part of. This value is the same
   * for all bases in a symbol and is used for casting.
   */
  a8_vm_symid self;

  /*
   * Total allocation size of this base, including the base header, the data
   * that follows, and any necessary alignment space in between.
   */
  a8_vm_usize size;
} a8_vm_symbol_base;
a8_vm_assert_size_std_aligned(a8_vm_symbol_base);

struct a8_vm_symbol {
#ifdef __cplusplus
  /*
   * Don't rely on this! It only frees the dynamic base list. If this symbol
   * contains other owned pointers, they will be leaked.
   */
  ~a8_vm_symbol() {
    if (this->dynamic_base) {
      delete [] this->dynamic_base;
    }
  }
#endif

  a8_vm_symbol_base base;
  a8_vm_strid name;
  a8_vm_symid parent;
  uint16_t dynamic_alloc;
  uint16_t dynamic_count;
  uint16_t static_count;
  uint16_t flags; /* reserved */
  /* TODO: Pointers to usize */
  a8_vm_symbol_base **dynamic_base;
};
a8_vm_assert_size_std_aligned(a8_vm_symbol);

typedef struct a8_vm_string {
  /* TODO: Pointers to usize */
  const char *str;
  a8_vm_usize len;
} a8_vm_string;

typedef struct a8_vm_string_map a8_vm_string_map;
typedef struct a8_vm_symbol_table a8_vm_symbol_table;
typedef struct a8_vm_scope a8_vm_scope;
typedef struct a8_vm_stack a8_vm_stack;
typedef struct a8_vm_code_buffer a8_vm_code_buffer;
typedef struct a8_vm_program a8_vm_program;

/*
 * 4 bits is enough to encode up to 8kB in powers of 2. 5 bits can encode
 * up to 2GB.
 */
const uint32_t A8_VM_DATA_ALIGN_BITS = 5;
const uint32_t A8_VM_DATA_MAX_ALIGN_VALUE = (1U << A8_VM_DATA_ALIGN_BITS) - 1;
const uint64_t A8_VM_DATA_MAX_ALIGN = 1U << A8_VM_DATA_MAX_ALIGN_VALUE;

typedef enum a8_vm_data_flag {
  /* The data can be modified. */
  a8_vm_data_flag_mutable         = 0x1,
  /* The data can be moved by a generic memcpy. */
  a8_vm_data_flag_move            = 0x2,
  /* The data can be copied by a generic memcpy. */
  a8_vm_data_flag_copy            = 0x4,
  /* The data does not contain any pointers to itself. */
  a8_vm_data_flag_no_self_link    = 0x8,
  /* The data does not contain any pointers outside itself. */
  a8_vm_data_flag_no_foreign_link = 0x10,
  /* The data does not contain any pointers. */
  a8_vm_data_flag_no_link         = 0x18,
  /* The size of the data is not greater than the size field. */
  a8_vm_data_flag_max_size        = 0x20,
  /* The size of the data is not less than the size field. */
  a8_vm_data_flag_min_size        = 0x40,
  /* The exact size of the data is known. */
  a8_vm_data_flag_const_size      = 0x60,
} a8_vm_data_flag;

typedef struct a8_vm_data {
  a8_vm_usize size;
  uint16_t align : A8_VM_DATA_ALIGN_BITS;
  uint16_t flags : 16 - A8_VM_DATA_ALIGN_BITS;
} a8_vm_data;
a8_vm_assert_size_std_aligned(a8_vm_data);

enum {
  a8_vm_symid_undef,
  a8_vm_symid_symid,
  a8_vm_symid_strid,
  a8_vm_symid_usize,
  a8_vm_symid_ssize,
  a8_vm_symid_symbol,
  a8_vm_symid_symbol_base_header,
  a8_vm_symid_string,
  a8_vm_symid_string_map,
  a8_vm_symid_scope,
  a8_vm_symid_stack,
  a8_vm_symid_code_buffer,
  a8_vm_symid_program,
  a8_vm_symid_data,

  a8_vm_std_symid_count__,
};

#ifdef __cplusplus
} /* extern "C" */

namespace a8::vm {
using symid = a8_vm_symid;
using strid = a8_vm_strid;
using symbol_table = a8_vm_symbol_table;
using symbol = a8_vm_symbol;
using symbol_base = a8_vm_symbol_base;
using string = a8_vm_string;
using string_map = a8_vm_string_map;
using scope = a8_vm_scope;
using stack = a8_vm_stack;
using code_buffer = a8_vm_code_buffer;
using program = a8_vm_program;
using data = a8_vm_data;
} /* namespace a8::vm */
#endif

#endif /* A8_VM_STDSYM_H_ */
