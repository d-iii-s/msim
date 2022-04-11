#ifndef RISCV_RV32IMA_INSTR_SYSTEM_H_
#define RISCV_RV32IMA_INSTR_SYSYEM_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t break_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t halt_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_SYSYEM_H_