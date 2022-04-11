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

rv_exc_t sub_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs - rhs;

    printf(" [sub instruction %d - %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t sll_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs << rhs;

    printf(" [sll instruction %d << %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t slt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    int32_t lhs = cpu->regs[instr.r.rs1];
    int32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    printf(" [slt instruction %d < %d ? %s]", lhs, rhs, (cpu->regs[instr.r.rd] ? "true" : "false"));

    return rv_exc_none;
}
rv_exc_t sltu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    printf(" [sltu instruction %d < %d ? %s]", lhs, rhs, (cpu->regs[instr.r.rd] ? "true" : "false"));

    return rv_exc_none;
}
rv_exc_t xor_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs ^ rhs;

    printf(" [xor instruction %08x ^ %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t srl_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs >> rhs;

    printf(" [srl instruction %d >> %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t sra_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    int32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = (uint32_t)(lhs >> rhs);

    printf(" [sra instruction %d >> %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t or_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs | rhs;

    printf(" [or instruction %08x | %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t and_instr(rv_cpu_t *cpu, rv_instr_t instr){
     ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs & rhs;

    printf(" [and instruction %08x & %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}