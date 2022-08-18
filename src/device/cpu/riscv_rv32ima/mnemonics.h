#ifndef RISCV_RV32IMA_MNEMONICS_H_
#define RISCV_RV32IMA_MNEMONICS_H_

#include <stdint.h>

#include "cpu.h"
#include "instr.h"

// Dissasembly mnemonics
typedef void (*rv_mnemonics_func_t)(uint32_t, rv_instr_t, string_t *, string_t *);

extern rv_mnemonics_func_t rv_decode_mnemonics(rv_instr_t instr);

extern void undefined_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// relative addressing
extern void rv_lui_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_auipc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// control transfer
extern void rv_jal_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_jalr_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_beq_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_bne_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_blt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_bge_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_bltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_bgeu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem load
extern void rv_lb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_lh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_lw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_lbu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_lhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem store
extern void rv_sb_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// op imm
extern void rv_addi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_slti_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sltiu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_xori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_ori_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_andi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_slli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_srli_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_srai_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// reg op
extern void rv_add_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sub_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sll_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_slt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sltu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_xor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_srl_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sra_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_or_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_and_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// mem misc
extern void rv_fence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// system
extern void rv_ecall_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_ebreak_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_ehalt_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_mret_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_wfi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sfence_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);


// CSR
extern void rv_csrrw_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrs_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrwi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrsi_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_csrrci_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// M extension
extern void rv_mul_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_mulh_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_mulhsu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_mulhu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_div_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_divu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_rem_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_remu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

// A extension
extern void rv_lr_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_sc_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amoswap_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amoadd_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amoxor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amoand_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amoor_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amomin_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amomax_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amominu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);
extern void rv_amomaxu_mnemonics(uint32_t addr, rv_instr_t instr, string_t *s_mnemonics, string_t *s_comments);

#endif // RISCV_RV32IMA_MNEMONICS_H_