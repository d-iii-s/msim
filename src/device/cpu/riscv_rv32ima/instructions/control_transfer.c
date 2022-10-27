/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Jump and branch instructions
 *
 */

#include "control_transfer.h"
#include "../../../../assert.h"
#include "../../../../utils.h"

rv_exc_t jal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.j.opcode == rv_opcJAL);

    // jump target is relative to the address of the instruction eg. pc
    uint32_t target = cpu->pc + RV_J_IMM(instr);

    if(!IS_ALIGNED(target, 4)){
        cpu->csr.tval_next = target;
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    cpu->pc_next = target;
    return rv_exc_none;
}

rv_exc_t jalr_instr(rv_cpu_t *cpu, rv_instr_t instr) {
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcJALR);
    ASSERT(instr.i.funct3 == 0);

    uint32_t target = cpu->regs[instr.i.rs1] + instr.i.imm;
    // lowest bit set to 0, as described in the specification
    target &= ~1;

    if(!IS_ALIGNED(target, 4)){
        cpu->csr.tval_next = target;
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    cpu->pc_next = target;
    return rv_exc_none;
}

rv_exc_t beq_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    uint32_t lhs = cpu->regs[instr.b.rs1];
    uint32_t rhs = cpu->regs[instr.b.rs2];

    if(lhs == rhs) {
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }

        cpu->pc_next = target;
    }

    return rv_exc_none;
}

rv_exc_t bne_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    uint32_t lhs = cpu->regs[instr.b.rs1];
    uint32_t rhs = cpu->regs[instr.b.rs2];

    if(lhs != rhs) {
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }

        cpu->pc_next = target;
    }

    return rv_exc_none;
}

rv_exc_t blt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    int32_t lhs = (int32_t)cpu->regs[instr.b.rs1];
    int32_t rhs = (int32_t)cpu->regs[instr.b.rs2];

    if(lhs < rhs) {
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

rv_exc_t bltu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    uint32_t lhs = cpu->regs[instr.b.rs1];
    uint32_t rhs = cpu->regs[instr.b.rs2];

    if(lhs < rhs) {
        
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

rv_exc_t bge_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    int32_t lhs = (int32_t)cpu->regs[instr.b.rs1];
    int32_t rhs = (int32_t)cpu->regs[instr.b.rs2];

    if(lhs >= rhs) {
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}

rv_exc_t bgeu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.b.opcode == rv_opcBRANCH);

    // target is relative to address of the instruction
    uint32_t target = cpu->pc + RV_B_IMM(instr);

    uint32_t lhs = cpu->regs[instr.b.rs1];
    uint32_t rhs = cpu->regs[instr.b.rs2];

    if(lhs >= rhs) {
        if(!IS_ALIGNED(target, 4)){
            cpu->csr.tval_next = target;
            return rv_exc_instruction_address_misaligned;
        }
        cpu->pc_next = target;
    }

    return rv_exc_none;
}