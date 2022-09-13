#include <stdint.h>

#include "mem_ops.h"
#include "../../../../assert.h"
#include "../../../../physmem.h"
#include "../../../../fault.h"
#include "../../../../utils.h"

rv_exc_t lb_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint8_t val;

    rv_exc_t ex = rv_read_mem8(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (int8_t)val;

    return rv_exc_none;
}

rv_exc_t lh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint16_t val;

    if(!IS_ALIGNED(virt, 2)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (int16_t)val;

    return rv_exc_none;
}

rv_exc_t lw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    if(!IS_ALIGNED(virt, 4)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    uint32_t val;

    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t lbu_instr(rv_cpu_t *cpu, rv_instr_t instr){
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

rv_exc_t lhu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    if(!IS_ALIGNED(virt, 2)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    uint16_t val;

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (uint32_t)val;

    return rv_exc_none;
}

rv_exc_t sb_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    rv_exc_t ex = rv_write_mem8(cpu, virt, (uint8_t)cpu->regs[instr.s.rs2], true);

    if(ex != rv_exc_none){
        return ex;
    } 

    return rv_exc_none;
}

rv_exc_t sh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    if(!IS_ALIGNED(virt, 2)){
        cpu->csr.tval_next = virt;
        return rv_exc_store_amo_address_misaligned;
    }

    rv_exc_t ex = rv_write_mem16(cpu, virt, (uint16_t)cpu->regs[instr.s.rs2], true);

    if(ex != rv_exc_none){
        return ex;
    } 

    return rv_exc_none;
}

rv_exc_t sw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    if(!IS_ALIGNED(virt, 4)){
        cpu->csr.tval_next = virt;
        return rv_exc_store_amo_address_misaligned;
    }

    rv_exc_t ex = rv_write_mem32(cpu, virt, cpu->regs[instr.s.rs2], true);

    if(ex != rv_exc_none){
        return ex;
    } 

    return rv_exc_none;
}


rv_exc_t fence_instr(rv_cpu_t *cpu, rv_instr_t instr){
    // FENCE instruction does nothing in deterministic emulator,
    // where out-of-order processing is not allowed
    return rv_exc_none;
}

/* A extension LR and SC */

rv_exc_t lr_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);
    ASSERT(instr.r.rs2 == 0);

    uint32_t virt = cpu->regs[instr.r.rs1];

    
    if(!IS_ALIGNED(virt, 4)){
        cpu->csr.tval_next = virt;
        return rv_exc_load_address_misaligned;
    }

    uint32_t val;
    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, true);

    if(ex != rv_exc_none){
        // if read failed, cancel all previous reservations
        sc_unregister(cpu->csr.mhartid);
        cpu->reserved_valid = false;
        return ex;
    }

    // store the read value
    cpu->regs[instr.r.rd] = val;

    // we track physical addresses, so convert
    // this should not fail

    ptr36_t phys;
    ex = rv_convert_addr(cpu, virt, &phys, false, false);
    ASSERT(ex == rv_exc_none);

    // register address for tracking

    sc_register(cpu->csr.mhartid);
    cpu->reserved_valid = true;
    cpu->reserved_addr = phys;

    return rv_exc_none;
}

rv_exc_t sc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcAMO);

    // convert addr and check if the target is tracked
    uint32_t virt = cpu->regs[instr.r.rs1];
    ptr36_t phys;

    if(!IS_ALIGNED(virt, 4)){
        cpu->csr.tval_next = virt;
        return rv_exc_store_amo_address_misaligned;
    }

    if(cpu->reserved_valid == false){
        // reservation is not valid
        cpu->regs[instr.r.rd] = 1;
        return rv_exc_none;
    }

    // SC always invalidates reservation by this hart
    cpu->reserved_valid = false;
    sc_unregister(cpu->csr.mhartid);

    rv_exc_t ex = rv_convert_addr(cpu, virt, &phys, true, true);

    if(ex != rv_exc_none){
        return ex;
    }

    if(phys != cpu->reserved_addr){
        // target differs
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

