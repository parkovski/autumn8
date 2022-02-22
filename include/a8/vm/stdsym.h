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

typedef a8_vm_usize a8_vm_symid;
typedef a8_vm_usize a8_vm_strid;

/** Invalid index (-1). */
const a8_vm_usize a8_vm_no_index = (a8_vm_usize)(-1);

/**
 * A symbol ID that does not exist. This should be used for lookup failures
 * and not as a placeholder - for that, use `empty`.
 */
const a8_vm_symid a8_vm_symid_undef = (a8_vm_symid)(-1);

/**
 * The empty string. Since interned strings are null terminated, this is a
 * pointer to the null character.
 */
const a8_vm_strid a8_vm_strid_empty = 0;

const a8_vm_usize a8_vm_std_align = alignof(a8_vm_usize);

#define a8_vm_align_ceil(T, p, alignval) \
  ((T)((((size_t)(p)) + ((alignval) - 1)) & (-(alignval))))

#define a8_vm_align_floor(T, p, alignval) \
  ((T)((((size_t)(p)) & (-(alignval)))))

#define a8_vm_const_struct_after(T, Struct, p) \
  a8_vm_align_ceil(T const *, ((char const *)(p)) + sizeof(Struct), alignof(T))

#define a8_vm_struct_after(T, Struct, p) \
  a8_vm_align_ceil(T *, ((char *)(p)) + sizeof(Struct), alignof(T))

#define a8_vm_std_align_ceil(T, p) a8_vm_align_ceil(T, p, a8_vm_std_align)
#define a8_vm_std_align_floor(T, p) a8_vm_align_floor(T, p, a8_vm_std_align)

#define a8_vm_assert_size_aligned(T, a)       \
  static_assert((sizeof(T) & ((a) - 1)) == 0, \
                "Size of '" #T "' does not meet alignment requirement.")

#define a8_vm_assert_size_std_aligned(T) \
  a8_vm_assert_size_aligned(T, a8_vm_std_align)

typedef struct a8_vm_string {
  /* TODO: Pointers to usize */
  const char *str;
  a8_vm_usize len;
} a8_vm_string;

typedef struct a8_vm_symbol a8_vm_symbol;
typedef struct a8_vm_meta_entry a8_vm_meta_entry;
typedef struct a8_vm_meta_entry_s a8_vm_meta_entry_s;
typedef struct a8_vm_meta_object a8_vm_meta_object;
typedef struct a8_vm_meta_object_m a8_vm_meta_object_m;
typedef struct a8_vm_meta_object_s a8_vm_meta_object_s;
typedef struct a8_vm_symbol_table a8_vm_symbol_table;
typedef struct a8_vm_symbol_table_s a8_vm_symbol_table_s;

enum {
  /*
   * Zero size symbol. In contrast to `undef` which represents a name that does
   * not exist, `empty` is a name that exists but has no value.
   */
  a8_vm_symid_empty,
  /*
   * "Black box" type. More abstract than a `void*` - it's not known whether a
   * value exists here and if it does, its size and alignment are unknown, so
   * unlike `void*`, any operations at all including casting this to another
   * type are undefined.
   */
  a8_vm_symid_unknown,
  /* The always-error type. */
  a8_vm_symid_never,

  /* Symbolic root of the VM. */
  a8_vm_symid_root,

  /* Core meta symbols. */
  a8_vm_symid_meta,
  /* Data-related builtins. */
  a8_vm_symid_data,
  /* Type construction utilities. */
  a8_vm_symid_type,
  /* Execution-related builtins. */
  a8_vm_symid_exec,
  /* Execution environments. */
  a8_vm_symid_env,

  a8_vm_symid_symid,
  a8_vm_symid_strid,
  a8_vm_symid_symbol,
  a8_vm_symid_meta_entry,
  a8_vm_symid_meta_object,
  a8_vm_symid_meta_ctor,
  a8_vm_symid_meta_dtor,

  a8_vm_symid_bit,
  a8_vm_symid_byte,
  a8_vm_symid_word,
  a8_vm_symid_string,
  a8_vm_symid_address,
  a8_vm_symid_bool,
  a8_vm_symid_true,
  a8_vm_symid_false,
  a8_vm_symid_num,
  a8_vm_symid_int,
  a8_vm_symid_sign2c,
  a8_vm_symid_lossy,
  a8_vm_symid_ieee_float,

  a8_vm_symid_type_field_set,
  /*  T = U | V
   *  T = U (+{+)+} V
   * ::  all t --> t < U || t < V
   * && some t --> t < U && t < V
   * &&  all u --> u < T
   * &&  all v --> v < T. */
  a8_vm_symid_type_union,
  /*  T = U & V
   *  T = U ( {+) } V
   * ::  all t --> t < U && t < V
   * && some u --> u < T
   * && some v --> v < T. */
  a8_vm_symid_type_intersection,
  /*  T = U + V
   *  T = U (+{*)+} V
   *  * When overlapping values are present, a type tag is required.
   * ::  all t --> t < U || t < V
   * &&  all u --> u < T
   * &&  all v --> v < T. */
  a8_vm_symid_type_sum,
  /*  T = U \ V (U - V)
   *  T = U (+{ ) } V
   * ::  all t --> t < U
   * && some u --> u < T. */
  a8_vm_symid_type_difference,
  /*  T = U x V (U * V)
   *  T = (U (+), V (+))
   * ::  all t --> t < U
   * &&  all u --> all v --> (u, v) < T. */
  a8_vm_symid_type_product,
  /* T = U ? V :: T < U || T < V */

  a8_vm_symid_text,
  a8_vm_symid_abi,
  a8_vm_symid_function,
  a8_vm_symid_memo,

  /* Host machine ("outside world") environment. */
  a8_vm_symid_env_host,
  /* Pure introspection environment. */
  a8_vm_symid_env_meta,
  /* VM direct control environment. */
  a8_vm_symid_env_control,

  a8_vm_symid_symbol_table,
  /*a8_vm_symid_constant_table,*/
  a8_vm_symid_string_map,
  a8_vm_symid_stack,
  a8_vm_symid_ibuf,
  a8_vm_symid_program,

  a8_vm_std_symid_count__,
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_STDSYM_H_ */
