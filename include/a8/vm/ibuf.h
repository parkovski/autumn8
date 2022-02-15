#ifndef A8_VM_IBUF_H_
#define A8_VM_IBUF_H_

#include "stdsym.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct a8_vm_ibuf a8_vm_ibuf;

size_t
a8_vm_ibuf_alloc_size();

a8_vm_ibuf *
a8_vm_ibuf_init(a8_vm_ibuf *buf, a8_vm_usize page_size);

a8_vm_ibuf *
a8_vm_ibuf_destroy(a8_vm_ibuf *buf);

void
a8_vm_ibuf_ldc(a8_vm_ibuf *b, unsigned ofs, unsigned rdst, uint16_t cst);

void
a8_vm_ibuf_uop(a8_vm_ibuf *b, unsigned op, unsigned rdst);

void
a8_vm_ibuf_iop(a8_vm_ibuf *b, unsigned op);

void
a8_vm_ibuf_bop(a8_vm_ibuf *b, unsigned op, unsigned rdst, unsigned rsrc);

void
a8_vm_ibuf_ldx(a8_vm_ibuf *b, bool sx, bool b64, unsigned rdst, uint8_t cst);

void
a8_vm_ibuf_caj(a8_vm_ibuf *b, unsigned cond, unsigned rdst, unsigned rsrc);

void
a8_vm_ibuf_naj(a8_vm_ibuf *b, unsigned rsrc);

void
a8_vm_ibuf_pgj(a8_vm_ibuf *b, unsigned ofs, uint16_t page);

void
a8_vm_ibuf_mov(a8_vm_ibuf *b, unsigned cond, unsigned rdst, unsigned rsrc);

void
a8_vm_ibuf_crj(a8_vm_ibuf *b, unsigned cond, signed ofs, unsigned rsrc);

void
a8_vm_ibuf_nrj(a8_vm_ibuf *b, signed ofs);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* A8_VM_IBUF_H_ */
