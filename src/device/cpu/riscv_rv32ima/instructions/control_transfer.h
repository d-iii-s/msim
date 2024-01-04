/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V Jump and branch instructions
 *
 */

#ifndef RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_
#define RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_

#include "../cpu.h"
#include "../instr.h"

extern rv_exc_t rv_jal_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_jalr_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_beq_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_bne_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_blt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_bltu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_bge_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_bgeu_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_