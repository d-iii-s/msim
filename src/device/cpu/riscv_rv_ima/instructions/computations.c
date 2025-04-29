/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V arithmetic instructions
 *
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#include <stdint.h>

#include "../../../../assert.h"
#include "../../../../utils.h"
#include "../exception.h"
#include "../instr.h"

rv_exc_t rv_read_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t *value, bool noisy);
rv_exc_t rv_read_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t *value, bool fetch, bool noisy);
rv_exc_t rv_read_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t *value, bool fetch, bool noisy);
rv_exc_t rv_write_mem8(rv_cpu_t *cpu, virt_t virt, uint8_t value, bool noisy);
rv_exc_t rv_write_mem16(rv_cpu_t *cpu, virt_t virt, uint16_t value, bool noisy);
rv_exc_t rv_write_mem32(rv_cpu_t *cpu, virt_t virt, uint32_t value, bool noisy);
rv_exc_t rv_write_mem64(rv_cpu_t *cpu, virt_t virt, uint64_t value, bool noisy);
rv_exc_t rv_convert_addr(rv_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);

/******
 * OP *
 ******/

static rv_exc_t rv_add_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs + rhs;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_addw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t) cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs + rhs));

    return rv_exc_none;
}

static rv_exc_t rv_sub_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs - rhs;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_subw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t) cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs - rhs));

    return rv_exc_none;
}

static rv_exc_t rv_sll_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 6 bits
    uxlen_t rhs = shift_instr_mask(XLEN) & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs << rhs;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_sllw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    uint32_t lhs = (uint32_t) cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    uint32_t result = lhs << rhs;

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) result);

    return rv_exc_none;
}

static rv_exc_t rv_slt_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = cpu->regs[instr.r.rs1];
    xlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}

static rv_exc_t rv_sltu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}

static rv_exc_t rv_xor_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs ^ rhs;

    return rv_exc_none;
}

static rv_exc_t rv_srl_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 6 bits
    uxlen_t rhs = shift_instr_mask(XLEN) & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs >> rhs;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_srlw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    uint32_t lhs = (uint32_t) cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    uint32_t result = lhs >> rhs;

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) result);

    return rv_exc_none;
}

static rv_exc_t rv_sra_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = (xlen_t) cpu->regs[instr.r.rs1];
    // based only on lowest 6 bits
    uxlen_t rhs = shift_instr_mask(XLEN) & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = (uxlen_t) (lhs >> rhs);

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_sraw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    int32_t result = (lhs >> rhs);

    cpu->regs[instr.r.rd] = (int64_t) result;

    return rv_exc_none;
}

static rv_exc_t rv_or_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs | rhs;

    return rv_exc_none;
}

static rv_exc_t rv_and_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs & rhs;

    return rv_exc_none;
}

/**********
 * OP-IMM *
 **********/

static rv_exc_t rv_addi_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    xlen_t imm;
    if (XLEN == 32) {
        imm = (int32_t) (instr.i.imm << 20) >> 20;
    } else {
        imm = (int64_t) ((int32_t) (instr.i.imm << 20) >> 20);
    }

    cpu->regs[instr.i.rd] = cpu->regs[instr.i.rs1] + imm;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_addiw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM_32);

    int32_t imm = (int32_t) (instr.i.imm << 20) >> 20;

    int32_t result = (int32_t) cpu->regs[instr.i.rs1] + imm;

    cpu->regs[instr.i.rd] = (int64_t) result;

    return rv_exc_none;
}

static rv_exc_t rv_slti_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    xlen_t imm = (xlen_t) ((int32_t) (instr.i.imm << 20) >> 20);

    bool cmp = ((xlen_t) cpu->regs[instr.i.rs1] < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

static rv_exc_t rv_sltiu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    // sign extend to 64 bits, then change to unsigned
    uxlen_t imm = (uxlen_t) ((xlen_t) ((int32_t) (instr.i.imm << 20) >> 20));

    bool cmp = ((cpu->regs[instr.i.rs1]) < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

static rv_exc_t rv_andi_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    xlen_t imm = (xlen_t) ((int32_t) (instr.i.imm << 20) >> 20);

    uxlen_t val = cpu->regs[instr.i.rs1] & imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

static rv_exc_t rv_ori_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    xlen_t imm = (xlen_t) ((int32_t) (instr.i.imm << 20) >> 20);

    uxlen_t val = cpu->regs[instr.i.rs1] | imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

static rv_exc_t rv_xori_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    xlen_t imm = (xlen_t) ((int32_t) (instr.i.imm << 20) >> 20);

    uxlen_t val = cpu->regs[instr.i.rs1] ^ imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

// TODO: figure this out
static rv_exc_t rv_slli_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & 0x3F;

    uint64_t val = cpu->regs[instr.i.rs1] << imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_slliw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM_32);

    uint32_t imm = instr.i.imm & 0x1F;

    uint32_t val = (uint32_t) cpu->regs[instr.i.rs1] << imm;

    cpu->regs[instr.i.rd] = (int64_t) ((int32_t) val);

    return rv_exc_none;
}

// TODO: figure this out
static rv_exc_t rv_srli_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & 0x3F;

    uint64_t val = cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_srliw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM_32);

    uint32_t imm = instr.i.imm & 0x1F;

    uint32_t val = (uint32_t) cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = (int64_t) ((int32_t) val);

    return rv_exc_none;
}

// TODO: figure this out
static rv_exc_t rv_srai_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & 0x3F;

    int64_t val = (int64_t) cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = (uint64_t) val;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_sraiw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM_32);

    uint32_t imm = instr.i.imm & 0x1F;

    int32_t val = (int32_t) cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = (int64_t) val;

    return rv_exc_none;
}

/*****************
 * LUI and AUIPC *
 *****************/

static rv_exc_t rv_lui_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcLUI);

    xlen_t imm_val = sign_extend_32_to_xlen(instr.u.imm << 12, XLEN);

    cpu->regs[instr.u.rd] = imm_val;

    return rv_exc_none;
}

static rv_exc_t rv_auipc_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcAUIPC);

    xlen_t offset = sign_extend_32_to_xlen(instr.u.imm << 12, XLEN);

    uxlen_t val = cpu->pc + offset;

    cpu->regs[instr.u.rd] = val;

    return rv_exc_none;
}

/***************
 * M extension *
 ***************/

static rv_exc_t rv_mul_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs * rhs;

    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_mulw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t) cpu->regs[instr.r.rs2];

    int32_t result = lhs * rhs;

    cpu->regs[instr.r.rd] = (int64_t) result;

    return rv_exc_none;
}

static rv_exc_t rv_mulh_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = (xlen_t) cpu->regs[instr.r.rs1];
    xlen_t rhs = (xlen_t) cpu->regs[instr.r.rs2];

    bigxlen_t res = (bigxlen_t) lhs * (bigxlen_t) rhs;

    cpu->regs[instr.r.rd] = (uxlen_t) (res >> XLEN);
    return rv_exc_none;
}

static rv_exc_t rv_mulhsu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = (xlen_t) cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    bigxlen_t res = (bigxlen_t) lhs * (bigxlen_t) rhs;

    cpu->regs[instr.r.rd] = (uxlen_t) (res >> XLEN);
    return rv_exc_none;
}

static rv_exc_t rv_mulhu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    ubigxlen_t res = (ubigxlen_t) lhs * (ubigxlen_t) rhs;

    cpu->regs[instr.r.rd] = (uxlen_t) (res >> XLEN);
    return rv_exc_none;
}

static rv_exc_t rv_div_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = (xlen_t) cpu->regs[instr.r.rs1];
    xlen_t rhs = (xlen_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the result to -1
        cpu->regs[instr.r.rd] = -1;
        return rv_exc_none;
    }

    if (lhs == xlen_min(XLEN) && rhs == -1) {
        // as per spec, divide overflow causes the result to be the minimal XLEN
        cpu->regs[instr.r.rd] = xlen_min(XLEN);
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs / rhs;
    return rv_exc_none;
}

static rv_exc_t rv_divu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the result to the maximal val
        cpu->regs[instr.r.rd] = uxlen_max(XLEN);
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs / rhs;
    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_divw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    // Truncate to 32 bits
    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the result to -1
        cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (-1));
        return rv_exc_none;
    }

    if (lhs == INT32_MIN && rhs == -1) {
        // as per spec, divide overflow causes the result to be the minimal int32
        cpu->regs[instr.r.rd] = (int64_t) INT32_MIN;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs / rhs));
    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_divuw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    uint32_t lhs = (uint32_t) cpu->regs[instr.r.rs1];
    uint32_t rhs = (uint32_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the result to the maximal val
        cpu->regs[instr.r.rd] = (int64_t) ((int32_t) UINT32_MAX);
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs / rhs));
    return rv_exc_none;
}

static rv_exc_t rv_rem_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    xlen_t lhs = (xlen_t) cpu->regs[instr.r.rs1];
    xlen_t rhs = (xlen_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the remained to the original value
        cpu->regs[instr.r.rd] = lhs;
        return rv_exc_none;
    }

    if (lhs == xlen_min(XLEN) && rhs == -1) {
        // as per spec, divide overflow causes the remainder to be set to 0
        cpu->regs[instr.r.rd] = 0;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs % rhs;
    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_remw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    int32_t lhs = (int32_t) cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the remained to the original value
        cpu->regs[instr.r.rd] = (int64_t) lhs;
        return rv_exc_none;
    }

    if (lhs == INT32_MIN && rhs == -1) {
        // as per spec, divide overflow causes the remainder to be set to 0
        cpu->regs[instr.r.rd] = 0;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs % rhs));
    return rv_exc_none;
}

static rv_exc_t rv_remu_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uxlen_t lhs = cpu->regs[instr.r.rs1];
    uxlen_t rhs = cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the remainder to the original value
        cpu->regs[instr.r.rd] = lhs;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs % rhs;
    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_remuw_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP_32);

    uint32_t lhs = (uint32_t) cpu->regs[instr.r.rs1];
    uint32_t rhs = (uint32_t) cpu->regs[instr.r.rs2];

    if (rhs == 0) {
        // as per spec, dividing by 0 sets the remainder to the original value
        cpu->regs[instr.r.rd] = (int64_t) ((int32_t) lhs);
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = (int64_t) ((int32_t) (lhs % rhs));
    return rv_exc_none;
}

/* A extension atomic operations */

#define throw_ex(cpu, virt, ex) \
    { \
        cpu->csr.tval_next = virt; \
        return ex; \
    }

#define throw_if_wrong_privilege(cpu, virt) \
    { \
        ptr36_t _; \
        rv_exc_t ex; \
        ex = rv_convert_addr(cpu, virt, &_, true, false, true); \
        if (ex != rv_exc_none) { \
            throw_ex(cpu, virt, ex); \
        } \
    }
#define throw_if_misaligned_word(cpu, virt) \
    { \
        if (!IS_ALIGNED(virt, 4)) { \
            throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned); \
        } \
    }
#define throw_if_misaligned_dword(cpu, virt) \
    { \
        if (!IS_ALIGNED(virt, 8)) { \
            throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned); \
        } \
    }

static rv_exc_t rv_amoswap_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;

    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    ex = rv_write_mem32(cpu, virt, (uint32_t) cpu->regs[instr.r.rs2], true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    return rv_exc_none;
}

/** RV64 ONLY */
static rv_exc_t rv_amoswap_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;

    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    ex = rv_write_mem64(cpu, virt, cpu->regs[instr.r.rs2], true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    return rv_exc_none;
}

static rv_exc_t rv_amoadd_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    // load from mem
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    // save loaded value to rd
    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    // add with rs2
    val += (uint32_t) cpu->regs[instr.r.rs2];

    //  write to mem
    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amoadd_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;
    // load from mem
    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    // save loaded value to rd
    cpu->regs[instr.r.rd] = val;
    // add with rs2
    val += cpu->regs[instr.r.rs2];

    //  write to mem
    ex = rv_write_mem64(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

static rv_exc_t rv_amoxor_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    val ^= (uint32_t) cpu->regs[instr.r.rs2];
    ex = rv_write_mem32(cpu, virt, val, true);

    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amoxor_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;
    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    val ^= cpu->regs[instr.r.rs2];
    ex = rv_write_mem64(cpu, virt, val, true);

    ASSERT(ex == rv_exc_none);
    return ex;
}

static rv_exc_t rv_amoand_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    val &= (uint32_t) cpu->regs[instr.r.rs2];

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amoand_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;

    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;

    val &= cpu->regs[instr.r.rs2];

    ex = rv_write_mem64(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);

    return ex;
}

static rv_exc_t rv_amoor_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    val |= (uint32_t) cpu->regs[instr.r.rs2];

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amoor_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;

    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;

    val |= cpu->regs[instr.r.rs2];

    ex = rv_write_mem64(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);

    return ex;
}

static rv_exc_t rv_amomin_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    int32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, (uint32_t *) &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    int32_t rs2 = (int32_t) cpu->regs[instr.r.rs2];
    val = rs2 < val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, (uint32_t) val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amomin_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    int64_t val;
    rv_exc_t ex = rv_read_mem64(cpu, virt, (uint64_t *) &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    int64_t rs2 = (int64_t) cpu->regs[instr.r.rs2];
    val = rs2 < val ? rs2 : val;

    ex = rv_write_mem64(cpu, virt, (uint64_t) val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

static rv_exc_t rv_amomax_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    int32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, (uint32_t *) &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = sign_extend_32_to_xlen(val, XLEN);
    int32_t rs2 = (int32_t) cpu->regs[instr.r.rs2];
    val = rs2 > val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, (uint32_t) val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amomax_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    int64_t val;
    rv_exc_t ex = rv_read_mem64(cpu, virt, (uint64_t *) &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    int64_t rs2 = (int64_t) cpu->regs[instr.r.rs2];
    val = rs2 > val ? rs2 : val;

    ex = rv_write_mem64(cpu, virt, (uint64_t) val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

static rv_exc_t rv_amominu_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = zero_extend_32_to_xlen(val, XLEN);
    uint32_t rs2 = (uint32_t) cpu->regs[instr.r.rs2];
    val = rs2 < val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amominu_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;
    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    uint64_t rs2 = cpu->regs[instr.r.rs2];
    val = rs2 < val ? rs2 : val;

    ex = rv_write_mem64(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

static rv_exc_t rv_amomaxu_w_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_word(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = zero_extend_32_to_xlen(val, XLEN);
    uint32_t rs2 = (uint32_t) cpu->regs[instr.r.rs2];
    val = rs2 > val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

/** RV64 ONLY */
static rv_exc_t rv_amomaxu_d_instr(rv_cpu_t *cpu, rv_instr_t instr)
{
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uxlen_t virt = cpu->regs[instr.r.rs1];

    throw_if_wrong_privilege(cpu, virt);
    throw_if_misaligned_dword(cpu, virt);

    uint64_t val;
    rv_exc_t ex = rv_read_mem64(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    uint64_t rs2 = cpu->regs[instr.r.rs2];
    val = rs2 > val ? rs2 : val;

    ex = rv_write_mem64(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}
