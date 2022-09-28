#ifndef RISCV_RV32IMA_INSTR_SYSTEM_H_
#define RISCV_RV32IMA_INSTR_SYSYEM_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t break_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t halt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t dump_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t call_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t sret_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t mret_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t wfi_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t sfence_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_SYSYEM_H_