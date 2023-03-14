/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V arithmetic instructions
 *
 */

#include <stdint.h>
#include "computations.h"
#include "../../../../assert.h"
#include "../../../../utils.h"

/******
 * OP *
 ******/

rv_exc_t rv_add_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs + rhs;

    return rv_exc_none;
}

rv_exc_t rv_sub_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs - rhs;

    return rv_exc_none;
}
rv_exc_t rv_sll_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs << rhs;

    return rv_exc_none;
}
rv_exc_t rv_slt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = cpu->regs[instr.r.rs1];
    int32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}
rv_exc_t rv_sltu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}
rv_exc_t rv_xor_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs ^ rhs;

    return rv_exc_none;
}
rv_exc_t rv_srl_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs >> rhs;

    return rv_exc_none;
}
rv_exc_t rv_sra_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    int32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = (uint32_t)(lhs >> rhs);

    return rv_exc_none;
}
rv_exc_t rv_or_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs | rhs;

    return rv_exc_none;
}
rv_exc_t rv_and_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs & rhs;

    return rv_exc_none;
}

/**********
 * OP-IMM *
 **********/

rv_exc_t rv_addi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    int32_t val = cpu->regs[instr.i.rs1] + imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_slti_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    bool cmp = ((int32_t)(cpu->regs[instr.i.rs1]) < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t rv_sltiu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    // sign extend to 32 bits, then change to unsigned
    uint32_t imm = (int32_t)instr.i.imm;

    bool cmp = ((cpu->regs[instr.i.rs1]) < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t rv_andi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] & imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_ori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] | imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_xori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] ^ imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_slli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] << imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_srli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_srai_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (int32_t)cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

/*****************
 * LUI and AUIPC *
 *****************/

rv_exc_t rv_lui_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcLUI);

    cpu->regs[instr.u.rd] = instr.u.imm << 12;

    return rv_exc_none;
}

rv_exc_t rv_auipc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcAUIPC);

    int32_t offset = instr.u.imm << 12;

    uint32_t val = cpu->pc + offset;

    cpu->regs[instr.u.rd] = val;

    return rv_exc_none;
}

/***************
 * M extension *
 ***************/

extern rv_exc_t rv_mul_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs * rhs;

    return rv_exc_none;
}

extern rv_exc_t rv_mulh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int64_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    int64_t rhs = (int32_t)cpu->regs[instr.r.rs2];

    int64_t res = lhs * rhs;

    res = res >> 32;

    cpu->regs[instr.r.rd] = (uint32_t)res;
    return rv_exc_none;
}

extern rv_exc_t rv_mulhsu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int64_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    uint64_t rhs = cpu->regs[instr.r.rs2];

    int64_t res = lhs * rhs;

    res = res >> 32;

    cpu->regs[instr.r.rd] = (uint32_t)res;
    return rv_exc_none;
}

extern rv_exc_t rv_mulhu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint64_t lhs = cpu->regs[instr.r.rs1];
    uint64_t rhs = cpu->regs[instr.r.rs2];

    uint64_t res = lhs * rhs;

    res = res >> 32;

    cpu->regs[instr.r.rd] = (uint32_t)res;
    return rv_exc_none;
}

extern rv_exc_t rv_div_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t)cpu->regs[instr.r.rs2];

    if(rhs == 0){
        // as per spec, dividing by 0 sets the result to -1
        cpu->regs[instr.r.rd] = -1;
        return rv_exc_none;
    }

    if(lhs == INT32_MIN && rhs == -1){
        // as per spec, divide overflow causes the result to be the minimal int32
        cpu->regs[instr.r.rd] = INT32_MIN;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs / rhs;
    return rv_exc_none;
}

extern rv_exc_t rv_divu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    if(rhs == 0){
        // as per spec, dividing by 0 sets the result to the maximal val
        cpu->regs[instr.r.rd] = UINT32_MAX;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs / rhs;
    return rv_exc_none;
}

extern rv_exc_t rv_rem_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    int32_t rhs = (int32_t)cpu->regs[instr.r.rs2];

    if(rhs == 0){
        // as per spec, dividing by 0 sets the remained to the original value
        cpu->regs[instr.r.rd] = lhs;
        return rv_exc_none;
    }

    if(lhs == INT32_MIN && rhs == -1){
        // as per spec, divide overflow causes the remainder to be set to 0
        cpu->regs[instr.r.rd] = 0;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs % rhs;
    return rv_exc_none;
}

extern rv_exc_t rv_remu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    if(rhs == 0){
        // as per spec, dividing by 0 sets the remainder to the original value
        cpu->regs[instr.r.rd] = lhs;
        return rv_exc_none;
    }

    cpu->regs[instr.r.rd] = lhs % rhs;
    return rv_exc_none;
}

/* A extension atomic operations */

#define throw_ex(cpu, virt, ex) {   \
        cpu->csr.tval_next = virt;  \
        return ex;                  \
    }

#define throw_if_wrong_privilege(cpu, virt) {               \
    ptr36_t _; rv_exc_t ex;                                 \
    ex = rv_convert_addr(cpu, virt, &_, true, false, true); \
    if(ex != rv_exc_none){                                  \
        throw_ex(cpu, virt, ex);                            \
    }                                                       \
}
#define throw_if_misaligned(cpu, virt) {                            \
    if(!IS_ALIGNED(virt, 4)){                                       \
        throw_ex(cpu, virt, rv_exc_store_amo_address_misaligned);   \
    }                                                               \
}


rv_exc_t rv_amoswap_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    ex = rv_write_mem32(cpu, virt, cpu->regs[instr.r.rs2], true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;   
    return rv_exc_none;
}

rv_exc_t rv_amoadd_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    // load from mem
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    // save loaded value to rd
    cpu->regs[instr.r.rd] = val;
    // add with rs2
    val += cpu->regs[instr.r.rs2];
    
    //  write to mem
    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
    
}

rv_exc_t rv_amoxor_instr(rv_cpu_t *cpu, rv_instr_t instr) {

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    val ^= cpu->regs[instr.r.rs2];
    ex = rv_write_mem32(cpu, virt, val, true);

    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amoand_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    val &= cpu->regs[instr.r.rs2];

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amoor_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    val |= cpu->regs[instr.r.rs2];

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amomin_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    int32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, (uint32_t*)&val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    int32_t rs2 = cpu->regs[instr.r.rs2];
    val =  rs2 < val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amomax_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];
    
    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    int32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, (uint32_t*)&val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    int32_t rs2 = cpu->regs[instr.r.rs2];
    val =  rs2 > val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amominu_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);

    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    uint32_t rs2 = cpu->regs[instr.r.rs2];
    val =  rs2 < val ? rs2 : val;

    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}

rv_exc_t rv_amomaxu_instr(rv_cpu_t *cpu, rv_instr_t instr){

    uint32_t virt = cpu->regs[instr.r.rs1];

    // Check write privileges first
    throw_if_wrong_privilege(cpu, virt);
    // Then alignment
    throw_if_misaligned(cpu, virt);
    
    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);
    ASSERT(ex == rv_exc_none);

    cpu->regs[instr.r.rd] = val;
    uint32_t rs2 = cpu->regs[instr.r.rs2];
    val =  rs2 > val ? rs2 : val;
    
    ex = rv_write_mem32(cpu, virt, val, true);
    ASSERT(ex == rv_exc_none);
    return ex;
}
