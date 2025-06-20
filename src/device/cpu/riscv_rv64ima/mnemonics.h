/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISC-V debugging information formatting
 *
 */

#ifndef RISCV_RV64IMA_MNEMONICS_H_
#define RISCV_RV64IMA_MNEMONICS_H_

#include <stdint.h>

#include "cpu.h"

/** Generics */
#include "../riscv_rv_ima/instr.h"

// Dissasembly mnemonics
typedef void (*rv_mnemonics_func_t)(uint32_t, rv_instr_t, string_t *, string_t *);

extern rv_mnemonics_func_t rv64_decode_mnemonics(rv_instr_t instr);

extern void rv64_undefined_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// relative addressing
extern void rv64_lui_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_auipc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// control transfer
extern void rv64_jal_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_jalr_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_beq_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_bne_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_blt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_bge_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_bltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_bgeu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem load
extern void rv64_lb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_lh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_lw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_lbu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_lhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem store
extern void rv64_sb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// op imm
extern void rv64_addi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_slti_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sltiu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_xori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_ori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_andi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_slli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_srli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_srai_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// reg op
extern void rv64_add_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sub_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sll_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_slt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_xor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_srl_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sra_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_or_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_and_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem misc
extern void rv64_fence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// system
extern void rv64_ecall_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_ebreak_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_ehalt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_edump_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_trace_set_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_trace_reset_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csr_rd_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_mret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_wfi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sfence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// CSR
extern void rv64_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// M extension
extern void rv64_mul_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_mulh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_mulhsu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_mulhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_div_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_divu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_rem_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_remu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// A extension
extern void rv64_lr_w_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_lr_d_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sc_w_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_sc_d_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amoswap_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amoadd_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amoxor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amoand_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amoor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amomin_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amomax_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amominu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv64_amomaxu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

extern void rv64_csr_dump_common(rv64_cpu_t *cpu, csr_num_t csr);

#endif // RISCV_RV32IMA_MNEMONICS_H_
