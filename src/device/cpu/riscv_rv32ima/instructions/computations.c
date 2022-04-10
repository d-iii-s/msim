#include <stdint.h>
#include "computations.h"
#include "../../../../assert.h"

rv_exc_t add_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs + rhs;

    printf(" [add instruction %d + %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}