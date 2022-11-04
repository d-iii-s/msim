/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * RISC-V Instructions that provide memory access
 * (Including LR/SC atomics from the A extension)
 *
 */

#ifndef RISCV_RV32IMA_INSTR_MEM_OPS_H_
#define RISCV_RV32IMA_INSTR_MEM_OPS_H_

#include "../instr.h"
#include "../cpu.h"

extern rv_exc_t rv_lb_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_lh_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_lw_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_lbu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_lhu_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_sb_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sh_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sw_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_fence_instr(rv_cpu_t *cpu, rv_instr_t instr);

/* A extension LR and SC */

extern rv_exc_t rv_lr_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sc_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_MEM_OPS_H_