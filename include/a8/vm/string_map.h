#ifndef A8_VM_STRING_MAP_H_
#define A8_VM_STRING_MAP_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct a8_vm_string_map a8_vm_string_map;

size_t
a8_vm_string_map_alloc_size();

a8_vm_string_map *
a8_vm_string_map_init(a8_vm_string_map *map);

a8_vm_string_map *
a8_vm_string_map_destroy(a8_vm_string_map *map);

/**
 * If length is specified, the string does not need to be null terminated.
 */
a8_vm_strid
a8_vm_string_map_get_id(a8_vm_string_map *map, const char *s, size_t length);

/**
 * If length is specified, the string does not need to be null terminated.
 */
a8_vm_string
a8_vm_string_map_get_string(a8_vm_string_map *map, const char *s,
                            size_t length);

a8_vm_string
a8_vm_string_map_get_string_at(a8_vm_string_map *map, a8_vm_strid id);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_STRING_MAP_H_ */
