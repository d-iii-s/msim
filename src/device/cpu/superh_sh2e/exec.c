/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#include "bitops.h"
#include "memops.h"
#include "cpu.h"
#include "debug.h"
#include "insn.h"
#include "exec.h"

#include "../../../assert.h"

#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * @brief Reads an 8-bit value (byte) from memory, zero-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param output_value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_readz_byte(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    uint32_t * const restrict output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    *output_value = sh2e_physmem_read8(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}


/**
 * @brief Reads an 8-bit value (byte) from memory, sign-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param output_value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_reads_byte(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    uint32_t * const restrict output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    *output_value = sign_extend_8_32(sh2e_physmem_read8(cpu->id, addr, true));
    return SH2E_EXCEPTION_NONE;
}

//

/**
 * @brief Reads a 16-bit value (word) from memory, zero-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_readz_word(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    uint32_t * const restrict output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // TODO Check alignment.
    *output_value = sh2e_physmem_read16(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}


/**
 * @brief Reads a 16-bit value (word) from memory, sign-extends it
 * to 32 bits, and stores it into `output_value`.
 *
 * @param cpu The CPU which makes the read.
 * @param addr The (physical) memory address to read from.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_reads_word(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    uint32_t * const restrict output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // TODO Check alignment.
    *output_value = sign_extend_16_32(sh2e_physmem_read16(cpu->id, addr, true));
    return SH2E_EXCEPTION_NONE;
}

//

/**
 * @brief Reads a long word (32 bits) from memory.
 *
 * @param cpu The cpu which makes the read.
 * @param addr The (physical) memory address from which to load data.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_read_long(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    uint32_t * const restrict output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // TODO Check alignment.
    *output_value = sh2e_physmem_read32(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}


/**
 * @brief Reads a float (32 bits) from memory.
 *
 * @param cpu The cpu which makes the read.
 * @param addr The (physical) memory address from which to load data.
 * @param value Pointer to where to store the loaded value.
 * @return sh2e_exception_t Exception code related to the access.
 */
static inline sh2e_exception_t
sh2e_cpu_read_float(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr,
    float32_t * const output_value
) {
    ASSERT(cpu != NULL);
    ASSERT(output_value != NULL);

    // TODO Check alignment.
    uint32_t * const output_uint32 = (uint32_t *) output_value;
    *output_uint32 = sh2e_physmem_read32(cpu->id, addr, true);
    return SH2E_EXCEPTION_NONE;
}


//

/** @brief Writes a byte value (8 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_byte(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, uint32_t const value
) {
    ASSERT(cpu != NULL);

    bool success = physmem_write8(cpu->id, addr, value, true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}


/** @brief Writes a word value (16 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_word(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, uint32_t const value
) {
    ASSERT(cpu != NULL);

    // TODO Check alignment.
    bool success = physmem_write16(cpu->id, addr, htobe16(value), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}


/** @brief Writes a long value (32 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_long(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, uint32_t const value
) {
    ASSERT(cpu != NULL);

    // TODO Check alignment.
    bool success = physmem_write32(cpu->id, addr, htobe32(value), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}


/** @brief Writes a float value (32 bits) to memory. */
static inline sh2e_exception_t
sh2e_cpu_write_float(
    sh2e_cpu_t const * const restrict cpu, uint32_t const addr, float32_t const value
) {
    ASSERT(cpu != NULL);

    // TODO Check alignment.
    sh2e_fpu_ul_t const conv = { .fvalue = value };
    bool success = physmem_write32(cpu->id, addr, htobe32(conv.ivalue), true);
    return success ? SH2E_EXCEPTION_NONE : SH2E_EXCEPTION_CPU_ADDRESS_ERROR;
}


static inline uint32_t
sh2e_cpu_get_sr(sh2e_cpu_t * const restrict cpu) {
    return cpu->cpu_regs.sr.value;
}


static inline void
sh2e_cpu_set_sr(sh2e_cpu_t * const restrict cpu, uint32_t const value) {
    cpu->cpu_regs.sr.value = value;
}


/****************************************************************************
 * SH-2E template instructions
 ****************************************************************************/

static ALWAYS_INLINE uint32_t
sh2e_addr_pc_relative_insn(sh2e_cpu_t * const restrict cpu, signed const disp) {
    // PC relative instruction addressing.
    return cpu->cpu_regs.pc + disp * sizeof(sh2e_insn_t);
}


static ALWAYS_INLINE uint32_t
sh2e_addr_gbr_relative_data(
    sh2e_cpu_t * const restrict cpu, unsigned const disp, uint_fast8_t const scale
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    // GBR relative addressing with unsigned displacement.
    return cpu->cpu_regs.gbr + (disp * scale);
}

// BT (Branch if True): Branch Instruction
// BF (Branch if False): Branch Instruction
// BT/S (Branch if True with Delay Slot): Branch Instruction [Delayed Slot Instruction]
// BF/S (Branch if False with Delay Slot): Branch Instruction [Delayed Slot Instruction]

static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_branch_t_eq(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn,
    unsigned const t_expected, bool const delayed
) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    if (cpu->cpu_regs.sr.t == t_expected) {
        // TODO Leave cycle counting to the higher-level function.
        cpu->cycles += delayed ? 1 : 2;

        uint32_t const disp = sign_extend_8_32(insn.d8);
        cpu->pc_next = sh2e_addr_pc_relative_insn(cpu, 2 + disp);
        cpu->br_state = delayed ? SH2E_BRANCH_STATE_DELAY : SH2E_BRANCH_STATE_EXECUTE;
    }

    return SH2E_EXCEPTION_NONE;
}

// EXTx (Extend as Signed or Unsigned): Arithmetic Instruction

static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_ext(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    sh2e_extend_fn_t const sh2e_extend
) {
    cpu->cpu_regs.general[insn.rn] = sh2e_extend(cpu->cpu_regs.general[insn.rm]);
    return SH2E_EXCEPTION_NONE;
}

// MOV (Move Immediate Data): Data Transfer Instruction

static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_movi(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nd8_t const insn, unsigned const scale,
    sh2e_cpu_read_fn_t const sh2e_cpu_read, sh2e_extend_fn_t const sh2e_sign_extend
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    uint32_t const disp = zero_extend_8_32(insn.d8);
    uint32_t const addr = (cpu->cpu_regs.pc & (~(scale - 1))) + (disp * scale);

    uint32_t mem_value;
    sh2e_exception_t const cpu_read_ex = sh2e_cpu_read(cpu, addr, &mem_value);
    if (cpu_read_ex == SH2E_EXCEPTION_NONE) {
        cpu->cpu_regs.general[insn.rn] = sh2e_sign_extend(mem_value);
    }

    return cpu_read_ex;
}

// MOV (Move Data): Data Transfer Instruction

static inline sh2e_exception_t
sh2e_insn_movl(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    sh2e_cpu_read_fn_t const sh2e_cpu_read
) {
    uint32_t const addr = cpu->cpu_regs.general[insn.rm];
    return sh2e_cpu_read(cpu, addr, &cpu->cpu_regs.general[insn.rn]);
}


static inline sh2e_exception_t
sh2e_insn_movs(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    sh2e_cpu_write_fn_t const sh2e_cpu_write
) {
    uint32_t const addr = cpu->cpu_regs.general[insn.rn];
    return sh2e_cpu_write(cpu, addr, cpu->cpu_regs.general[insn.rm]);
}

//

static inline sh2e_exception_t
sh2e_insn_movp(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    unsigned const scale, sh2e_cpu_read_fn_t const sh2e_cpu_read
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    uint32_t const addr = cpu->cpu_regs.general[insn.rm];
    sh2e_exception_t const cpu_read_ex = sh2e_cpu_read(cpu, addr, &cpu->cpu_regs.general[insn.rn]);
    if (cpu_read_ex != SH2E_EXCEPTION_NONE) {
        return cpu_read_ex;
    }

    if (insn.rn != insn.rm) {
        cpu->cpu_regs.general[insn.rm] = addr + scale;
    }

    return SH2E_EXCEPTION_NONE;
}


static inline sh2e_exception_t
sh2e_insn_movm(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    unsigned const scale, sh2e_cpu_write_fn_t const sh2e_cpu_write
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    uint32_t const addr = cpu->cpu_regs.general[insn.rn] - scale;
    sh2e_exception_t const cpu_write_ex = sh2e_cpu_write(cpu, addr, cpu->cpu_regs.general[insn.rm]);
    if (cpu_write_ex != SH2E_EXCEPTION_NONE) {
        return cpu_write_ex;
    }

    cpu->cpu_regs.general[insn.rn] = addr;
    return SH2E_EXCEPTION_NONE;
}

//

static inline sh2e_exception_t
sh2e_insn_movl0(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    sh2e_cpu_read_fn_t const sh2e_cpu_read
) {
    uint32_t const addr = cpu->cpu_regs.r0 + cpu->cpu_regs.general[insn.rm];
    return sh2e_cpu_read(cpu, addr, &cpu->cpu_regs.general[insn.rn]);
}


static inline sh2e_exception_t
sh2e_insn_movs0(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn,
    sh2e_cpu_write_fn_t const sh2e_cpu_write
) {
    uint32_t const addr = cpu->cpu_regs.r0 + cpu->cpu_regs.general[insn.rn];
    return sh2e_cpu_write(cpu, addr, cpu->cpu_regs.general[insn.rm]);
}

// MOV (Move Structure Data): Data Transfer Instruction

static inline sh2e_exception_t
sh2e_insn_movl4(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_md_t const insn,
    unsigned const scale, sh2e_cpu_read_fn_t const sh2e_cpu_read
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    uint32_t const disp = zero_extend_4_32(insn.d4);
    uint32_t const addr = cpu->cpu_regs.general[insn.rm] + (disp * scale);
    return sh2e_cpu_read(cpu, addr, &cpu->cpu_regs.r0);
}


static inline sh2e_exception_t
sh2e_insn_movs4(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_nd4_t const insn,
    unsigned const scale, sh2e_cpu_write_fn_t const sh2e_cpu_write
) {
    ASSERT(0 <= POWER_OF_TWO(scale) && POWER_OF_TWO(scale) <= 2 && "invalid `scale` value");

    uint32_t const disp = zero_extend_4_32(insn.d4);
    uint32_t const addr = cpu->cpu_regs.general[insn.rn] + (disp * scale);
    return sh2e_cpu_write(cpu, addr, cpu->cpu_regs.r0);
}

// MOV (Move Peripheral Data): Data Transfer Instruction

static inline sh2e_exception_t
sh2e_insn_movlg(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn,
    uint_fast8_t const scale, sh2e_cpu_read_fn_t const sh2e_cpu_read
) {
    uint32_t const disp = zero_extend_8_32(insn.d8);
    uint32_t const addr = sh2e_addr_gbr_relative_data(cpu, disp, scale);
    return sh2e_cpu_read(cpu, addr, &cpu->cpu_regs.r0);
}


static inline sh2e_exception_t
sh2e_insn_movsg(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn,
    uint_fast8_t const scale, sh2e_cpu_write_fn_t const sh2e_cpu_write
) {
    uint32_t const disp = zero_extend_8_32(insn.d8);
    uint32_t const addr = sh2e_addr_gbr_relative_data(cpu, disp, scale);
    return sh2e_cpu_write(cpu, addr, cpu->cpu_regs.r0);
}

// Set T Bit: System Control Instruction

static inline sh2e_exception_t
sh2e_insn_set_t(sh2e_cpu_t * const restrict cpu, bool const value) {
    cpu->cpu_regs.sr.t = value;
    return SH2E_EXCEPTION_NONE;
}

//

static inline sh2e_exception_t
sh2e_insn_stcsr(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn,
    uint32_t const * const csrs
) {
    uint_fast8_t const csr = (insn.ic_l >> 4) & 0b11;
    cpu->cpu_regs.general[insn.rn] = csrs[csr];
    return SH2E_EXCEPTION_NONE;
}


static inline sh2e_exception_t
sh2e_insn_stcsrm(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn,
    uint32_t const * const csrs
) {
    uint_fast8_t const csr = (insn.ic_l >> 4) & 0b11;
    uint32_t const addr = cpu->cpu_regs.general[insn.rn] - sizeof(uint32_t);
    sh2e_exception_t const cpu_write_ex = sh2e_cpu_write_long(cpu, addr, csrs[csr]);
    if (cpu_write_ex != SH2E_EXCEPTION_NONE) {
        return cpu_write_ex;
    }

    // Only update the register if there was no exception.
    cpu->cpu_regs.general[insn.rn] = addr;
    return SH2E_EXCEPTION_NONE;
}


/****************************************************************************
 * SH-2E CPU instructions
 ****************************************************************************/

// ADD (ADD Binary): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_add(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] += cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

sh2e_exception_t
sh2e_insn_exec_addi(sh2e_cpu_t * const restrict cpu, sh2e_insn_ni_t const insn) {
    cpu->cpu_regs.general[insn.rn] += sign_extend_8_32(insn.i8);
    return SH2E_EXCEPTION_NONE;
}

// ADDC (ADD with Carry): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_addc(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const sum = rn + cpu->cpu_regs.general[insn.rm];
    uint32_t const sum_with_carry = sum + cpu->cpu_regs.sr.t;

    // Carry must be set if the 32-bit results are lesser than
    // the original operands. Check after each addition.
    cpu->cpu_regs.general[insn.rn] = sum_with_carry;
    cpu->cpu_regs.sr.t = ((rn > sum) || (sum > sum_with_carry)) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// ADDV (ADD with V Flag Overflow Check): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_addv(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    uint32_t const result = rn + rm;
    cpu->cpu_regs.general[insn.rn] = result;

    // Overflow occurs if both operands have the same sign and the
    // sign of the result differs from that of the operands.
    uint32_t const rn_sign = bit_value_msb(rn);
    uint32_t const rm_sign = bit_value_msb(rm);
    uint32_t const result_sign = bit_value_msb(result);
    cpu->cpu_regs.sr.t = ((rn_sign == rm_sign) && (rm_sign != result_sign)) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// AND (AND Logical): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_and(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] &= cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_andi(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    cpu->cpu_regs.r0 &= zero_extend_8_32(insn.i8);
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_andm(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t value;
    uint32_t const addr = cpu->cpu_regs.gbr + cpu->cpu_regs.r0;
    sh2e_exception_t cpu_rdwr_ex = sh2e_cpu_readz_byte(cpu, addr, &value);
    if (cpu_rdwr_ex == SH2E_EXCEPTION_NONE) {
        uint32_t const result = value & zero_extend_8_32(insn.i8);
        cpu_rdwr_ex = sh2e_cpu_write_byte(cpu, addr, result);
    }

    return cpu_rdwr_ex;
}

// BF (Branch if False): Branch Instruction

sh2e_exception_t
sh2e_insn_exec_bf(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_branch_t_eq(cpu, insn, 0, false);
}

// BF/S (Branch if False): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_bfs(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_branch_t_eq(cpu, insn, 0, true);
}

// BRA (Branch): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_bra(sh2e_cpu_t * const restrict cpu, sh2e_insn_d12_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    uint32_t const disp = sign_extend_12_32(insn.d12);
    cpu->pc_next = sh2e_addr_pc_relative_insn(cpu, 2 + disp);
    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// BRAF (Branch Far): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_braf(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    cpu->pc_next = sh2e_addr_pc_relative_insn(cpu, 2) + cpu->cpu_regs.general[insn.rm];
    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// BSR (Branch to Subroutine): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_bsr(sh2e_cpu_t * const restrict cpu, sh2e_insn_d12_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    uint32_t const disp = sign_extend_12_32(insn.d12);
    cpu->cpu_regs.pr = sh2e_addr_pc_relative_insn(cpu, 2);
    cpu->pc_next = sh2e_addr_pc_relative_insn(cpu, 2 + disp);
    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// BSRF (Branch to Subroutine Far): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_bsrf(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    cpu->cpu_regs.pr = sh2e_addr_pc_relative_insn(cpu, 2);
    cpu->pc_next = sh2e_addr_pc_relative_insn(cpu, 2) + cpu->cpu_regs.general[insn.rm];
    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// BT (Branch if True): Branch Instruction

sh2e_exception_t
sh2e_insn_exec_bt(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_branch_t_eq(cpu, insn, 1, false);
}

// BTS (Branch if True): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_bts(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_branch_t_eq(cpu, insn, 1, true);
}

// CLRMAC (Clear MAC Register): System Control Instruction

sh2e_exception_t
sh2e_insn_exec_clrmac(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    cpu->cpu_regs.mach = 0;
    cpu->cpu_regs.macl = 0;
    return SH2E_EXCEPTION_NONE;
}

// CLRT (Clear T Bit): System Control Instruction

sh2e_exception_t
sh2e_insn_exec_clrt(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    return sh2e_insn_set_t(cpu, false);
}

// CMP/cond (Compare Conditionally): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_cmpeq(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (rn == rm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmpge(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    int32_t const rn = cpu->cpu_regs.general[insn.rn];
    int32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (rn >= rm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmpgt(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    int32_t const rn = cpu->cpu_regs.general[insn.rn];
    int32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (rn > rm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmphi(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (rn > rm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmphs(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (rn >= rm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmpim(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t const imm = sign_extend_8_32(insn.i8);
    cpu->cpu_regs.sr.t = (cpu->cpu_regs.r0 == imm) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmppl(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    int32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.sr.t = (rn > 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmppz(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    int32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.sr.t = (rn >= 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_cmpstr(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    //
    // Compares the general registers Rn and Rm, and sets the T bit if
    // any of the 4 bytes in Rn are equal to the corresponding byte in Rm.
    // The contents of Rn and Rm are not changed.
    //
    // See https://github.com/qemu/qemu/blob/master/target/sh4/translate.c#L737
    //
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];

    uint32_t const rn_xor_rm = rn ^ rm;
    uint32_t const sub_val = rn_xor_rm - 0x01010101;
    uint32_t const andc_val = sub_val & ~rn_xor_rm;
    uint32_t const sign_val = andc_val & 0x80808080;

    cpu->cpu_regs.sr.t = (sign_val != 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// DIV0S (Divide Step 0 as Signed): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_div0s(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.sr.q = bit_value_msb(cpu->cpu_regs.general[insn.rn]);
    cpu->cpu_regs.sr.m = bit_value_msb(cpu->cpu_regs.general[insn.rm]);
    cpu->cpu_regs.sr.t = cpu->cpu_regs.sr.q ^ cpu->cpu_regs.sr.m;
    return SH2E_EXCEPTION_NONE;
}

// DIV0U (Divide Step 0 as Unsigned): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_div0u(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    cpu->cpu_regs.sr.m = 0;
    cpu->cpu_regs.sr.q = 0;
    cpu->cpu_regs.sr.t = 0;
    return SH2E_EXCEPTION_NONE;
}

// DIV1 (Divide 1 Step): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_div1(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    // See https://github.com/qemu/qemu/blob/master/target/sh4/translate.c#L753
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rn_sign_xor_m = bit_value_msb(rn) ^ cpu->cpu_regs.sr.m;

    // Subtract (add) Rm from (to) Rn when Q == M (Q != M).
    uint32_t const shifted_rn = (rn << 1) | cpu->cpu_regs.sr.t;
    uint32_t const q_xor_m = cpu->cpu_regs.sr.q ^ cpu->cpu_regs.sr.m;
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    uint32_t const next_rn = shifted_rn + (q_xor_m == 0) ? -rm : rm;
    cpu->cpu_regs.general[insn.rn] = next_rn;

    // Determine Q and T based on carry and divider sign.
    uint32_t const carry = ((q_xor_m == 0) ? next_rn > shifted_rn : next_rn < shifted_rn) ? 1 : 0;
    cpu->cpu_regs.sr.q = carry ^ rn_sign_xor_m;
    cpu->cpu_regs.sr.t = (cpu->cpu_regs.sr.q == cpu->cpu_regs.sr.m) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// DT (Decrement and Test): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_dt(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    int32_t const rn_dec = cpu->cpu_regs.general[insn.rn] - 1;
    cpu->cpu_regs.general[insn.rn] = rn_dec;
    cpu->cpu_regs.sr.t = (rn_dec == 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// EXTS (Extend as Signed): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_extsb(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_ext(cpu, insn, sign_extend_8_32);
}

sh2e_exception_t
sh2e_insn_exec_extsw(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_ext(cpu, insn, sign_extend_16_32);
}

// EXTU (Extend as Unsigned): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_extub(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_ext(cpu, insn, zero_extend_8_32);
}

sh2e_exception_t
sh2e_insn_exec_extuw(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_ext(cpu, insn, zero_extend_16_32);
}

// JMP (Jump): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_jmp(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    cpu->pc_next = cpu->cpu_regs.general[insn.rm];

    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// JSR (Jump to Subroutine): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_jsr(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    cpu->cpu_regs.pr = sh2e_addr_pc_relative_insn(cpu, 2);
    cpu->pc_next = cpu->cpu_regs.general[insn.rm];

    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// MOV (Move Data): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_mov(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] = cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_movbs(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs(cpu, insn, sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movws(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs(cpu, insn, sh2e_cpu_write_word);
}


sh2e_exception_t
sh2e_insn_exec_movls(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs(cpu, insn, sh2e_cpu_write_long);
}


sh2e_exception_t
sh2e_insn_exec_movbl(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl(cpu, insn, sh2e_cpu_reads_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwl(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl(cpu, insn, sh2e_cpu_reads_word);
}


sh2e_exception_t
sh2e_insn_exec_movll(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl(cpu, insn, sh2e_cpu_read_long);
}


sh2e_exception_t
sh2e_insn_exec_movbm(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movm(cpu, insn, sizeof(uint8_t), sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwm(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movm(cpu, insn, sizeof(uint16_t), sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movlm(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movm(cpu, insn, sizeof(uint32_t), sh2e_cpu_write_long);
}


sh2e_exception_t
sh2e_insn_exec_movbp(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movp(cpu, insn, sizeof(uint8_t), sh2e_cpu_reads_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwp(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movp(cpu, insn, sizeof(uint16_t), sh2e_cpu_reads_word);
}


sh2e_exception_t
sh2e_insn_exec_movlp(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movp(cpu, insn, sizeof(uint32_t), sh2e_cpu_read_long);
}


sh2e_exception_t
sh2e_insn_exec_movbs0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs0(cpu, insn, sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movws0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs0(cpu, insn, sh2e_cpu_write_word);
}


sh2e_exception_t
sh2e_insn_exec_movls0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movs0(cpu, insn, sh2e_cpu_write_long);
}


sh2e_exception_t
sh2e_insn_exec_movbl0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl0(cpu, insn, sh2e_cpu_reads_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwl0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl0(cpu, insn, sh2e_cpu_reads_word);
}


sh2e_exception_t
sh2e_insn_exec_movll0(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_movl0(cpu, insn, sh2e_cpu_read_long);
}

// MOV (Move Immediate Data): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_movi(sh2e_cpu_t * const restrict cpu, sh2e_insn_ni_t const insn) {
    // MOV (Move Immediate Data): Data Transfer Instruction
    cpu->cpu_regs.general[insn.rn] = sign_extend_8_32(insn.i8);
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_movwi(sh2e_cpu_t * const restrict cpu, sh2e_insn_nd8_t const insn) {
    return sh2e_insn_movi(cpu, insn, sizeof(uint16_t), sh2e_cpu_readz_word, sign_extend_16_32);
}


sh2e_exception_t
sh2e_insn_exec_movli(sh2e_cpu_t * const restrict cpu, sh2e_insn_nd8_t const insn) {
    return sh2e_insn_movi(cpu, insn, sizeof(uint32_t), sh2e_cpu_read_long, sign_extend_32_32);
}

// MOV (Move Peripheral Data): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_movblg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movlg(cpu, insn, sizeof(uint8_t), sh2e_cpu_reads_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwlg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movlg(cpu, insn, sizeof(uint16_t), sh2e_cpu_reads_word);
}


sh2e_exception_t
sh2e_insn_exec_movllg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movlg(cpu, insn, sizeof(uint32_t), sh2e_cpu_read_long);
}


sh2e_exception_t
sh2e_insn_exec_movbsg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movsg(cpu, insn, sizeof(uint8_t), sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwsg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movsg(cpu, insn, sizeof(uint16_t), sh2e_cpu_write_word);
}


sh2e_exception_t
sh2e_insn_exec_movlsg(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    return sh2e_insn_movsg(cpu, insn, sizeof(uint32_t), sh2e_cpu_write_long);
}

// MOV (Move Structure Data): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_movbl4(sh2e_cpu_t * const restrict cpu, sh2e_insn_md_t const insn) {
    return sh2e_insn_movl4(cpu, insn, sizeof(uint8_t), sh2e_cpu_reads_byte);
}


sh2e_exception_t
sh2e_insn_exec_movwl4(sh2e_cpu_t * const restrict cpu, sh2e_insn_md_t const insn) {
    return sh2e_insn_movl4(cpu, insn, sizeof(uint16_t), sh2e_cpu_reads_word);
}


sh2e_exception_t
sh2e_insn_exec_movll4(sh2e_cpu_t * const restrict cpu, sh2e_insn_nmd_t const insn) {
    uint32_t const disp = zero_extend_4_32(insn.d4);
    uint32_t const addr = cpu->cpu_regs.general[insn.rm] + (disp * sizeof(uint32_t));
    return sh2e_cpu_read_long(cpu, addr, &cpu->cpu_regs.general[insn.rn]);
}


sh2e_exception_t
sh2e_insn_exec_movbs4(sh2e_cpu_t * const restrict cpu, sh2e_insn_nd4_t const insn) {
    return sh2e_insn_movs4(cpu, insn, sizeof(uint8_t), sh2e_cpu_write_byte);
}


sh2e_exception_t
sh2e_insn_exec_movws4(sh2e_cpu_t * const restrict cpu, sh2e_insn_nd4_t const insn) {
    return sh2e_insn_movs4(cpu, insn, sizeof(uint16_t), sh2e_cpu_write_word);
}


sh2e_exception_t
sh2e_insn_exec_movls4(sh2e_cpu_t * const restrict cpu, sh2e_insn_nmd_t const insn) {
    uint32_t const disp = zero_extend_4_32(insn.d4);
    uint32_t const addr = cpu->cpu_regs.general[insn.rn] + (disp * sizeof(uint32_t));
    return sh2e_cpu_write_long(cpu, addr, cpu->cpu_regs.general[insn.rm]);
}

// MOVA (Move Effective Address): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_mova(sh2e_cpu_t * const restrict cpu, sh2e_insn_d_t const insn) {
    // TODO Check address calculation! The starting address of PC is unclear.
    uint32_t const disp = zero_extend_8_32(insn.d8);
    cpu->cpu_regs.r0 = (cpu->cpu_regs.pc & UINT32_C(0xFFFFFFFC)) + (disp * sizeof(uint32_t));
    return SH2E_EXCEPTION_NONE;
}

// MOVT (Move T Bit): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_movt(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    cpu->cpu_regs.general[insn.rn] = cpu->cpu_regs.sr.t;
    return SH2E_EXCEPTION_NONE;
}

// MUL.L (Multiply Long): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_mull(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.macl = cpu->cpu_regs.general[insn.rn] * cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

// MULS.W (Multiply as Signed Word): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_mulsw(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    int32_t const rn = (int16_t) cpu->cpu_regs.general[insn.rn];
    int32_t const rm = (int16_t) cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.macl = rn * rm;
    return SH2E_EXCEPTION_NONE;
}

// MULU.W (Multiply as Unsigned Word): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_muluw(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = (uint16_t) cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = (uint16_t) cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.macl = rn * rm;
    return SH2E_EXCEPTION_NONE;
}

// NEG (Negate): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_neg(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] = 0 - cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

// NEGC (Negate with Carry): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_negc(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const neg_rm = 0 - cpu->cpu_regs.general[insn.rm];
    uint32_t const neg_rm_with_carry = neg_rm - cpu->cpu_regs.sr.t;
    cpu->cpu_regs.general[insn.rn] = neg_rm_with_carry;
    cpu->cpu_regs.sr.t = ((0 < neg_rm) || (neg_rm < neg_rm_with_carry)) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// NOP (No Operation): System Control Instruction

sh2e_exception_t
sh2e_insn_exec_nop(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    return SH2E_EXCEPTION_NONE;
}

// NOT (NOT-Logical Complement): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_not(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] = ~cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

// OR (OR Logical): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_or(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] |= cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_ori(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    cpu->cpu_regs.r0 |= zero_extend_8_32(insn.i8);
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_orm(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t value;
    uint32_t const addr = cpu->cpu_regs.gbr + cpu->cpu_regs.r0;
    sh2e_exception_t cpu_rdwr_ex = sh2e_cpu_readz_byte(cpu, addr, &value);
    if (cpu_rdwr_ex == SH2E_EXCEPTION_NONE) {
        uint32_t const result = value | zero_extend_8_32(insn.i8);
        cpu_rdwr_ex = sh2e_cpu_write_byte(cpu, addr, result);
    }

    return cpu_rdwr_ex;
}

// RTE (Return from Exception): System Control Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_rte(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    sh2e_exception_t cpu_read_ex;

    uint32_t stack_pc;
    uint32_t stack_pc_addr = cpu->cpu_regs.sp;
    cpu_read_ex = sh2e_cpu_read_long(cpu, stack_pc_addr, &stack_pc);
    if (cpu_read_ex != SH2E_EXCEPTION_NONE) {
        return cpu_read_ex;
    }

    uint32_t stack_sr;
    uint32_t stack_sr_addr = stack_pc_addr + sizeof(uint32_t);
    cpu_read_ex = sh2e_cpu_read_long(cpu, stack_sr_addr, &stack_sr);
    if (cpu_read_ex != SH2E_EXCEPTION_NONE) {
        return cpu_read_ex;
    }

    cpu->cpu_regs.sp = stack_sr_addr + sizeof(uint32_t);
    sh2e_cpu_set_sr(cpu, stack_sr & 0x0FFF0FFF);
    cpu->pc_next = stack_pc;

    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// RTS (Return from Subroutine): Branch Instruction [Delayed Branch Instruction]

sh2e_exception_t
sh2e_insn_exec_rts(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    if (cpu->br_state == SH2E_BRANCH_STATE_DELAY) {
        return SH2E_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION;
    }

    //
    // We expect JSR to set PR to point at the target instruction.
    //
    // This differs from the spec code, where RTS sets PC = PR + 4 while JSR
    // sets PR = PC when jumping. But the description of JSR in the spec says
    // that the stored/saved PC is the address four bytes after JSR, indicating
    // that JSR should set PR = PC + 4, thus requiring RTS to set PC = PR (which
    // makes a bit more sense, why should PR point at the jump instruction?).
    //
    cpu->pc_next = cpu->cpu_regs.pr;
    cpu->br_state = SH2E_BRANCH_STATE_DELAY;
    return SH2E_EXCEPTION_NONE;
}

// ROTCL (Rotate with Carry Left): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_rotcl(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const lsb = cpu->cpu_regs.sr.t;
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = (rn << 1) | lsb;
    cpu->cpu_regs.sr.t = bit_value_msb(rn);
    return SH2E_EXCEPTION_NONE;
}

// ROTCR (Rotate with Carry Right): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_rotcr(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const msb = cpu->cpu_regs.sr.t << (8 * sizeof(uint32_t) - 1);
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = msb | (rn >> 1);
    cpu->cpu_regs.sr.t = bit_value_lsb(rn);
    return SH2E_EXCEPTION_NONE;
}

// ROTL (Rotate Left): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_rotl(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = rotate_left(rn, 1);
    cpu->cpu_regs.sr.t = bit_value_msb(rn);
    return SH2E_EXCEPTION_NONE;
}

// ROTR (Rotate Right): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_rotr(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = rotate_right(rn, 1);
    cpu->cpu_regs.sr.t = bit_value_lsb(rn);
    return SH2E_EXCEPTION_NONE;
}

// SETT (Set T Bit): System Control Instruction

sh2e_exception_t
sh2e_insn_exec_sett(sh2e_cpu_t * const restrict cpu, sh2e_insn_z_t const insn) {
    return sh2e_insn_set_t(cpu, true);
}

// SHAL (Shift Arithmetic Left): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_shal(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = rn << 1;
    cpu->cpu_regs.sr.t = bit_value_msb(rn);
    return SH2E_EXCEPTION_NONE;
}

// SHAR (Shift Arithmetic Right): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_shar(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = ((int32_t) rn) >> 1;
    cpu->cpu_regs.sr.t = bit_value_lsb(rn);
    return SH2E_EXCEPTION_NONE;
}

// SHLL (Shift Logical Left): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_shll(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = rn << 1;
    cpu->cpu_regs.sr.t = bit_value_msb(rn);
    return SH2E_EXCEPTION_NONE;
}

// SHLLn (Shift Logical Left n Bits): Shift Instruction

static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_shlln(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn, unsigned const amount
) {
    cpu->cpu_regs.general[insn.rn] <<= amount;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_shll2(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlln(cpu, insn, 2);
}


sh2e_exception_t
sh2e_insn_exec_shll8(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlln(cpu, insn, 8);
}


sh2e_exception_t
sh2e_insn_exec_shll16(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlln(cpu, insn, 16);
}

// SHLR (Shift Logical Right): Shift Instruction

sh2e_exception_t
sh2e_insn_exec_shlr(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    cpu->cpu_regs.general[insn.rn] = rn >> 1;
    cpu->cpu_regs.sr.t = bit_value_lsb(rn);
    return SH2E_EXCEPTION_NONE;
}

// SHLRn (Shift Logical Right n Bits): Shift Instruction

static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_shlrn(
    sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn, unsigned const amount
) {
    cpu->cpu_regs.general[insn.rn] >>= amount;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_shlr2(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlrn(cpu, insn, 2);
}


sh2e_exception_t
sh2e_insn_exec_shlr8(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlrn(cpu, insn, 8);
}


sh2e_exception_t
sh2e_insn_exec_shlr16(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_shlrn(cpu, insn, 16);
}

// STC (Store Control Register): System Control Instruction [Interrupt Disabled Instruction]

sh2e_exception_t
sh2e_insn_exec_stc_cpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsr(cpu, insn, cpu->cpu_regs.control);
}

sh2e_exception_t
sh2e_insn_exec_stcm_cpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsrm(cpu, insn, cpu->cpu_regs.control);
}

// STS (Store System Register): System Control Instruction [Interrupt Disabled Instruction]

sh2e_exception_t
sh2e_insn_exec_sts_cpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsr(cpu, insn, cpu->cpu_regs.system);
}

sh2e_exception_t
sh2e_insn_exec_stsm_cpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsrm(cpu, insn, cpu->cpu_regs.system);
}

// SUB (Subtract Binary): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_sub(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] -= cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

// SUBC (Subtract with Carry): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_subc(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const result = rn - cpu->cpu_regs.general[insn.rm];
    uint32_t const result_with_borrow = result - cpu->cpu_regs.sr.t;

    // Borrow must be set if the 32-bit results are greater than
    // the original operands. Check after each operation.
    cpu->cpu_regs.general[insn.rn] = result_with_borrow;
    cpu->cpu_regs.sr.t = ((rn < result) || (result < result_with_borrow)) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// SUBV (Subtract with V Flag Underflow Check): Arithmetic Instruction

sh2e_exception_t
sh2e_insn_exec_subv(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    uint32_t const result = rn - rm;
    cpu->cpu_regs.general[insn.rn] = result;

    // Underflow occurs if the two operands have different signs and
    // the sign of the result differs from that of the subtrahend.
    uint32_t const rn_sign = bit_value_msb(rn);
    uint32_t const rm_sign = bit_value_msb(rm);
    uint32_t const result_sign = bit_value_msb(result);
    cpu->cpu_regs.sr.t = ((rn_sign != rm_sign) && (rm_sign != result_sign)) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// SWAP (Swap Register Halves): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_swapb(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.general[insn.rn] = (rm & 0xFFFF0000) | ((rm & 0xFF) << 8) | ((rm >> 8) & 0xFF);
    return SH2E_EXCEPTION_NONE;
}

sh2e_exception_t
sh2e_insn_exec_swapw(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.general[insn.rn] = rotate_left(rm, 16);
    return SH2E_EXCEPTION_NONE;
}

// TAS (Test and Set): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_tas(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    uint32_t const addr = cpu->cpu_regs.general[insn.rn];

    /* BUS-LOCK-ENABLE */

    uint32_t value;
    sh2e_exception_t const cpu_read_ex = sh2e_cpu_reads_byte(cpu, addr, &value);
    if (cpu_read_ex != SH2E_EXCEPTION_NONE) {
        return cpu_read_ex;
    }

    sh2e_exception_t const cpu_write_ex = sh2e_cpu_write_byte(cpu, addr, (value | 0x80));
    if (cpu_write_ex != SH2E_EXCEPTION_NONE) {
        return cpu_write_ex;
    }

    /* BUS-LOCK-DISABLE */

    cpu->cpu_regs.sr.t = (value == 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

// TRAPA (Trap Always): System Control Instruction

sh2e_exception_t
sh2e_insn_exec_trapa(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    sh2e_exception_t cpu_rdwr_ex;

    uint32_t const stack_sr = sh2e_cpu_get_sr(cpu);
    uint32_t const stack_sr_addr = cpu->cpu_regs.sp - sizeof(uint32_t);
    cpu_rdwr_ex = sh2e_cpu_write_long(cpu, stack_sr_addr, stack_sr);
    if (cpu_rdwr_ex != SH2E_EXCEPTION_NONE) {
        return cpu_rdwr_ex;
    }

    uint32_t const stack_pc = sh2e_addr_pc_relative_insn(cpu, 1);
    uint32_t const stack_pc_addr = stack_sr_addr - sizeof(uint32_t);
    cpu_rdwr_ex = sh2e_cpu_write_long(cpu, stack_pc_addr, stack_pc);
    if (cpu_rdwr_ex != SH2E_EXCEPTION_NONE) {
        return cpu_rdwr_ex;
    }

    uint32_t vbr_pc;
    uint32_t const imm = zero_extend_8_32(insn.i8);
    uint32_t const vbr_pc_addr = cpu->cpu_regs.vbr + (imm << 2);
    cpu_rdwr_ex = sh2e_cpu_read_long(cpu, vbr_pc_addr, &vbr_pc);
    if (cpu_rdwr_ex != SH2E_EXCEPTION_NONE) {
        return cpu_rdwr_ex;
    }

    cpu->cpu_regs.sp = stack_pc_addr;

    // Execute the branch immediately.
    cpu->pc_next = vbr_pc;
    cpu->br_state = SH2E_BRANCH_STATE_EXECUTE;

    return SH2E_EXCEPTION_NONE;
}

// TST (Test Logical): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_tst(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const test = cpu->cpu_regs.general[insn.rn] & cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.sr.t = (test == 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_tsti(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t const test = cpu->cpu_regs.r0 & zero_extend_8_32(insn.i8);
    cpu->cpu_regs.sr.t = (test == 0) ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_tstm(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t value;
    uint32_t const addr = cpu->cpu_regs.gbr + cpu->cpu_regs.r0;
    sh2e_exception_t cpu_read_ex = sh2e_cpu_readz_byte(cpu, addr, &value);
    if (cpu_read_ex == SH2E_EXCEPTION_NONE) {
        uint32_t const test = value & zero_extend_8_32(insn.i8);
        cpu->cpu_regs.sr.t = (test == 0) ? 1 : 0;
    }

    return cpu_read_ex;
}

// XOR (Exclusive OR Logical): Logic Operation Instruction

sh2e_exception_t
sh2e_insn_exec_xor(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->cpu_regs.general[insn.rn] ^= cpu->cpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_xori(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    cpu->cpu_regs.r0 ^= zero_extend_8_32(insn.i8);
    return SH2E_EXCEPTION_NONE;
}


sh2e_exception_t
sh2e_insn_exec_xorm(sh2e_cpu_t * const restrict cpu, sh2e_insn_i_t const insn) {
    uint32_t value;
    uint32_t const addr = cpu->cpu_regs.gbr + cpu->cpu_regs.r0;
    sh2e_exception_t cpu_rdwr_ex = sh2e_cpu_readz_byte(cpu, addr, &value);
    if (cpu_rdwr_ex == SH2E_EXCEPTION_NONE) {
        uint32_t const result = value ^ zero_extend_8_32(insn.i8);
        cpu_rdwr_ex = sh2e_cpu_write_byte(cpu, addr, result);
    }

    return cpu_rdwr_ex;
}

// XTRCT (Extract): Data Transfer Instruction

sh2e_exception_t
sh2e_insn_exec_xtrct(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const rn = cpu->cpu_regs.general[insn.rn];
    uint32_t const rm = cpu->cpu_regs.general[insn.rm];
    cpu->cpu_regs.general[insn.rn] = (rm << 16) | (rn >> 16);
    return SH2E_EXCEPTION_NONE;
}


/****************************************************************************
 * SH-2E FPU instructions
 ****************************************************************************/

static ALWAYS_INLINE bool
__is_snan(float32_t const value) {
    float32_bits_t const value_bits = { .fvalue = value };
    return (value_bits.exponent == UINT32_C(0xff)) && (value_bits.mantissa & UINT32_C(1 << 22)) != 0;
}


static ALWAYS_INLINE float
__qnan(uint16_t const bits) {
    return ({
        float32_bits_t const _qnan = {
            .sign = 0, .exponent = 0xff, .mantissa = 1 << 21 | bits
        };
        _qnan.fvalue;
    });
}


static ALWAYS_INLINE void
sh2e_cpu_clear_cv_cz(sh2e_cpu_t * const restrict cpu) {
    cpu->fpu_regs.fpscr.cv = 0;
    cpu->fpu_regs.fpscr.cz = 0;
}


static ALWAYS_INLINE void
sh2e_cpu_set_cv_fv(sh2e_cpu_t * const restrict cpu) {
    cpu->fpu_regs.fpscr.cv = 1;
    cpu->fpu_regs.fpscr.fv = 1;
}


// FABS (Floating Point Absolute Value): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fabs(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];

    float32_t result = fabsf(frn);
    if (isnan(result) && __is_snan(frn)) {
        // FABS special cases (REJ09B0316-0200, page 170).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = __qnan(insn.word);
    }

    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}

// FADD (Floating Point Add): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fadd(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    float32_t result = frn + frm;
    if (isnan(result) && (
        // Any of FRn and FRm is a signaling NaN.
        __is_snan(frn) || __is_snan(frm) ||
        // Both FRn and FRm are infinite but with different sign.
        (isinf(frn) * isinf(frm) < 0)
    )) {
        // FADD special cases (REJ09B0316-0200, page 173).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = __qnan(insn.word);
    }

    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}


// FCMP (Floating Point Compare): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fcmpeq(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    bool const result = frn == frm;
    if (!result && (
        // Any of FRn and FRm is a signaling NaN.
        __is_snan(frn) || __is_snan(frm)
    )) {
        // FCMP/EQ special cases (REJ09B0316-0200, page 177).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }
    }

    cpu->cpu_regs.sr.t = result ? 1 : 0;
    return SH2E_EXCEPTION_NONE;
}

sh2e_exception_t
sh2e_insn_exec_fcmpgt(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    uint32_t const result = isgreater(frn, frm);
    if (result == 0 && (
        // Any of FRn and FRm is a NaN.
        isunordered(frn, frm) == 1
    )) {
        // FCMP/GT special cases (REJ09B0316-0200, page 177).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }
    }

    cpu->cpu_regs.sr.t = result;
    return SH2E_EXCEPTION_NONE;
}

// FDIV (Floating Point Divide): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fdiv(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];
    float32_t const result = frn / frm;

    // TODO Handle exceptions.
    // FDIV special cases (REJ09B0316-0200, page 180).
    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}

// FLDI0 (Floating Point Load Immediate 0): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fldi0(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    cpu->fpu_regs.general[insn.rn] = 0.0f;
    return SH2E_EXCEPTION_NONE;
}

// FLDI1 (Floating Point Load Immediate 1): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fldi1(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    cpu->fpu_regs.general[insn.rn] = 1.0f;
    return SH2E_EXCEPTION_NONE;
}

// FLDS (Floating Point Load to System Register): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_flds(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    cpu->fpu_regs.fpul.fvalue = cpu->fpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}

// FLOAT (Floating Point Convert from Integer): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_float(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);

    // No exceptions are raised.
    cpu->fpu_regs.general[insn.rn] = (float) cpu->fpu_regs.fpul.ivalue;
    return SH2E_EXCEPTION_NONE;
}

// FMAC (Floating Point Multiply Accumulate): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fmac(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const fr0 = cpu->fpu_regs.fr0;
    float32_t const frm = cpu->fpu_regs.general[insn.rm];
    float32_t const frn = cpu->fpu_regs.general[insn.rn];

    // TODO Handle exceptions.
    cpu->fpu_regs.general[insn.rn] = fmaf(fr0, frm, frn);
    return SH2E_EXCEPTION_NONE;
}

// FMOV (Floating Point Move): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fmov(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    cpu->fpu_regs.general[insn.rn] = cpu->fpu_regs.general[insn.rm];
    return SH2E_EXCEPTION_NONE;
}


static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_fmovl(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn, uint32_t const base) {
    uint32_t const addr = base + cpu->cpu_regs.general[insn.rm];
    return sh2e_cpu_read_float(cpu, addr, &cpu->fpu_regs.general[insn.rn]);
}

sh2e_exception_t
sh2e_insn_exec_fmovl(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_fmovl(cpu, insn, 0);
}

sh2e_exception_t
sh2e_insn_exec_fmovli(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_fmovl(cpu, insn, cpu->cpu_regs.r0);
}


sh2e_exception_t
sh2e_insn_exec_fmovlr(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const addr = cpu->cpu_regs.general[insn.rm];
    float32_t * const frn = &cpu->fpu_regs.general[insn.rn];

    sh2e_exception_t const cpu_read_ex = sh2e_cpu_read_float(cpu, addr, frn);
    if (cpu_read_ex == SH2E_EXCEPTION_NONE) {
        // Post-increment Rm on success.
        cpu->cpu_regs.general[insn.rm] = addr + sizeof(uint32_t);
    }

    return cpu_read_ex;
}


static ALWAYS_INLINE sh2e_exception_t
sh2e_insn_fmovs(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn, uint32_t const base) {
    uint32_t const addr = base + cpu->cpu_regs.general[insn.rn];
    return sh2e_cpu_write_float(cpu, addr, cpu->fpu_regs.general[insn.rm]);
}

sh2e_exception_t
sh2e_insn_exec_fmovs(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_fmovs(cpu, insn, 0);
}

sh2e_exception_t
sh2e_insn_exec_fmovsi(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    return sh2e_insn_fmovs(cpu, insn, cpu->cpu_regs.r0);
}


sh2e_exception_t
sh2e_insn_exec_fmovss(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    uint32_t const addr = cpu->cpu_regs.general[insn.rn] - sizeof(uint32_t);
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    sh2e_exception_t const cpu_write_ex = sh2e_cpu_write_float(cpu, addr, frm);
    if (cpu_write_ex == SH2E_EXCEPTION_NONE) {
        // Store pre-decremented Rn on success.
        cpu->cpu_regs.general[insn.rn] = addr;
    }

    return cpu_write_ex;
}

// FMUL (Floating Point Multiply): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fmul(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    // TODO Handle exceptions.
    float32_t result = frn * frm;
    if (isnan(result) && (
        // Any of FRn and FRm is a signaling NaN.
        __is_snan(frn) || __is_snan(frm) ||
        // Either of FRn or FRm is infinity and the other is zero.
        (isinf(frn) && fpclassify(frm) == FP_ZERO) ||
        (fpclassify(frn) == FP_ZERO && isinf(frm))
    )) {
        // FMUL special cases (REJ09B0316-0200, page 193).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = __qnan(insn.word);
    }

    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}

// FNEG (Floating Point Negate): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fneg(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];

    float32_t result = -frn;
    if (isnan(result) && __is_snan(frn)) {
        // FNEG special cases (REJ09B0316-0200, page 194).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = __qnan(insn.word);
    }

    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}

// FSTS (Floating Point Store From System Register): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fsts(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    cpu->fpu_regs.general[insn.rn] = cpu->fpu_regs.fpul.fvalue;
    return SH2E_EXCEPTION_NONE;
}

// FSUB (Floating Point Subtract): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_fsub(sh2e_cpu_t * const restrict cpu, sh2e_insn_nm_t const insn) {
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frn = cpu->fpu_regs.general[insn.rn];
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    float32_t result = frn - frm;
    if (isnan(result) && (
        // Any of FRn and FRm is a signaling NaN.
        __is_snan(frn) || __is_snan(frm) ||
        // Both FRn and FRm are infinite with the same sign.
        (isinf(frn) * isinf(frm) == 1)
    )) {
        // FSUB special cases (REJ09B0316-0200, page 197).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = __qnan(insn.word);
    }

    cpu->fpu_regs.general[insn.rn] = result;
    return SH2E_EXCEPTION_NONE;
}

// FTRC (Floating Point Truncate And Convert To Integer): Floating Point Instruction

sh2e_exception_t
sh2e_insn_exec_ftrc(sh2e_cpu_t * const restrict cpu, sh2e_insn_m_t const insn) {
    //
    // Converts the contents of the floating point register FRm to an integer
    // by truncating everything after the decimal point. Non-normalized values
    // are treated as zero. The result is stored as an integer in FPUL.
    //
    sh2e_cpu_clear_cv_cz(cpu);
    float32_t const frm = cpu->fpu_regs.general[insn.rm];

    uint32_t result = MIN(MAX(INT32_MIN, (int64_t) truncf(frm)), INT32_MAX);
    if (!isfinite(frm)) {
        // FTRC special cases (REJ09B0316-0200, page 200).
        sh2e_cpu_set_cv_fv(cpu);
        if (cpu->fpu_regs.fpscr.ev) {
            return SH2E_EXCEPTION_FPU_OPERATION;
        }

        result = isinf(frm) > 0 ? INT32_MAX : INT32_MIN;
    }

    cpu->fpu_regs.fpul.ivalue = result;
    return SH2E_EXCEPTION_NONE;
}

// STS (Store from FPU System Register): FPU Related CPU Instruction

sh2e_exception_t
sh2e_insn_exec_sts_fpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsr(cpu, insn, cpu->fpu_regs.system);
}

sh2e_exception_t
sh2e_insn_exec_stsm_fpu(sh2e_cpu_t * const restrict cpu, sh2e_insn_n_t const insn) {
    return sh2e_insn_stcsrm(cpu, insn, cpu->fpu_regs.system);
}

//

sh2e_exception_t
sh2e_insn_exec_illegal(sh2e_cpu_t * const restrict cpu, sh2e_insn_t const insn) {
    alert("instruction 0x%04x is invalid", insn.word);
    return SH2E_EXCEPTION_ILLEGAL_INSTRUCTION;
}

sh2e_exception_t
sh2e_insn_exec_not_implemented(sh2e_cpu_t * const restrict cpu, sh2e_insn_t const insn) {
    alert("instruction 0x%04x not implemented", insn.word);
    return SH2E_EXCEPTION_ILLEGAL_INSTRUCTION;
}
