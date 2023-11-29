/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_EXEC_H_
#define SUPERH_SH2E_EXEC_H_

#include "cpu.h"
#include "insn.h"


// Instruction execution functions

extern sh2e_exception_t sh2e_insn_exec_add(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_addi(sh2e_cpu_t * cpu, sh2e_insn_ni_t insn);
extern sh2e_exception_t sh2e_insn_exec_addc(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_addv(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_and(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_andi(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_andm(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_bf(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_bfs(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_bra(sh2e_cpu_t * cpu, sh2e_insn_d12_t insn);
extern sh2e_exception_t sh2e_insn_exec_braf(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_bsr(sh2e_cpu_t * cpu, sh2e_insn_d12_t insn);
extern sh2e_exception_t sh2e_insn_exec_bsrf(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_bt(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_bts(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_clrmac(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_clrt(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmpeq(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmpge(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmpgt(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmphi(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmphs(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmpim(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmppl(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmppz(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_cmpstr(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_div0s(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_div0u(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_div1(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_dt(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_extsb(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_extsw(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_extub(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_extuw(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_jmp(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_jsr(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_mov(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbs(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movws(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movls(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbl(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwl(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movll(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbm(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwm(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movlm(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbp(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwp(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movlp(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbs0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movws0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movls0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbl0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwl0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movll0(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_movi(sh2e_cpu_t * cpu, sh2e_insn_ni_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwi(sh2e_cpu_t * cpu, sh2e_insn_nd8_t insn);
extern sh2e_exception_t sh2e_insn_exec_movli(sh2e_cpu_t * cpu, sh2e_insn_nd8_t insn);
extern sh2e_exception_t sh2e_insn_exec_movblg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwlg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movllg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbsg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwsg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movlsg(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbl4(sh2e_cpu_t * cpu, sh2e_insn_md_t insn);
extern sh2e_exception_t sh2e_insn_exec_movwl4(sh2e_cpu_t * cpu, sh2e_insn_md_t insn);
extern sh2e_exception_t sh2e_insn_exec_movll4(sh2e_cpu_t * cpu, sh2e_insn_nmd_t insn);
extern sh2e_exception_t sh2e_insn_exec_movbs4(sh2e_cpu_t * cpu, sh2e_insn_nd4_t insn);
extern sh2e_exception_t sh2e_insn_exec_movws4(sh2e_cpu_t * cpu, sh2e_insn_nd4_t insn);
extern sh2e_exception_t sh2e_insn_exec_movls4(sh2e_cpu_t * cpu, sh2e_insn_nmd_t insn);
extern sh2e_exception_t sh2e_insn_exec_mova(sh2e_cpu_t * cpu, sh2e_insn_d_t insn);
extern sh2e_exception_t sh2e_insn_exec_movt(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_mull(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_mulsw(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_muluw(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_neg(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_negc(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_nop(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_not(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_or(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_ori(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_orm(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_rte(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_rts(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_rotcl(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_rotcr(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_rotl(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_rotr(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_sett(sh2e_cpu_t * cpu, sh2e_insn_z_t insn);
extern sh2e_exception_t sh2e_insn_exec_shal(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shar(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shll(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shll2(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shll8(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shll16(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shlr(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shlr2(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shlr8(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_shlr16(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_stc_cpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_stcm_cpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_sts_cpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_stsm_cpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_sub(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_subc(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_subv(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_swapb(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_swapw(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_tas(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_trapa(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_tst(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_tsti(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_tstm(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_xor(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_xori(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_xorm(sh2e_cpu_t * cpu, sh2e_insn_i_t insn);
extern sh2e_exception_t sh2e_insn_exec_xtrct(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fabs(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_fadd(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fcmpeq(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fcmpgt(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fdiv(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fldi0(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_fldi1(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_flds(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_float(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmac(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmov(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovl(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovli(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovlr(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovs(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovsi(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmovss(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fmul(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_fneg(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_fsts(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_fsub(sh2e_cpu_t * cpu, sh2e_insn_nm_t insn);
extern sh2e_exception_t sh2e_insn_exec_ftrc(sh2e_cpu_t * cpu, sh2e_insn_m_t insn);
extern sh2e_exception_t sh2e_insn_exec_sts_fpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_stsm_fpu(sh2e_cpu_t * cpu, sh2e_insn_n_t insn);
extern sh2e_exception_t sh2e_insn_exec_illegal(sh2e_cpu_t * cpu, sh2e_insn_t insn);
extern sh2e_exception_t sh2e_insn_exec_not_implemented(sh2e_cpu_t * cpu, sh2e_insn_t insn);

#endif // SUPERH_SH2E_EXEC_H_
