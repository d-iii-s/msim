#include <stdint.h>

#include "mem_ops.h"
#include "../../../../assert.h"

rv_exc_t lb_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint8_t val;

    printf("[reading 8 bits (signed) from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem8(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (int8_t)val;

    printf("val: %d to: x%d]", cpu->regs[instr.i.rd], instr.i.rd);

    return rv_exc_none;
}

rv_exc_t lh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint16_t val;

    printf("[reading 16 bits (signed) from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (int16_t)val;

    printf("val: %d to: x%d]", cpu->regs[instr.i.rd], instr.i.rd);

    return rv_exc_none;
}

rv_exc_t lw_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint32_t val;

    printf("[reading 32 bits from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, true);
    
    printf("val: %u to: x%d]", val, instr.i.rd);

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

    printf("[reading 8 bits (unsigned) from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem8(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (uint32_t)val;

    printf("val: %d to: x%d]", cpu->regs[instr.i.rd], instr.i.rd);

    return rv_exc_none;
}

rv_exc_t lhu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    uint16_t val;

    printf("[reading 16 bits (unsigned) from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem16(cpu, virt, &val, true);
    
    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = (uint32_t)val;

    printf("val: %d to: x%d]", cpu->regs[instr.i.rd], instr.i.rd);

    return rv_exc_none;
}


rv_exc_t store_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.s.opcode == rv_opcSTORE);

    uint32_t virt = cpu->regs[instr.s.rs1] + RV_S_IMM(instr);

    printf(" [writing to: 0x%08x val: %u from x%d]", virt, cpu->regs[instr.s.rs2], instr.s.rs2);

    //! CHANGE THIS TO SUPPORT DIFFERENT LENGTHS
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
