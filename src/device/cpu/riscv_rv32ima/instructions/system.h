/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * RISC-V System instructions
 *
 */

#ifndef RISCV_RV32IMA_INSTR_SYSTEM_H_
#define RISCV_RV32IMA_INSTR_SYSYEM_H_

#include "../cpu.h"
#include "../instr.h"

extern rv_exc_t rv_break_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_halt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_dump_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_trace_set_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_trace_reset_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csr_rd_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_call_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_sret_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_mret_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_wfi_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_csrrw_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csrrs_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csrrc_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csrrwi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csrrsi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_csrrci_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_sfence_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_SYSYEM_H_
