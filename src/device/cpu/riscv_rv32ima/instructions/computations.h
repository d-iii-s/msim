/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V arithmetic instructions (includes AMO instructions from A extension)
 *
 */

#ifndef RISCV_RV32IMA_INSTR_COMPUTATIONS_H_
#define RISCV_RV32IMA_INSTR_COMPUTATIONS_H_

#include "../cpu.h"
#include "../instr.h"

extern rv_exc_t rv_add_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sub_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sll_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_slt_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sltu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_xor_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_srl_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sra_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_or_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_and_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_addi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_slti_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_sltiu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_andi_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_ori_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_xori_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_slli_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_srli_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_srai_instr(rv_cpu_t *cpu, rv_instr_t instr);

extern rv_exc_t rv_lui_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_auipc_instr(rv_cpu_t *cpu, rv_instr_t instr);

/* M extension */

extern rv_exc_t rv_mul_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_mulh_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_mulhsu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_mulhu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_div_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_divu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_rem_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_remu_instr(rv_cpu_t *cpu, rv_instr_t instr);

/* A extension atomic operations */

extern rv_exc_t rv_amoswap_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amoadd_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amoxor_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amoand_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amoor_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amomin_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amomax_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amominu_instr(rv_cpu_t *cpu, rv_instr_t instr);
extern rv_exc_t rv_amomaxu_instr(rv_cpu_t *cpu, rv_instr_t instr);

#endif // RISCV_RV32IMA_INSTR_COMPUTATIONS_H_
