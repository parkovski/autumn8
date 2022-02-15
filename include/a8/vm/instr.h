#ifndef A8_VM_INSTR_H_
#define A8_VM_INSTR_H_

/*
 * Categories:
 * - No op
 * - Load/store:
 *   - Move reg->reg
 *   - Load constant->reg
 *   - Load indirect address->reg
 *   - Store reg->address
 * - Stack:
 *   - Push register
 *   - Pop register
 *   - Get stack offset
 *   - Set stack offset
 *   - Align stack
 * - Special/strings:
 *   - String -> strid
 *   - String -> interned string
 *   - strid -> interned string
 * - Special/symbol:
 *   - New symbol
 *   - symid -> symbol pointer
 *   - Cast symbol +/-ensure
 * - Special/scope:
 *   - Resolve name +/-ensure
 *   - Resolve path (name chain)
 * - Integers:
 *   - Add/sub (overflow/wrapping?)
 *   - Mul/div/mod
 *   - Sign/zero extend
 *   - Not/neg
 *   - And/or/xor
 *   - Bit shift/rotate
 * - Float: TBD
 * - Control flow:
 *   - Jump
 *   - Conditional jump
 *   - Enter/exit
 *   - Call/return
 *   - Call with custom CC/ABI (native code)
 *   - Try/catch/unwind TBD
 */

/*
 * Key: 0, 1 = Decision bits.
 *      A... = Instruction parameter.
 *      f... = Not reserved, with condition/requirement.
 *      _    = Any bit.
 *      ^    = Subset of above prefix.
 *      .    = Argument infix, not part of decision prefix.
 *
 * 00__ ____ ____ ____ Form 0. Misc.
 * ^^00 0000 00__ ____ > Form 0.0.
 * 0000 0000 00BB RRRR   Load 16-bit constant. R[B] = pc+1; ++pc.
 * 0000 ffff ff__ ____ > Empty, f != 0.
 * ^^01 ____ ____ ____ > Form 0.1.
 * 0001 XXXX XXXX RRRR   Nullary / Unary / Intrinsic: R = X(R?).
 * ^^10 ____ ____ 1111 > Form 0.2.
 * 0010 XXXX XXXX 1111   ABI defined intrinsic: X().
 * ^^10 ____ ____ ____ > Form 0.3.
 * 0010 SSSS RRRR XXXX   More binary ops (?), X < 0xF.
 * 0011 ____ ____ ____ > Empty.
 *
 * 01__ ____ ____ ____ Form 1. Binary ops.
 * 01XX XXXX SSSS RRRR Binary op: R = X(R, S).
 *
 * 10__ ____ ____ ____ Form 2. Load/extend.
 * 10TW NNNN NNNN RRRR Load 8 bit constant: R.W = ext[T](N).
 *
 * 110_ ____ ____ ____ Form 3. Conditional.
 * ^^^_ __00 ____ ____ > Form 3.0.
 * ^^^. ..^^ ____ ____ --> Form 3.0.0.
 * 110C CC00 SSSS RRRR     Absolute jump cond.: if C(S) then pc = *R.
 * ^^^1 00^^ 1111 ____ --> Form 3.0.1.
 * 1101 0000 1111 RRRR     Absolute jump: pc = *R.
 * ^^^1 00^^ 10__ ____ --> Form 3.0.2.
 * 1101 0000 10AA AAAA     Page jump.
 * 1101 0000 0___ ____ --> Empty.
 * ^^^. ..01 ____ ____ > Form 3.1.
 * 110C CC01 SSSS RRRR   Conditional move: if C(R) then R = S.
 * 1101 0001 SSSS RRRR   Unconditional move: R = S.
 * 110_ __1_ ____ ____ > Empty.
 *
 * 111_ ____ ____ ____ Form 4. Relative jump.
 * ^^^_ ____ ____ ____ > Form 4.0.
 * 111C CCAA AAAA RRRR   Relative jump cond: if C(R) then pc += A.
 * 111f ff00 0000 ____   Nop, f != 100 (if who-cares then pc += 0).
 * 111f ff11 1111 ____   Invalid - jump to self (pc -= 1), f != 100.
 * ^^^1 00__ ____ 1101 > Form 4.1.
 * 1111 00AA AAAA 1101   Relative jump: pc += A.
 * 1111 0000 0000 1101   Nop (pc += 0).
 * 1111 0011 1111 1101   Invalid - jump to self (pc -= 1).
 * 1111 00__ ____ ffff > Empty, f != 1101.
 */

/// Load 16-bit constant from next instruction slot.
/// @param B Double-byte slice of register to write to, 0-3.
/// @param R Register to receive the constant.
#define a8_vm_instr_form_0_0(B, R) \
  ((uint16_t)(((B) << 4) | (R)))

/// Unary or nullary operation. Performs R = X(R) or R = X().
/// @param X Operation, 8 bits (max 0xFF).
/// @param R Destination/source register, 4 bits (max 0xF).
#define a8_vm_instr_form_0_1(X, R) \
  ((uint16_t)(0x1000U | ((X) << 4) | (R)))

/// ABI defined intrinsic. Performs intrinsic[X]().
/// @param X Intrinsic index, 8 bits (max 0xFF).
#define a8_vm_instr_form_0_2(X) \
  ((uint16_t)(0x200FU | ((X) << 4)))

/// Additional binary operations.
/// @param X Operation, 4 bits, < 0xF (max 0xE).
/// @param R Destination/source register, 4 bits (max 0xF).
/// @param S Source register, 4 bits (max 0xF).
#define a8_vm_instr_form_0_3(X, R, S) \
  ((uint16_t)(0x2000U | ((S) << 8) | ((R) << 4) | (X)))

/// Binary operation. Performs R = X(R, S).
/// @param X Operation, 6 bits (max 0x3F).
/// @param R Destination/source register, 4 bits (max 0xF).
/// @param S Source register, 4 bits (max 0xF).
#define a8_vm_instr_form_1(X, R, S) \
  ((uint16_t)(0x4000U | ((X) << 8) | ((S) << 4) | (R)))

/// Load 8-bit constant and extend.
/// @param T Extend mode, 1 bit, 0 = zero extend, 1 = sign extend.
/// @param W Extend size, 1 bit, 0 = 32 bits, 1 = 64 bits.
/// @param N Constant, 8 bits (max 0xFF).
/// @param R Register to receive the constant.
#define a8_vm_instr_form_2(T, W, N, R) \
  ((uint16_t)(0x8000U | ((T) << 13) | ((W) << 12) | ((N) << 4) | (R)))

/// Conditional absolute jump. Performs if C(S) then pc = *R.
/// @param C Condition flag, 3 bits, not 100 (max 0x7, except 0x4).
/// @param R Jump destination register, 4 bits (max 0xF).
/// @param S Comparison source register, 4 bits (max 0xF).
#define a8_vm_instr_form_3_0_0(C, R, S) \
  ((uint16_t)(0xC000U | ((C) << 10) | ((S) << 4) | (R)))

/// Unconditional absolute jump. Performs pc = *R.
/// Encoding equal to 3.0.0(0x4, 0xF, R).
/// Encoding equal to 3.0.2(0xF0 | R).
/// @param R Jump destination register, 4 bits (max 0xF).
#define a8_vm_instr_form_3_0_1(R) \
  ((uint16_t)(0xD0F0U | (R)))

/// Page jump.
/// @param A Instruction offset in page, 6 bits (max 0x3F).
#define a8_vm_instr_form_3_0_2(A) \
  ((uint16_t)(0xD080U | (A)))

/// Conditional move, performs if C(R) then R = S.
/// When C == 0x4, it is unconditional and not dependent on R.
/// @param C Condition flag, 3 bits,
#define a8_vm_instr_form_3_1(C, R, S) \
  ((uint16_t)(0xC100U | ((C) << 10) | ((S) << 4) | (R)))

/// Conditional relative jump. Performs if C(R) then pc += A.
/// @param C Condition flag, 3 bits, not 100 (max 0x6, except 0x4).
/// @param A Address offset, 6 bits signed (max 0x3F / [-0x20, 0x1F]).
/// @param R Condition source register, 4 bits (max 0xF).
#define a8_vm_instr_form_4_0(C, A, R) \
  ((uint16_t)(0xE000U | ((C) << 10) | ((A) << 4) | (R)))

/// Unconditional relative jump. Performs pc += A.
/// @param A Address offset, 6 bits signed (max 0x3F / [-0x20, 0x1F]).
#define a8_vm_instr_form_4_1(A) \
  ((uint16_t)(0xF00DU | ((A) << 4)))

typedef enum a8_vm_uop {
  a8_vm_uop_ldm_1,
  a8_vm_uop_ldm_2,
  a8_vm_uop_ldm_4,
  a8_vm_uop_ldm_8,

  a8_vm_uop_st_1,
  a8_vm_uop_st_2,
  a8_vm_uop_st_4,
  a8_vm_uop_st_8,

  a8_vm_uop_push_1,
  a8_vm_uop_push_2,
  a8_vm_uop_push_4,
  a8_vm_uop_push_8,

  a8_vm_uop_pop_1,
  a8_vm_uop_pop_2,
  a8_vm_uop_pop_4,
  a8_vm_uop_pop_8,

  a8_vm_uop_zext_8_16,
  a8_vm_uop_zext_8_32,
  a8_vm_uop_zext_8_64,
  a8_vm_uop_zext_16_32,
  a8_vm_uop_zext_16_64,
  a8_vm_uop_zext_32_64,

  a8_vm_uop_sext_8_16,
  a8_vm_uop_sext_8_32,
  a8_vm_uop_sext_8_64,
  a8_vm_uop_sext_16_32,
  a8_vm_uop_sext_16_64,
  a8_vm_uop_sext_32_64,

  a8_vm_uop_call,

  a8_vm_uop_not,
  a8_vm_uop_neg,

  a8_vm_uop_count_,
} a8_vm_uop;

typedef enum a8_vm_bop {
  a8_vm_bop_add,
  a8_vm_bop_sub,
  a8_vm_bop_mul,
  a8_vm_bop_div,
  a8_vm_bop_sdiv,
  a8_vm_bop_rem,
  a8_vm_bop_srem,

  a8_vm_bop_and,
  a8_vm_bop_or,
  a8_vm_bop_xor,

  a8_vm_bop_shl,
  a8_vm_bop_shr,
  a8_vm_bop_sar,
  a8_vm_bop_rol,
  a8_vm_bop_ror,

  a8_vm_bop_count_,
} a8_vm_bop;

typedef enum a8_vm_iop {
  a8_vm_iop_abort,
  a8_vm_iop_break,
  a8_vm_iop_enter,
  a8_vm_iop_exit,
  a8_vm_iop_unwind,

  a8_vm_iop_strget,
  a8_vm_iop_strid,

  a8_vm_iop_symget,
  a8_vm_iop_symnew,
  a8_vm_iop_symcast,
  a8_vm_iop_symappcast,

  a8_vm_iop_scopename,
  a8_vm_iop_scopeappname,
  a8_vm_iop_scopepath,

  a8_vm_iop_count_,
} a8_vm_iop;

typedef enum a8_vm_cond {
  a8_vm_cond_zero,
  a8_vm_cond_pos,
  a8_vm_cond_neg,
  a8_vm_cond_notzero,
  a8_vm_cond_always,
  a8_vm_cond_notpos,
  a8_vm_cond_notneg,

  a8_vm_cond_reserved_,

  a8_vm_cond_count_ = a8_vm_cond_reserved_,
} a8_vm_cond;

#endif /* A8_VM_INSTR_H_ */