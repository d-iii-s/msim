#include <stdint.h>

#include "mem_ops.h"
#include "../../../../assert.h"

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
