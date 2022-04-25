#ifndef RISCV_RV32IMA_DEBUG_H_
#define RISCV_RV32IMA_DEBUG_H_

#include <stdint.h>
#include <stdbool.h>
#include "instr.h"
#include "csr.h"
#include "cpu.h"

extern void rv_reg_dump(rv_cpu_t *cpu);
extern void rv_idump(rv_cpu_t *cpu, uint32_t addr, rv_instr_t instr, bool modregs);

#endif // RISCV_RV32IMA_DEBUG_H_

