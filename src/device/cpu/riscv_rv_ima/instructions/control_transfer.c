/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Jump and branch instructions
 *
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#include <stdint.h>

#include "../../../../assert.h"
#include "../../../../utils.h"
#include "../exception.h"
#include "../instr.h"
#include "../types.h"

rv_exc_t rv_read_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t *value, bool noisy);
rv_exc_t rv_read_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t *value, bool fetch, bool noisy);
rv_exc_t rv_write_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t value, bool noisy);
rv_exc_t rv_write_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t value, bool noisy);
rv_exc_t rv_write_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t value, bool noisy);
rv_exc_t rv_write_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t value, bool noisy);
rv_exc_t rv_convert_addr(rv_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);

static rv_exc_t rv_jal_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.j.opcode == rv_opcJAL);

    // jump target is relative to the address of the instruction eg. pc
    uxlen_t target = cpu->pc + RV_J_IMM(instr);

    if (!IS_ALIGNED(target, 4)) {
        cpu->csr.tval_next = target;
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    cpu->pc_next = target;
    return rv_exc_none;
}

static rv_exc_t rv_jalr_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcJALR);
    ASSERT(instr.i.funct3 == 0);

    uxlen_t target = cpu->regs[instr.i.rs1] + instr.i.imm;
    // lowest bit set to 0, as described in the specification
    target &= ~1;

    if (!IS_ALIGNED(target, 4)) {
        cpu->csr.tval_next = target;
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    cpu->pc_next = target;
    return rv_exc_none;
}

static rv_exc_t rv_beq_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    uxlen_t lhs = cpu->regs[instr.b.rs1];
    uxlen_t rhs = cpu->regs[instr.b.rs2];

    if (lhs == rhs) {
        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }

        cpu->pc_next = target;
    }

    return rv_exc_none;
}

static rv_exc_t rv_bne_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    uxlen_t lhs = cpu->regs[instr.b.rs1];
    uxlen_t rhs = cpu->regs[instr.b.rs2];

    if (lhs != rhs) {
        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }

        cpu->pc_next = target;
    }

    return rv_exc_none;
}

static rv_exc_t rv_blt_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    xlen_t lhs = (xlen_t) cpu->regs[instr.b.rs1];
    xlen_t rhs = (xlen_t) cpu->regs[instr.b.rs2];

    if (lhs < rhs) {
        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

static rv_exc_t rv_bltu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    uxlen_t lhs = cpu->regs[instr.b.rs1];
    uxlen_t rhs = cpu->regs[instr.b.rs2];

    if (lhs < rhs) {

        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

static rv_exc_t rv_bge_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    xlen_t lhs = (xlen_t) cpu->regs[instr.b.rs1];
    xlen_t rhs = (xlen_t) cpu->regs[instr.b.rs2];

    if (lhs >= rhs) {
        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

static rv_exc_t rv_bgeu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uxlen_t target = cpu->pc + RV_B_IMM(instr);

    uxlen_t lhs = cpu->regs[instr.b.rs1];
    uxlen_t rhs = cpu->regs[instr.b.rs2];

    if (lhs >= rhs) {
        if (!IS_ALIGNED(target, 4)) {
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}
