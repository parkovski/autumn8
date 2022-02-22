#ifndef A8_VM_SYMBOL_H_
#define A8_VM_SYMBOL_H_

#include "stdsym.h"

#ifdef __cplusplus
#include <tuple>

extern "C" {
#endif

/**
 * Type T_s includes a symbol header named 'base' and a T field named 'self'.
 */
#define A8_VM_DEFINE_SYMBOL_TYPE(T) \
  typedef struct T##_s {            \
    a8_vm_symbol base;              \
    T            self;              \
  } T##_s

/**
 * Type T_m includes only a meta_entry header.
 * Type T_s includes a meta_entry_s header, which itself includes a symbol
 * header.
 */
#define A8_VM_DEFINE_META_ENTRY_TYPES(T) \
  typedef struct T##_m {                 \
    a8_vm_meta_entry base;               \
    T self;                              \
  } T##_m;                               \
  typedef struct T##_s {                 \
    a8_vm_meta_entry_s base;             \
    T self;                              \
  } T##_s

/**
 * Everything directly reachable from the symbol table starts with this.
 * The single field points to the meta symbol that describes this object.
 */
typedef struct a8_vm_symbol {
  /*
   * ID of the meta symbol that describes the content of this symbol.
   */
  a8_vm_symid meta;
} a8_vm_symbol;

#define a8_vm_symbol_value(T, psym) a8_vm_struct_after(T, a8_vm_symbol, psym)

/**
 * Every meta object contains one or more of this header in addition to the
 * symbol header.
 */
typedef struct a8_vm_meta_entry {
  /*
   * ID of the symbol whose value this entry is part of. This value is the same
   * for all entries in a symbol and is used for casting.
   */
  a8_vm_symid id;
} a8_vm_meta_entry;

A8_VM_DEFINE_SYMBOL_TYPE(a8_vm_meta_entry);

#define a8_vm_meta_entry_value(T, pentry) \
  a8_vm_struct_after(T, a8_vm_meta_entry, pentry)

#define a8_vm_meta_entry_s_value(T, pentry) \
  a8_vm_struct_after(T, a8_vm_meta_entry_s, pentry)

typedef struct a8_vm_meta_object {
  a8_vm_strid name;
  a8_vm_symid parent;
  uint16_t flags; /* reserved */
  uint16_t dynamic_alloc;
  uint16_t dynamic_count;
  uint16_t static_count;

  /**
   * Dynamic entries are stored as a mutable array in entries.
   * Static entries are stored inline in the struct, following the entries
   * array. So, dynamic entry n is located at *entries[n] (aka *(*entries + n))
   * and static entry n is located at (&entries)[1 + n]. It looks like this:
   *
   * Meta object: [
   *   Meta entry: [
   *     Symbol header [ meta id ]
   *     self id
   *   ]
   *   meta object fields
   *   pointer to dynamic entry list -> &D0, ..., &DN
   *   pointer to static entry 0     -> &S0
   *   ...
   *   pointer to static entry n     -> &SN
   *   static entry 0                -> S0
   *   ...
   *   static entry n                -> SN
   * ]
   */
  a8_vm_meta_entry_s **entries;
} a8_vm_meta_object;

#define a8_vm_meta_object_dynamic_entry(pobj, n) (*(pobj)->entries)[n]
#define a8_vm_meta_object_static_entry(pobj, n) \
  ((a8_vm_meta_entry_s *)(void *)((pobj)->entries))[1 + (n)]

A8_VM_DEFINE_META_ENTRY_TYPES(a8_vm_meta_object);

/**
 * Gets a pointer to the symbol header for the symbol identified by `id`, or
 * null if `id` is not a valid symbol ID.
 */
a8_vm_symbol *
a8_vm_symbol_get(a8_vm_symid id, const a8_vm_symbol_table *symtab);

/**
 * TODO: Alignment.
 * Gets the post-header value of the symbol identified by `id`, or null if
 * `id` is not a valid symbol ID. This is less error-prone than using
 * a8_vm_symbol_value(T, a8_vm_symbol_get(...)), since it will assume the
 * pointer is not null and return an offset from null for an undefined symbol.
 */
/*
void *
a8_vm_symbol_get_value(a8_vm_symid id, const a8_vm_symbol_table *symtab);
*/

/**
 * Returns the meta object for the symbol identified by `id`. If the symbol's
 * meta ID is a8_symid_undef, returns null.
 */
a8_vm_meta_object_s *
a8_vm_symbol_get_meta_object(a8_vm_symid id, const a8_vm_symbol_table *symtab);

/**
 * Returns the size of a a8_vm_meta_object_m - including the meta entry header,
 * but _not_ the symbol header since the symbol table adds that automatically.
 * The returned size includes space for the entries in `ids`,
 * alignment/padding, and the static entry table.
 */
a8_vm_usize
a8_vm_meta_object_size(const a8_vm_symid *eids, a8_vm_usize count,
                       const a8_vm_symbol_table *symtab);

/**
 * Tries to cast `m` to the entry of type `eid`. If the entry doesn't exist,
 * returns null.
 */
a8_vm_meta_entry_s *
a8_vm_meta_object_cast(a8_vm_meta_object *m, a8_vm_symid eid);

/**
 * Tries to cast `m` to the entry of type `eid`. If the entry does not exist,
 * tries to add it as a dynamic entry. If the entry couldn't be added,
 * returns null.
 */
a8_vm_meta_entry_s *
a8_vm_meta_object_cast_append(a8_vm_meta_object *m, a8_vm_symid eid,
                              const a8_vm_symbol_table *symtab);

/*
 * TODO: Reporting failures?
 */
/*
a8_vm_usize
a8_vm_meta_object_append_all(a8_vm_meta_object *m, const a8_vm_symid *eids,
                             a8_vm_usize count,
                             const a8_vm_symbol_table *symtab);
*/

/*
 * Returns the meta entry for `eid` in the meta object of the symbol for `id`.
 * If the symbol has no meta object or the meta object doesn't have the
 * specified entry, returns null.
 */
a8_vm_meta_entry_s *
a8_vm_symbol_get_meta_entry(a8_vm_symid id, a8_vm_symid eid,
                            const a8_vm_symbol_table *symtab);

/**
 * Tries to cast `m` to the entry of type `id`. If the entry doesn't exist,
 * returns null.
 */
a8_vm_meta_entry_s *
a8_vm_meta_entry_cast(a8_vm_meta_entry *e, a8_vm_symid eid,
                      const a8_vm_symbol_table *symtab);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_SYMBOL_H_ */
