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

    return rv_exc_none;
}

rv_exc_t sub_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs - rhs;

    return rv_exc_none;
}
rv_exc_t sll_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs << rhs;

    return rv_exc_none;
}
rv_exc_t slt_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = cpu->regs[instr.r.rs1];
    int32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}
rv_exc_t sltu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = (lhs < rhs) ? 1 : 0;

    return rv_exc_none;
}
rv_exc_t xor_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs ^ rhs;

    return rv_exc_none;
}
rv_exc_t srl_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    uint32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = lhs >> rhs;

    return rv_exc_none;
}
rv_exc_t sra_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    int32_t lhs = (int32_t)cpu->regs[instr.r.rs1];
    // based only on lowest 5 bits
    int32_t rhs = 0x1F & (cpu->regs[instr.r.rs2]);

    cpu->regs[instr.r.rd] = (uint32_t)(lhs >> rhs);

    return rv_exc_none;
}
rv_exc_t or_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs | rhs;

    return rv_exc_none;
}
rv_exc_t and_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.r.opcode == rv_opcOP);

    uint32_t lhs = cpu->regs[instr.r.rs1];
    uint32_t rhs = cpu->regs[instr.r.rs2];

    cpu->regs[instr.r.rd] = lhs & rhs;

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

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t slti_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    bool cmp = ((int32_t)(cpu->regs[instr.i.rs1]) < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t sltiu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    // sign extend to 32 bits, then change to unsigned
    uint32_t imm = (int32_t)instr.i.imm;

    bool cmp = ((cpu->regs[instr.i.rs1]) < imm);

    cpu->regs[instr.i.rd] = cmp ? 1 : 0;

    return rv_exc_none;
}

rv_exc_t andi_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] & imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t ori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] | imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t xori_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    int32_t imm = instr.i.imm;

    uint32_t val = cpu->regs[instr.i.rs1] ^ imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t slli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);
    
    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] << imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t srli_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (uint32_t)cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

rv_exc_t srai_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.i.opcode == rv_opcOP_IMM);

    uint32_t imm = instr.i.imm & RV_IMM_SHIFT_SHAMT_MASK;

    uint32_t val = (int32_t)cpu->regs[instr.i.rs1] >> imm;

    cpu->regs[instr.i.rd] = val;

    return rv_exc_none;
}

/*****************
 * LUI and AUIPC *
 *****************/

rv_exc_t lui_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcLUI);

    cpu->regs[instr.u.rd] = instr.u.imm << 12;

    return rv_exc_none;
}

rv_exc_t auipc_instr(rv_cpu_t *cpu, rv_instr_t instr){
    ASSERT(cpu != NULL);
    ASSERT(instr.u.opcode == rv_opcAUIPC);

    int32_t offset = instr.u.imm << 12;

    uint32_t val = cpu->pc + offset;

    cpu->regs[instr.u.rd] = val;

    return rv_exc_none;
}

/***************
 * M extension *
 ***************/

extern rv_exc_t mul_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t mulh_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t mulhsu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t mulhu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t div_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t divu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t rem_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}

extern rv_exc_t remu_instr(rv_cpu_t *cpu, rv_instr_t instr){
    return rv_exc_none;
}
