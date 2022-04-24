#ifndef RISCV_RV32IMA_INSTR_MEM_OPS_H_
#define RISCV_RV32IMA_INSTR_MEM_OPS_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t load_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t store_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t fence_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_MEM_OPS_H_