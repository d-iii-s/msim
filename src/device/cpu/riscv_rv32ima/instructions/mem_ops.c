/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * RISC-V Instructions that provide memory access
 *
 */

#include <stdint.h>

#include "mem_ops.h"
#include "../../../../assert.h"
#include "../../../../physmem.h"
#include "../../../../fault.h"
#include "../../../../utils.h"

rv_exc_t rv_lb_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint8_t val;

    rv_exc_t ex = rv_read_mem8(cpu, virt, &val, true);

    if(ex != rv_exc_none){
        return ex;
    }

    // Sign extension magic
    cpu->regs[instr.i.rd] = (int8_t)val;

    return rv_exc_none;
}

rv_exc_t rv_lh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint16_t val;

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, false, true);

    if(ex != rv_exc_none){
        return ex;
    }

    // Sign extension magic
    cpu->regs[instr.i.rd] = (int16_t)val;

    return rv_exc_none;
}

rv_exc_t rv_lw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    if(!IS_ALIGNED(virt, 4)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    uint32_t val;

    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);

    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t rv_lbu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint8_t val;

    rv_exc_t ex = rv_read_mem8(cpu, virt, &val, true);

    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (uint32_t)val;

    return rv_exc_none;
}

rv_exc_t rv_lhu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    if(!IS_ALIGNED(virt, 2)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    uint16_t val;

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, false, true);

    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (uint32_t)val;

    return rv_exc_none;
}

rv_exc_t rv_sb_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    return rv_write_mem8(cpu, virt, (uint8_t)cpu->regs[instr.s.rs2], true);
}

rv_exc_t rv_sh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    return rv_write_mem16(cpu, virt, (uint16_t)cpu->regs[instr.s.rs2], true);
}

rv_exc_t rv_sw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    return rv_write_mem32(cpu, virt, cpu->regs[instr.s.rs2], true);
}


rv_exc_t rv_fence_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // FENCE instruction does nothing in deterministic emulator,
    // where out-of-order processing is not allowed
    return rv_exc_none;
}

/* A extension LR and SC */

rv_exc_t rv_lr_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);
    ASSERT(instr.r.rs2 == 0);

    uint32_t virt = cpu->regs[instr.r.rs1];

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, false, true);

    if(ex != rv_exc_none){
        // if read failed, cancel all previous reservations
        sc_unregister(cpu->csr.mhartid);
        cpu->reserved_valid = false;
        return ex;
    }

    // The missalignment should be caught in rv_read_mem32
    // Bit if we would choose to allow missaligned accesses in the future,
    // this would break, so this is here just for safety
    if(!IS_ALIGNED(virt, 4)){
        sc_unregister(cpu->csr.mhartid);
        cpu->reserved_valid = false;
        return rv_exc_load_address_misaligned;
    }

    // store the read value
    cpu->regs[instr.r.rd] = val;

    // we track physical addresses, so convert
    // this should not fail

    ptr36_t phys;
    ex = rv_convert_addr(cpu, virt, &phys, false, false, false);
    ASSERT(ex == rv_exc_none);

    // register address for tracking

    sc_register(cpu->csr.mhartid);
    cpu->reserved_valid = true;
    cpu->reserved_addr = phys;

    return rv_exc_none;
}

rv_exc_t rv_sc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    // convert addr and check if the target is tracked
    uint32_t virt = cpu->regs[instr.r.rs1];
    ptr36_t phys;

    if(cpu->reserved_valid == false){
        // reservation is not valid
        cpu->regs[instr.r.rd] = 1;
        return rv_exc_none;
    }

    // SC always invalidates reservation by this hart
    cpu->reserved_valid = false;
    sc_unregister(cpu->csr.mhartid);

    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, false, true);

    if(ex != rv_exc_none){
        cpu->regs[instr.r.rd] = 1;
        return ex;
    }

    if(!IS_ALIGNED(virt, 4)){
        cpu->regs[instr.r.rd] = 1;
        return rv_exc_store_amo_address_misaligned;
    }

    if(phys != cpu->reserved_addr){
        alert("RV32IMA: LR/SC addresses do not match");
        cpu->regs[instr.r.rd] = 1;
        return rv_exc_none;
    }

    // phys == reserved_addr
    // here we suppose that only 32-bit reservations to aligned addresses are possible
    // but since mips does not allow unaligned accesses, and only 32-bit ll is now supported in msim for mips
    // and risc-v allows only aligned accesses (without Zam extension) and only 32-bit atomics are supported here
    // this should be fine

    ex = rv_write_mem32(cpu, virt, cpu->regs[instr.r.rs2], true);

    if(ex != rv_exc_none){
        alert("RV32IMA: SC write failed after successful address translation");
        cpu->regs[instr.r.rd] = 1;
        return ex;
    }

    cpu->regs[instr.r.rd] = 0;
    return rv_exc_none;
}

