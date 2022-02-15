#include "a8/vm/instr.h"
#include "a8/vm/internal/ibuf.h"

namespace {
  constexpr bool is_ldc_offset(unsigned ofs) {
    return ofs < 4;
  }

  constexpr bool is_reg(unsigned r) {
    return r <= 0xF;
  }

  static_assert(a8_vm_uop_count_ <= 0x100);
  constexpr bool is_uop(unsigned op) {
    return op < a8_vm_uop_count_;
  }

  static_assert(a8_vm_iop_count_ <= 0x100);
  constexpr bool is_iop(unsigned op) {
    return op < a8_vm_iop_count_;
  }

  static_assert(a8_vm_bop_count_ <= 0x40);
  constexpr bool is_bop(unsigned op) {
    return op < a8_vm_bop_count_;
  }

  constexpr bool is_cond(unsigned op, bool allow_always) {
    if (!allow_always && op == a8_vm_cond_always) {
      return false;
    }
    return op < a8_vm_cond_count_;
  }

  constexpr bool is_page_offset(unsigned ofs) {
    return ofs <= 0x3F;
  }

  constexpr bool is_rel_jmp_offset(signed ofs) {
    return ofs >= -0x20 && ofs <= 0x1F;
  }
}

a8_vm_ibuf::a8_vm_ibuf(a8_vm_usize pageSize)
  : _workBuf{std::make_unique<uint16_t[]>(WorkSize)},
    _workOffset{0},
    _pageSize{pageSize >> 1},
    _pageOffset{0}
{
  addPage();
}

void write(uint16_t *insn, a8_vm_usize count) {
}

// C API {{{
size_t
a8_vm_ibuf_alloc_size() {
  return sizeof(a8_vm_ibuf);
}

a8_vm_ibuf *
a8_vm_ibuf_init(a8_vm_ibuf *buf, a8_vm_usize page_size) {
  new (buf) a8_vm_ibuf(page_size);
  return buf;
}

a8_vm_ibuf *
a8_vm_ibuf_destroy(a8_vm_ibuf *buf) {
  buf->~a8_vm_ibuf();
  return buf;
}

void
a8_vm_ibuf_ldc(a8_vm_ibuf *b, unsigned ofs, unsigned rdst, uint16_t cst) {
  assert(is_ldc_offset(ofs));
  assert(is_reg(rdst));

  uint16_t insns[2] = {
    a8_vm_instr_form_0_0(ofs, rdst),
    cst,
  };
  b->write(insns, 2);
}

void
a8_vm_ibuf_uop(a8_vm_ibuf *b, unsigned op, unsigned rdst) {
  assert(is_uop(op));
  assert(is_reg(rdst));

  b->write(a8_vm_instr_form_0_1(op, rdst));
}

void
a8_vm_ibuf_iop(a8_vm_ibuf *b, unsigned op) {
  assert(is_iop(op));

  b->write(a8_vm_instr_form_0_2(op));
}

void
a8_vm_ibuf_bop(a8_vm_ibuf *b, unsigned op, unsigned rdst, unsigned rsrc) {
  assert(is_bop(op));
  assert(is_reg(rdst));
  assert(is_reg(rsrc));

  b->write(a8_vm_instr_form_1(op, rdst, rsrc));
}

void
a8_vm_ibuf_ldx(a8_vm_ibuf *b, bool sx, bool b64, unsigned rdst, uint8_t cst) {
  assert(is_reg(rdst));

  b->write(a8_vm_instr_form_2((unsigned)sx, (unsigned)b64, cst, rdst));
}

void
a8_vm_ibuf_caj(a8_vm_ibuf *b, unsigned cond, unsigned rdst, unsigned rsrc) {
  assert(is_cond(cond, false));
  assert(is_reg(rdst));
  assert(is_reg(rsrc));

  b->write(a8_vm_instr_form_3_0_0(cond, rdst, rsrc));
}

void
a8_vm_ibuf_naj(a8_vm_ibuf *b, unsigned rsrc) {
  assert(is_reg(rsrc));

  b->write(a8_vm_instr_form_3_0_1(rsrc));
}

void
a8_vm_ibuf_pgj(a8_vm_ibuf *b, unsigned ofs, uint16_t page) {
  assert(is_page_offset(ofs));

  uint16_t insns[2] = {
    a8_vm_instr_form_3_0_2(ofs),
    page,
  };
  b->write(insns, 2);
}

void
a8_vm_ibuf_mov(a8_vm_ibuf *b, unsigned cond, unsigned rdst, unsigned rsrc) {
  assert(is_cond(cond, true));
  assert(is_reg(rdst));
  assert(is_reg(rsrc));

  b->write(a8_vm_instr_form_3_1(cond, rdst, rsrc));
}

void
a8_vm_ibuf_crj(a8_vm_ibuf *b, unsigned cond, signed ofs, unsigned rsrc) {
  assert(is_cond(cond, false));
  assert(is_rel_jmp_offset(ofs));
  assert(is_reg(rsrc));

  unsigned uofs = static_cast<unsigned>(ofs) & 0x3FU;
  b->write(a8_vm_instr_form_4_0(cond, uofs, rsrc));
}

void
a8_vm_ibuf_nrj(a8_vm_ibuf *b, signed ofs) {
  assert(is_rel_jmp_offset(ofs));

  unsigned uofs = static_cast<unsigned>(ofs) & 0x3FU;
  b->write(a8_vm_instr_form_4_1(uofs));
}
// C API }}}
