#include "control_transfer.h"
#include "../../../../assert.h"

extern rv_exc_t jal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.j.opcode == rv_opcJAL);

    // jump target is relative to the address of the instruction eg. pc
    uint32_t target = cpu->pc + RV_J_IMM(instr);

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    printf("jumping to %08x writing return address to x%d", target, instr.j.rd);

    cpu->pc_next = target;
    return rv_exc_none;
}