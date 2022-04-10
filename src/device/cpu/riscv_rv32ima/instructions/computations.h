#ifndef RISCV_RV32IMA_INSTR_COMPUTATIONS_H_
#define RISCV_RV32IMA_INSTR_COMPUTATIONS_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t add_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sub_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sll_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t slt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sltu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t xor_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t srl_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sra_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t or_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t and_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_COMPUTATIONS_H_