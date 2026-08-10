/*
 * Copyright (c) 2025 Lubomir Bulej
 * Copyright (c) 2026 Matus Jurcak
 *
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_INSN_H_
#define SUPERH_SH2E_INSN_H_

#include <stdint.h>

#include "../../../../config.h"

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/** Helper opcodes */
typedef enum {
    sh2e_insn_z_ic_clrmac = 0b0000000000101000,
    sh2e_insn_z_ic_clrt = 0b0000000000001000,
    sh2e_insn_z_ic_div0u = 0b0000000000011001,
    sh2e_insn_z_ic_nop = 0b0000000000001001,
    sh2e_insn_z_ic_rte = 0b0000000000101011,
    sh2e_insn_z_ic_rts = 0b0000000000001011,
    sh2e_insn_z_ic_sett = 0b0000000000011000,
    sh2e_insn_z_ic_sleep = 0b0000000000011011,
    sh2e_insn_z_ic_halt = 0b1000001000000000,
    sh2e_insn_z_ic_cpu_reg_dump = 0b1000001000000001,
    sh2e_insn_z_ic_fpu_reg_dump = 0b1000001000000010,
} sh2e_insn_z_ic_t;

typedef enum {
    sh2e_insn_n_ic_h_cmppl = 0b0100,
    sh2e_insn_n_ic_h_cmppz = 0b0100,
    sh2e_insn_n_ic_h_dt = 0b0100,
    sh2e_insn_n_ic_h_movt = 0b0000,
    sh2e_insn_n_ic_h_rotcl = 0b0100,
    sh2e_insn_n_ic_h_rotcr = 0b0100,
    sh2e_insn_n_ic_h_rotl = 0b0100,
    sh2e_insn_n_ic_h_rotr = 0b0100,
    sh2e_insn_n_ic_h_shal = 0b0100,
    sh2e_insn_n_ic_h_shar = 0b0100,
    sh2e_insn_n_ic_h_shll = 0b0100,
    sh2e_insn_n_ic_h_shll2 = 0b0100,
    sh2e_insn_n_ic_h_shll8 = 0b0100,
    sh2e_insn_n_ic_h_shll16 = 0b0100,
    sh2e_insn_n_ic_h_shlr = 0b0100,
    sh2e_insn_n_ic_h_shlr2 = 0b0100,
    sh2e_insn_n_ic_h_shlr8 = 0b0100,
    sh2e_insn_n_ic_h_shlr16 = 0b0100,
    sh2e_insn_n_ic_h_stc_sr = 0b0000,
    sh2e_insn_n_ic_h_stc_gbr = 0b0000,
    sh2e_insn_n_ic_h_stc_vbr = 0b0000,
    sh2e_insn_n_ic_h_stcm_sr = 0b0100,
    sh2e_insn_n_ic_h_stcm_gbr = 0b0100,
    sh2e_insn_n_ic_h_stcm_vbr = 0b0100,
    sh2e_insn_n_ic_h_sts_fpul = 0b0000,
    sh2e_insn_n_ic_h_sts_fpscr = 0b0000,
    sh2e_insn_n_ic_h_sts_mach = 0b0000,
    sh2e_insn_n_ic_h_sts_macl = 0b0000,
    sh2e_insn_n_ic_h_sts_pr = 0b0000,
    sh2e_insn_n_ic_h_stsm_fpul = 0b0100,
    sh2e_insn_n_ic_h_stsm_fpscr = 0b0100,
    sh2e_insn_n_ic_h_stsm_mach = 0b0100,
    sh2e_insn_n_ic_h_stsm_macl = 0b0100,
    sh2e_insn_n_ic_h_stsm_pr = 0b0100,
    sh2e_insn_n_ic_h_tas = 0b0100,
    sh2e_insn_n_ic_h_fabs = 0b1111,
    sh2e_insn_n_ic_h_fldi0 = 0b1111,
    sh2e_insn_n_ic_h_fldi1 = 0b1111,
    sh2e_insn_n_ic_h_float = 0b1111,
    sh2e_insn_n_ic_h_fneg = 0b1111,
    sh2e_insn_n_ic_h_fsts = 0b1111,
} sh2e_insn_n_ic_h_t;

typedef enum {
    sh2e_insn_n_ic_l_cmppl = 0b00010101,
    sh2e_insn_n_ic_l_cmppz = 0b00010001,
    sh2e_insn_n_ic_l_dt = 0b00010000,
    sh2e_insn_n_ic_l_movt = 0b00101001,
    sh2e_insn_n_ic_l_rotcl = 0b00100100,
    sh2e_insn_n_ic_l_rotcr = 0b00100101,
    sh2e_insn_n_ic_l_rotl = 0b00000100,
    sh2e_insn_n_ic_l_rotr = 0b00000101,
    sh2e_insn_n_ic_l_shal = 0b00100000,
    sh2e_insn_n_ic_l_shar = 0b00100001,
    sh2e_insn_n_ic_l_shll = 0b00000000,
    sh2e_insn_n_ic_l_shll2 = 0b00001000,
    sh2e_insn_n_ic_l_shll8 = 0b00011000,
    sh2e_insn_n_ic_l_shll16 = 0b00101000,
    sh2e_insn_n_ic_l_shlr = 0b00000001,
    sh2e_insn_n_ic_l_shlr2 = 0b00001001,
    sh2e_insn_n_ic_l_shlr8 = 0b00011001,
    sh2e_insn_n_ic_l_shlr16 = 0b00101001,
    sh2e_insn_n_ic_l_stc_sr = 0b00000010,
    sh2e_insn_n_ic_l_stc_gbr = 0b00010010,
    sh2e_insn_n_ic_l_stc_vbr = 0b00100010,
    sh2e_insn_n_ic_l_stcm_sr = 0b00000011,
    sh2e_insn_n_ic_l_stcm_gbr = 0b00010011,
    sh2e_insn_n_ic_l_stcm_vbr = 0b00100011,
    sh2e_insn_n_ic_l_sts_fpul = 0b01011010,
    sh2e_insn_n_ic_l_sts_fpscr = 0b01101010,
    sh2e_insn_n_ic_l_sts_mach = 0b00001010,
    sh2e_insn_n_ic_l_sts_macl = 0b00011010,
    sh2e_insn_n_ic_l_sts_pr = 0b00101010,
    sh2e_insn_n_ic_l_stsm_fpul = 0b01010010,
    sh2e_insn_n_ic_l_stsm_fpscr = 0b01100010,
    sh2e_insn_n_ic_l_stsm_mach = 0b00000010,
    sh2e_insn_n_ic_l_stsm_macl = 0b00010010,
    sh2e_insn_n_ic_l_stsm_pr = 0b00100010,
    sh2e_insn_n_ic_l_tas = 0b00011011,
    sh2e_insn_n_ic_l_fabs = 0b01011101,
    sh2e_insn_n_ic_l_fldi0 = 0b10001101,
    sh2e_insn_n_ic_l_fldi1 = 0b10011101,
    sh2e_insn_n_ic_l_float = 0b00101101,
    sh2e_insn_n_ic_l_fneg = 0b01001101,
    sh2e_insn_n_ic_l_fsts = 0b00001101,
} sh2e_insn_n_ic_l_t;

typedef enum {
    sh2e_insn_m_ic_h_braf = 0b0000,
    sh2e_insn_m_ic_h_bsrf = 0b0000,
    sh2e_insn_m_ic_h_jmp = 0b0100,
    sh2e_insn_m_ic_h_jsr = 0b0100,
    sh2e_insn_m_ic_h_ldc_sr = 0b0100,
    sh2e_insn_m_ic_h_ldc_gbr = 0b0100,
    sh2e_insn_m_ic_h_ldc_vbr = 0b0100,
    sh2e_insn_m_ic_h_ldcl_sr = 0b0100,
    sh2e_insn_m_ic_h_ldcl_gbr = 0b0100,
    sh2e_insn_m_ic_h_ldcl_vbr = 0b0100,
    sh2e_insn_m_ic_h_lds_mach = 0b0100,
    sh2e_insn_m_ic_h_lds_macl = 0b0100,
    sh2e_insn_m_ic_h_lds_pr = 0b0100,
    sh2e_insn_m_ic_h_lds_fpscr = 0b0100,
    sh2e_insn_m_ic_h_lds_fpul = 0b0100,
    sh2e_insn_m_ic_h_ldsl_mach = 0b0100,
    sh2e_insn_m_ic_h_ldsl_macl = 0b0100,
    sh2e_insn_m_ic_h_ldsl_pr = 0b0100,
    sh2e_insn_m_ic_h_ldsl_fpscr = 0b0100,
    sh2e_insn_m_ic_h_ldsl_fpul = 0b0100,
    sh2e_insn_m_ic_h_flds = 0b1111,
    sh2e_insn_m_ic_h_ftrc = 0b1111,
} sh2e_insn_m_ic_h_t;

typedef enum {
    sh2e_insn_m_ic_l_braf = 0b00100011,
    sh2e_insn_m_ic_l_bsrf = 0b00000011,
    sh2e_insn_m_ic_l_jmp = 0b00101011,
    sh2e_insn_m_ic_l_jsr = 0b00001011,
    sh2e_insn_m_ic_l_ldc_sr = 0b00001110,
    sh2e_insn_m_ic_l_ldc_gbr = 0b00011110,
    sh2e_insn_m_ic_l_ldc_vbr = 0b00101110,
    sh2e_insn_m_ic_l_ldcl_sr = 0b00000111,
    sh2e_insn_m_ic_l_ldcl_gbr = 0b00010111,
    sh2e_insn_m_ic_l_ldcl_vbr = 0b00100111,
    sh2e_insn_m_ic_l_lds_mach = 0b00001010,
    sh2e_insn_m_ic_l_lds_macl = 0b00011010,
    sh2e_insn_m_ic_l_lds_pr = 0b00101010,
    sh2e_insn_m_ic_l_lds_fpscr = 0b01101010,
    sh2e_insn_m_ic_l_lds_fpul = 0b01011010,
    sh2e_insn_m_ic_l_ldsl_mach = 0b00000110,
    sh2e_insn_m_ic_l_ldsl_macl = 0b00010110,
    sh2e_insn_m_ic_l_ldsl_pr = 0b00100110,
    sh2e_insn_m_ic_l_ldsl_fpscr = 0b01100110,
    sh2e_insn_m_ic_l_ldsl_fpul = 0b01010110,
    sh2e_insn_m_ic_l_flds = 0b00011101,
    sh2e_insn_m_ic_l_ftrc = 0b00111101,
} sh2e_insn_m_ic_l_t;
typedef enum {
    sh2e_insn_nm_ic_h_extsb = 0b0110,
    sh2e_insn_nm_ic_h_extsw = 0b0110,
    sh2e_insn_nm_ic_h_extub = 0b0110,
    sh2e_insn_nm_ic_h_extuw = 0b0110,
    sh2e_insn_nm_ic_h_movbl = 0b0110,
    sh2e_insn_nm_ic_h_movwl = 0b0110,
    sh2e_insn_nm_ic_h_movll = 0b0110,
    sh2e_insn_nm_ic_h_movbs = 0b0010,
    sh2e_insn_nm_ic_h_movws = 0b0010,
    sh2e_insn_nm_ic_h_movls = 0b0010,
    sh2e_insn_nm_ic_h_movbp = 0b0110,
    sh2e_insn_nm_ic_h_movwp = 0b0110,
    sh2e_insn_nm_ic_h_movlp = 0b0110,
    sh2e_insn_nm_ic_h_movbm = 0b0010,
    sh2e_insn_nm_ic_h_movwm = 0b0010,
    sh2e_insn_nm_ic_h_movlm = 0b0010,
    sh2e_insn_nm_ic_h_movbl0 = 0b0000,
    sh2e_insn_nm_ic_h_movwl0 = 0b0000,
    sh2e_insn_nm_ic_h_movll0 = 0b0000,
    sh2e_insn_nm_ic_h_movbs0 = 0b0000,
    sh2e_insn_nm_ic_h_movws0 = 0b0000,
    sh2e_insn_nm_ic_h_movls0 = 0b0000,
    sh2e_insn_nm_ic_h_add = 0b0011,
    sh2e_insn_nm_ic_h_addc = 0b0011,
    sh2e_insn_nm_ic_h_addv = 0b0011,
    sh2e_insn_nm_ic_h_and = 0b0010,
    sh2e_insn_nm_ic_h_cmpeq = 0b0011,
    sh2e_insn_nm_ic_h_cmpge = 0b0011,
    sh2e_insn_nm_ic_h_cmpgt = 0b0011,
    sh2e_insn_nm_ic_h_cmphi = 0b0011,
    sh2e_insn_nm_ic_h_cmphs = 0b0011,
    sh2e_insn_nm_ic_h_cmpstr = 0b0010,
    sh2e_insn_nm_ic_h_div0s = 0b0010,
    sh2e_insn_nm_ic_h_div1 = 0b0011,
    sh2e_insn_nm_ic_h_dmulsl = 0b0011,
    sh2e_insn_nm_ic_h_dmulul = 0b0011,
    sh2e_insn_nm_ic_h_macl = 0b0000,
    sh2e_insn_nm_ic_h_macw = 0b0100,
    sh2e_insn_nm_ic_h_mov = 0b0110,
    sh2e_insn_nm_ic_h_mull = 0b0000,
    sh2e_insn_nm_ic_h_mulsw = 0b0010,
    sh2e_insn_nm_ic_h_muluw = 0b0010,
    sh2e_insn_nm_ic_h_neg = 0b0110,
    sh2e_insn_nm_ic_h_negc = 0b0110,
    sh2e_insn_nm_ic_h_not = 0b0110,
    sh2e_insn_nm_ic_h_or = 0b0010,
    sh2e_insn_nm_ic_h_sub = 0b0011,
    sh2e_insn_nm_ic_h_subc = 0b0011,
    sh2e_insn_nm_ic_h_subv = 0b0011,
    sh2e_insn_nm_ic_h_swapb = 0b0110,
    sh2e_insn_nm_ic_h_swapw = 0b0110,
    sh2e_insn_nm_ic_h_tst = 0b0010,
    sh2e_insn_nm_ic_h_xor = 0b0010,
    sh2e_insn_nm_ic_h_xtrct = 0b0010,
    sh2e_insn_nm_ic_h_fadd = 0b1111,
    sh2e_insn_nm_ic_h_fcmpeq = 0b1111,
    sh2e_insn_nm_ic_h_fcmpgt = 0b1111,
    sh2e_insn_nm_ic_h_fdiv = 0b1111,
    sh2e_insn_nm_ic_h_fmac = 0b1111,
    sh2e_insn_nm_ic_h_fmov = 0b1111,
    sh2e_insn_nm_ic_h_fmovl = 0b1111,
    sh2e_insn_nm_ic_h_fmovli = 0b1111,
    sh2e_insn_nm_ic_h_fmovlr = 0b1111,
    sh2e_insn_nm_ic_h_fmovs = 0b1111,
    sh2e_insn_nm_ic_h_fmovsi = 0b1111,
    sh2e_insn_nm_ic_h_fmovss = 0b1111,
    sh2e_insn_nm_ic_h_fmul = 0b1111,
    sh2e_insn_nm_ic_h_fsub = 0b1111,
} sh2e_insn_nm_ic_h_t;

typedef enum {
    sh2e_insn_nm_ic_l_extsb = 0b1110,
    sh2e_insn_nm_ic_l_extsw = 0b1111,
    sh2e_insn_nm_ic_l_extub = 0b1100,
    sh2e_insn_nm_ic_l_extuw = 0b1101,
    sh2e_insn_nm_ic_l_movbl = 0b0000,
    sh2e_insn_nm_ic_l_movwl = 0b0001,
    sh2e_insn_nm_ic_l_movll = 0b0010,
    sh2e_insn_nm_ic_l_movbs = 0b0000,
    sh2e_insn_nm_ic_l_movws = 0b0001,
    sh2e_insn_nm_ic_l_movls = 0b0010,
    sh2e_insn_nm_ic_l_movbp = 0b0100,
    sh2e_insn_nm_ic_l_movwp = 0b0101,
    sh2e_insn_nm_ic_l_movlp = 0b0110,
    sh2e_insn_nm_ic_l_movbm = 0b0100,
    sh2e_insn_nm_ic_l_movwm = 0b0101,
    sh2e_insn_nm_ic_l_movlm = 0b0110,
    sh2e_insn_nm_ic_l_movbl0 = 0b1100,
    sh2e_insn_nm_ic_l_movwl0 = 0b1101,
    sh2e_insn_nm_ic_l_movll0 = 0b1110,
    sh2e_insn_nm_ic_l_movbs0 = 0b0100,
    sh2e_insn_nm_ic_l_movws0 = 0b0101,
    sh2e_insn_nm_ic_l_movls0 = 0b0110,
    sh2e_insn_nm_ic_l_add = 0b1100,
    sh2e_insn_nm_ic_l_addc = 0b1110,
    sh2e_insn_nm_ic_l_addv = 0b1111,
    sh2e_insn_nm_ic_l_and = 0b1001,
    sh2e_insn_nm_ic_l_cmpeq = 0b0000,
    sh2e_insn_nm_ic_l_cmpge = 0b0011,
    sh2e_insn_nm_ic_l_cmpgt = 0b0111,
    sh2e_insn_nm_ic_l_cmphi = 0b0110,
    sh2e_insn_nm_ic_l_cmphs = 0b0010,
    sh2e_insn_nm_ic_l_cmpstr = 0b1100,
    sh2e_insn_nm_ic_l_div0s = 0b0111,
    sh2e_insn_nm_ic_l_div1 = 0b0100,
    sh2e_insn_nm_ic_l_dmulsl = 0b1101,
    sh2e_insn_nm_ic_l_dmulul = 0b0101,
    sh2e_insn_nm_ic_l_macl = 0b1111,
    sh2e_insn_nm_ic_l_macw = 0b1111,
    sh2e_insn_nm_ic_l_mov = 0b0011,
    sh2e_insn_nm_ic_l_mull = 0b0111,
    sh2e_insn_nm_ic_l_mulsw = 0b1111,
    sh2e_insn_nm_ic_l_muluw = 0b1110,
    sh2e_insn_nm_ic_l_neg = 0b1011,
    sh2e_insn_nm_ic_l_negc = 0b1010,
    sh2e_insn_nm_ic_l_not = 0b0111,
    sh2e_insn_nm_ic_l_or = 0b1011,
    sh2e_insn_nm_ic_l_sub = 0b1000,
    sh2e_insn_nm_ic_l_subc = 0b1010,
    sh2e_insn_nm_ic_l_subv = 0b1011,
    sh2e_insn_nm_ic_l_swapb = 0b1000,
    sh2e_insn_nm_ic_l_swapw = 0b1001,
    sh2e_insn_nm_ic_l_tst = 0b1000,
    sh2e_insn_nm_ic_l_xor = 0b1010,
    sh2e_insn_nm_ic_l_xtrct = 0b1101,
    sh2e_insn_nm_ic_l_fadd = 0b0000,
    sh2e_insn_nm_ic_l_fcmpeq = 0b0100,
    sh2e_insn_nm_ic_l_fcmpgt = 0b0101,
    sh2e_insn_nm_ic_l_fdiv = 0b0011,
    sh2e_insn_nm_ic_l_fmac = 0b1110,
    sh2e_insn_nm_ic_l_fmov = 0b1100,
    sh2e_insn_nm_ic_l_fmovl = 0b1000,
    sh2e_insn_nm_ic_l_fmovli = 0b0110,
    sh2e_insn_nm_ic_l_fmovlr = 0b1001,
    sh2e_insn_nm_ic_l_fmovs = 0b1010,
    sh2e_insn_nm_ic_l_fmovsi = 0b0111,
    sh2e_insn_nm_ic_l_fmovss = 0b1011,
    sh2e_insn_nm_ic_l_fmul = 0b0010,
    sh2e_insn_nm_ic_l_fsub = 0b0001,
} sh2e_insn_nm_ic_l_t;

typedef enum {
    sh2e_insn_md_ic_movbl4 = 0b10000100,
    sh2e_insn_md_ic_movwl4 = 0b10000101,
} sh2e_insn_md_ic_t;

typedef enum {
    sh2e_insn_nd4_ic_movbs4 = 0b10000000,
    sh2e_insn_nd4_ic_movws4 = 0b10000001,
} sh2e_insn_nd4_ic_t;

typedef enum {
    sh2e_insn_nmd_ic_movls4 = 0b0001,
    sh2e_insn_nmd_ic_movll4 = 0b0101,
} sh2e_insn_nmd_ic_t;

typedef enum {
    sh2e_insn_d_ic_bt = 0b10001001,
    sh2e_insn_d_ic_bts = 0b10001101,
    sh2e_insn_d_ic_bf = 0b10001011,
    sh2e_insn_d_ic_bfs = 0b10001111,
    sh2e_insn_d_ic_movblg = 0b11000100,
    sh2e_insn_d_ic_movwlg = 0b11000101,
    sh2e_insn_d_ic_movllg = 0b11000110,
    sh2e_insn_d_ic_movbsg = 0b11000000,
    sh2e_insn_d_ic_movwsg = 0b11000001,
    sh2e_insn_d_ic_movlsg = 0b11000010,
    sh2e_insn_d_ic_mova = 0b11000111,
} sh2e_insn_d_ic_t;

typedef enum {
    sh2e_insn_d12_ic_bra = 0b1010,
    sh2e_insn_d12_ic_bsr = 0b1011,
} sh2e_insn_d12_ic_t;

typedef enum {
    sh2e_insn_nd8_ic_movwi = 0b1001,
    sh2e_insn_nd8_ic_movli = 0b1101,
} sh2e_insn_nd8_ic_t;

typedef enum {
    sh2e_insn_i_ic_andi = 0b11001001,
    sh2e_insn_i_ic_andm = 0b11001101,
    sh2e_insn_i_ic_cmpim = 0b10001000,
    sh2e_insn_i_ic_ori = 0b11001011,
    sh2e_insn_i_ic_orm = 0b11001111,
    sh2e_insn_i_ic_trapa = 0b11000011,
    sh2e_insn_i_ic_tsti = 0b11001000,
    sh2e_insn_i_ic_tstm = 0b11001100,
    sh2e_insn_i_ic_xori = 0b11001010,
    sh2e_insn_i_ic_xorm = 0b11001110,
} sh2e_insn_i_ic_t;

typedef enum {
    sh2e_insn_ni_ic_addi = 0b0111,
    sh2e_insn_ni_ic_movi = 0b1110,
} sh2e_insn_ni_ic_t;

/****************************************************************************
 * SH-2E instruction formats
 ****************************************************************************/

// 0-format
//
// ic ... instruction code
//
typedef struct {
    sh2e_insn_z_ic_t ic : 16;
} PACKED sh2e_insn_z_t;

_Static_assert(sizeof(sh2e_insn_z_t) == sizeof(uint16_t), "invalid sh2e_insn_z_t size!");

// n-format
//
// ic_h:ic_l ... instruction code
// rn        ... destination register
//
// Direct register addressing.
// Direct register addressing (store with control or system registers).
// Indirect register addressing.
// Pre-decrement indirect register addressing.
// Floating-point instruction.
//
typedef union {
    uint16_t word;
    struct {
#ifdef WORDS_BIGENDIAN
        sh2e_insn_n_ic_h_t ic_h : 4;
        uint16_t rn : 4;
        sh2e_insn_n_ic_l_t ic_l : 8;
#else
        sh2e_insn_n_ic_l_t ic_l : 8;
        uint16_t rn : 4;
        sh2e_insn_n_ic_h_t ic_h : 4;
#endif
    } PACKED;
} sh2e_insn_n_t;

_Static_assert(sizeof(sh2e_insn_n_t) == sizeof(uint16_t), "invalid sh2e_insn_n_t size!");

// m-format
//
// ic_h:ic_l ... instruction code
// rm        ... source register
//
// Direct register addressing (load with control or system registers).
// PC relative addressing with Rm.
// Indirect register addressing.
// Post-increment indirect register addressing.
// Floating-point instruction.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_m_ic_h_t ic_h : 4;
    uint16_t rm : 4;
    sh2e_insn_m_ic_l_t ic_l : 8;
#else
    sh2e_insn_m_ic_l_t ic_l : 8;
    uint16_t rm : 4;
    sh2e_insn_m_ic_h_t ic_h : 4;
#endif
} PACKED sh2e_insn_m_t;

_Static_assert(sizeof(sh2e_insn_m_t) == sizeof(uint16_t), "invalid sh2e_insn_m_t size!");

// nm-format
//
// ic_h:ic_l ... instruction code
// rn        ... destination register
// rm        ... source register
//
// Direct register addressing (load with control or system registers).
// PC relative addressing with Rm.
// Indirect register addressing.
// Post-increment indirect register addressing.
// Floating-point instruction.
//
typedef union {
    uint16_t word;
    struct {
#ifdef WORDS_BIGENDIAN
        sh2e_insn_nm_ic_h_t ic_h : 4;
        uint16_t rn : 4;
        uint16_t rm : 4;
        sh2e_insn_nm_ic_l_t ic_l : 4;
#else
        sh2e_insn_nm_ic_l_t ic_l : 4;
        uint16_t rm : 4;
        uint16_t rn : 4;
        sh2e_insn_nm_ic_h_t ic_h : 4;
#endif
    } PACKED;
} sh2e_insn_nm_t;

_Static_assert(sizeof(sh2e_insn_nm_t) == sizeof(uint16_t), "invalid sh2e_insn_nm_t size!");

// md-format
//
// ic ... instruction code
// rm ... displacement
// d4 ... 4-bit immediate displacement
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_md_ic_t ic : 8;
    uint16_t rm : 4;
    uint16_t d4 : 4;
#else
    uint16_t d4 : 4;
    uint16_t rm : 4;
    sh2e_insn_md_ic_t ic : 8;
#endif
} PACKED sh2e_insn_md_t;

_Static_assert(sizeof(sh2e_insn_md_t) == sizeof(uint16_t), "invalid sh2e_insn_md_t size!");

// nd4-format
//
// ic ... instruction code
// rn ... destination register
// d4 ... 4-bit immediate displacement
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    // R0 source
    sh2e_insn_nd4_ic_t ic : 8;
    uint16_t rn : 4;
    uint16_t d4 : 4;
#else
    uint16_t d4 : 4;
    uint16_t rn : 4;
    sh2e_insn_nd4_ic_t ic : 8;
#endif
} PACKED sh2e_insn_nd4_t;

_Static_assert(sizeof(sh2e_insn_nd4_t) == sizeof(uint16_t), "invalid sh2e_insn_nd4_t size!");

// nmd-format
//
// ic ... instruction code
// rn ... destination register
// d4 ... 4-bit immediate displacement
// rm ... source register
//
// Indirect register addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_nmd_ic_t ic : 4;
    uint16_t rn : 4;
    uint16_t rm : 4;
    uint16_t d4 : 4;
#else
    uint16_t d4 : 4;
    uint16_t rm : 4;
    uint16_t rn : 4;
    sh2e_insn_nmd_ic_t ic : 4;
#endif
} PACKED sh2e_insn_nmd_t;

_Static_assert(sizeof(sh2e_insn_nmd_t) == sizeof(uint16_t), "invalid sh2e_insn_nmd_t size!");

// d-format
//
// ic ... instruction code
// d8 ... 8-bit immediate displacement
//
// Indirect GBR addressing with displacement.
// Indirect PC addressing with displacement.
// PC relative addressing.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    // R0 source
    sh2e_insn_d_ic_t ic : 8;
    uint16_t d8 : 8;
#else
    uint16_t d8 : 8;
    sh2e_insn_d_ic_t ic : 8;
#endif
} PACKED sh2e_insn_d_t;

_Static_assert(sizeof(sh2e_insn_d_t) == sizeof(uint16_t), "invalid sh2e_insn_d_t size!");

// d12-format
//
// ic  ... instruction code
// d12 ... 12-bit immediate displacement
//
// PC relative addressing.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_d12_ic_t ic : 4;
    uint16_t d12 : 12;
#else
    uint16_t d12 : 12;
    sh2e_insn_d12_ic_t ic : 4;
#endif
} PACKED sh2e_insn_d12_t;

_Static_assert(sizeof(sh2e_insn_d12_t) == sizeof(uint16_t), "invalid sh2e_insn_d12_t size!");

// nd8-format
//
// ic ... instruction code
// rn ... destination register
// d8 ... 8-bit immediate displacement
//
// PC relative addressing with displacement.
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_nd8_ic_t ic : 4;
    uint16_t rn : 4;
    uint16_t d8 : 8;
#else
    uint16_t d8 : 8;
    uint16_t rn : 4;
    sh2e_insn_nd8_ic_t ic : 4;
#endif
} PACKED sh2e_insn_nd8_t;

_Static_assert(sizeof(sh2e_insn_nd8_t) == sizeof(uint16_t), "invalid sh2e_insn_nd8_t size!");

// i-format
//
// ic ... instruction code
// i8 ... 8-bit immediate
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_i_ic_t ic : 8;
    uint16_t i8 : 8;
#else
    uint16_t i8 : 8;
    sh2e_insn_i_ic_t ic : 8;
#endif
} PACKED sh2e_insn_i_t;

_Static_assert(sizeof(sh2e_insn_i_t) == sizeof(uint16_t), "invalid sh2e_insn_i_t size!");

// ni-format
//
// ic ... instruction code
// rn ... destination register
// i8 ... 8-bit immediate
//
typedef struct {
#ifdef WORDS_BIGENDIAN
    sh2e_insn_ni_ic_t ic : 4;
    uint16_t rn : 4;
    uint16_t i8 : 8;
#else
    uint16_t i8 : 8;
    uint16_t rn : 4;
    sh2e_insn_ni_ic_t ic : 4;
#endif
} PACKED sh2e_insn_ni_t;

_Static_assert(sizeof(sh2e_insn_ni_t) == sizeof(uint16_t), "invalid sh2e_insn_ni_t size!");

/****************************************************************************
 * SH-2E instruction
 ****************************************************************************/

typedef union sh2e_insn {
    uint16_t word;

    sh2e_insn_z_t z_form;
    sh2e_insn_n_t n_form;
    sh2e_insn_m_t m_form;
    sh2e_insn_nm_t nm_form;
    sh2e_insn_md_t md_form;
    sh2e_insn_nd4_t nd4_form;
    sh2e_insn_nmd_t nmd_form;
    sh2e_insn_d_t d_form;
    sh2e_insn_d12_t d12_form;
    sh2e_insn_nd8_t nd8_form;
    sh2e_insn_i_t i_form;
    sh2e_insn_ni_t ni_form;
} sh2e_insn_t;

_Static_assert(sizeof(sh2e_insn_t) == sizeof(uint16_t), "invalid sh2e_insn_t size!");

#endif // SUPERH_SH2E_INSN_H_
