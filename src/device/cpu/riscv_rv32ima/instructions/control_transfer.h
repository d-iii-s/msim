#ifndef RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_
#define RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t jal_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t jalr_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif //RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_