/*
 * Instructions that provide memory access
 * (Including LR/SC atomics from the A extension)
 */

#ifndef RISCV_RV32IMA_INSTR_MEM_OPS_H_
#define RISCV_RV32IMA_INSTR_MEM_OPS_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t lb_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t lh_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t lw_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t lbu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t lhu_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t sb_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sh_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sw_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t fence_instr(rv_cpu_t *cpu, rv_instr_t instr);

/* A extension LR and SC */

extern rv_exc_t lr_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t sc_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_MEM_OPS_H_