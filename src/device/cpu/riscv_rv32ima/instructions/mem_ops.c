#include <stdint.h>

#include "mem_ops.h"
#include "../../../../assert.h"

rv_exc_t load_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcLOAD);

    // TODO: test if it works for negative immediates
    uint32_t virt = cpu->regs[instr.i.rs1] + (int32_t)instr.i.imm;

    // for now only 32 bit
    //! CHANGE THIS!!!!!
    uint32_t val;

    printf("[reading from: 0x%08x ", virt);

    rv_exc_t ex = rv_read_mem32(cpu, virt, &val, true);
    
    printf("val: %u to: x%d]", val, instr.i.rd);

    if(ex != rv_exc_none){
        return ex;
    }

    cpu->regs[instr.i.rd] = val;

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

