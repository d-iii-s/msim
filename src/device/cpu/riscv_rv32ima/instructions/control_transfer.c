#include "control_transfer.h"
#include "../../../../assert.h"
#include "../../../../utils.h"

rv_exc_t jal_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.j.opcode == rv_opcJAL);

    // jump target is relative to the address of the instruction eg. pc
    uint32_t target = cpu->pc + RV_J_IMM(instr);

    if(!IS_ALIGNED(target, 4)){
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    printf("JAL jumping to %08x writing return address to x%d", target, instr.j.rd);

    cpu->pc_next = target;
    return rv_exc_none;
}

rv_exc_t jalr_instr(rv_cpu_t *cpu, rv_instr_t instr) {
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcJALR);
    ASSERT(instr.i.func3 == 0);

    uint32_t target = cpu->regs[instr.i.rs1] + instr.i.imm;
    // lowest bit set to 0, as described in the specification
    target &= ~1;

    if(!IS_ALIGNED(target, 4)){
        return rv_exc_instruction_address_misaligned;
    }

    cpu->regs[instr.j.rd] = cpu->pc + 4;

    printf("JALR jumping to %08x writing return address to x%d", target, instr.j.rd);

    cpu->pc_next = target;
    return rv_exc_none;
}