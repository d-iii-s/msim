#include <stdint.h>
#include "computations.h"
#include "../../../../assert.h"

/******
 * OP *
 ******/

rv_exc_t add_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs + rhs;

    printf(" [add instruction %d + %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}

rv_exc_t sub_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs - rhs;

    printf(" [sub instruction %d - %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t sll_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs << rhs;

    printf(" [sll instruction %d << %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t slt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = cpu->regs[instr.r.rs1];
    int32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    printf(" [slt instruction %d < %d ? %s]", lhs, rhs, (cpu->regs[instr.r.rd] ? "true" : "false"));

    return rv_exc_none;
}
rv_exc_t sltu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    printf(" [sltu instruction %d < %d ? %s]", lhs, rhs, (cpu->regs[instr.r.rd] ? "true" : "false"));

    return rv_exc_none;
}
rv_exc_t xor_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs ^ rhs;

    printf(" [xor instruction %08x ^ %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t srl_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs >> rhs;

    printf(" [srl instruction %d >> %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t sra_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    int32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = (uint32_t)(lhs >> rhs);

    printf(" [sra instruction %d >> %d = %d]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t or_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs | rhs;

    printf(" [or instruction %08x | %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}
rv_exc_t and_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs & rhs;

    printf(" [and instruction %08x & %08x = %08x]", lhs, rhs, cpu->regs[instr.r.rd]);

    return rv_exc_none;
}

/**********
 * OP-IMM *
 **********/

rv_exc_t addi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    int32_t val = cpu->regs[instr.i.rs1] + imm;

    printf(" [addi %08x (imm) + %08x (reg) = %08x]", imm, cpu->regs[instr.i.rs1], val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t slti_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    bool cmp = ((int32_t)(cpu->regs[instr.i.rs1]) < imm);

    printf(" [slti %d (imm) > %d (reg) ? %s]", imm, cpu->regs[instr.i.rs1], (cmp ? "true" : "false"));

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t sltiu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    // sign extend to 32 bits, then change to unsigned
    uint32_t imm = (int32_t)instr.i.imm;

    bool cmp = ((cpu->regs[instr.i.rs1]) < imm);

    printf(" [sltiu %08x (imm) > %08x (reg) ? %s]", imm, cpu->regs[instr.i.rs1], (cmp ? "true" : "false"));

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t andi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] & imm;

    printf(" [andi %08x (imm) & %08x (reg) = %08x]", imm, cpu->regs[instr.i.rs1], val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t ori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] | imm;

    printf(" [ori %08x (imm) | %08x (reg) = %08x]", imm, cpu->regs[instr.i.rs1], val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t xori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] ^ imm;

    printf(" [xori %08x (imm) ^ %08x (reg) = %08x]", imm, cpu->regs[instr.i.rs1], val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t slli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] << imm;

    printf(" [slli  %08x << %d = %08x]", cpu->regs[instr.i.rs1], imm, val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t srli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] >> imm;

    printf(" [srli  %08x >> %d = %08x]", cpu->regs[instr.i.rs1], imm, val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t srai_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (int32_t)cpu->regs[instr.i.rs1] >> imm;

     printf(" [srai  %08x >> %d = %08x]", cpu->regs[instr.i.rs1], imm, val);

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

/*****************
 * LUI and AUIPC *
 *****************/

rv_exc_t lui_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcLUI);

    return rv_exc_none;
}

rv_exc_t auipc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcAUIPC);

    return rv_exc_none;
}
