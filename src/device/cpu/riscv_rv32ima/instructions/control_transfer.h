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

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t jal_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t jalr_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t beq_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t bne_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t blt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t bltu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t bge_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t bgeu_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif //RISCV_RV32IMA_INSTR_CONTROL_TRANSFER_H_