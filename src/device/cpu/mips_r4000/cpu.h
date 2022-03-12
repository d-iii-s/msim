/*
 * Copyright (c) 2000-2008 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS R4000 simulation
 *
 */

#ifndef MIPS_R4000_CPU_H_
#define MIPS_R4000_CPU_H_

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "../../../list.h"
#include "../../../utils.h"
#include "../../../physmem.h"

#define R4K_REG_COUNT     32
#define R4K_REG_VARIANTS  3

#define TLB_ENTRIES   48
#define INTR_COUNT    8
#define TLB_PHYSMASK  UINT64_C(0x780000000)

#define INSTR2ADDR(instr) \
	((instr) << 2)

#define ADDR2INSTR(addr) \
	((addr) >> 2)

#define INSTRS2SIZE(instrs) \
	((instrs) << 2)

#define SIZE2INSTRS(size) \
	((size) >> 2)

/* cp0 registers */
typedef enum {
	/* 0 */
	cp0_Index,
	cp0_Random,
	cp0_EntryLo0,
	cp0_EntryLo1,
	cp0_Context,
	cp0_PageMask,
	cp0_Wired,
	cp0_Res1,
	
	/* 8 */
	cp0_BadVAddr,
	cp0_Count,
	cp0_EntryHi,
	cp0_Compare,
	cp0_Status,
	cp0_Cause,
	cp0_EPC,
	cp0_PRId,
	
	/* 16 */
	cp0_Config,
	cp0_LLAddr,
	cp0_WatchLo,
	cp0_WatchHi,
	cp0_XContext,
	cp0_Res2, 
	cp0_Res3,
	cp0_Res4,
	
	/* 24 */
	cp0_Res5,
	cp0_Res6,
	cp0_ECC,
	cp0_CacheErr,
	cp0_TagLo,
	cp0_TagHi,
	cp0_ErrorEPC,
	cp0_Res7
} cp0_regs_t;

#define cp0_index_index_mask UINT32_C(0x0000003f)
#define cp0_index_res_mask   UINT32_C(0x7fffffc0)
#define cp0_index_p_mask     UINT32_C(0x80000000)

#define cp0_index_index_shift  0
#define cp0_index_res_shift    6
#define cp0_index_p_shift      31

#define cp0_index_index(cpu)  (((cpu)->cp0[cp0_Index].val & cp0_index_index_mask) >> cp0_index_index_shift)
#define cp0_index_res(cpu)    (((cpu)->cp0[cp0_Index].val & cp0_index_res_mask) >> cp0_index_res_shift)
#define cp0_index_p(cpu)      (((cpu)->cp0[cp0_Index].val & cp0_index_p_mask) >> cp0_index_p_shift)

#define cp0_random_random_mask  UINT32_C(0x0000003f)
#define cp0_random_res_mask     UINT32_C(0xffffffc0)

#define cp0_random_random_shift  0
#define cp0_random_res_shift     6

#define cp0_random_random(cpu)  (((cpu)->cp0[cp0_Random].val & cp0_random_random_mask) >> cp0_random_random_shift)
#define cp0_random_res(cpu)     (((cpu)->cp0[cp0_Random].val & cp0_random_res_mask) >> cp0_random_res_shift)

#define cp0_status_ie_mask    UINT32_C(0x00000001)
#define cp0_status_exl_mask   UINT32_C(0x00000002)
#define cp0_status_erl_mask   UINT32_C(0x00000004)
#define cp0_status_ksu_mask   UINT32_C(0x00000018)
#define cp0_status_ux_mask    UINT32_C(0x00000020)
#define cp0_status_sx_mask    UINT32_C(0x00000040)
#define cp0_status_kx_mask    UINT32_C(0x00000080)
#define cp0_status_im_mask    UINT32_C(0x0000ff00)
#define cp0_status_de_mask    UINT32_C(0x00010000)
#define cp0_status_ce_mask    UINT32_C(0x00020000)
#define cp0_status_ch_mask    UINT32_C(0x00040000)
#define cp0_status_res1_mask  UINT32_C(0x00080000)
#define cp0_status_sr_mask    UINT32_C(0x00100000)
#define cp0_status_ts_mask    UINT32_C(0x00200000)
#define cp0_status_bev_mask   UINT32_C(0x00400000)
#define cp0_status_res2_mask  UINT32_C(0x01800000)
#define cp0_status_re_mask    UINT32_C(0x02000000)
#define cp0_status_fr_mask    UINT32_C(0x04000000)
#define cp0_status_rp_mask    UINT32_C(0x08000000)
#define cp0_status_cu0_mask   UINT32_C(0x10000000)
#define cp0_status_cu1_mask   UINT32_C(0x20000000)
#define cp0_status_cu2_mask   UINT32_C(0x40000000)
#define cp0_status_cu3_mask   UINT32_C(0x80000000)
#define cp0_status_cu_mask    UINT32_C(0xf0000000)

#define cp0_status_ie_shift    0
#define cp0_status_exl_shift   1
#define cp0_status_erl_shift   2
#define cp0_status_ksu_shift   3
#define cp0_status_ux_shift    5
#define cp0_status_sx_shift    6
#define cp0_status_kx_shift    7
#define cp0_status_im_shift    8
#define cp0_status_de_shift    16
#define cp0_status_ce_shift    17
#define cp0_status_ch_shift    18
#define cp0_status_res1_shift  19
#define cp0_status_sr_shift    20
#define cp0_status_ts_shift    21
#define cp0_status_bev_shift   22
#define cp0_status_res2_shift  23
#define cp0_status_re_shift    25
#define cp0_status_fr_shift    26
#define cp0_status_rp_shift    27
#define cp0_status_cu0_shift   28
#define cp0_status_cu1_shift   29
#define cp0_status_cu2_shift   30
#define cp0_status_cu3_shift   31
#define cp0_status_cu_shift    28

#define cp0_status_ie(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_ie_mask) >> 0)
#define cp0_status_exl(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_exl_mask) >> 1)
#define cp0_status_erl(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_erl_mask) >> 2)
#define cp0_status_ksu(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_ksu_mask) >> 3)
#define cp0_status_ux(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_ux_mask) >> 5)
#define cp0_status_sx(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_sx_mask) >> 6)
#define cp0_status_kx(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_kx_mask) >> 7)
#define cp0_status_im(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_im_mask) >> 8)
#define cp0_status_de(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_de_mask) >> 16)
#define cp0_status_ce(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_ce_mask) >> 17)
#define cp0_status_ch(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_ch_mask) >> 18)
#define cp0_status_res1(cpu) (((cpu)->cp0[cp0_Status].val & cp0_status_res1_mask) >> 19)
#define cp0_status_sr(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_sr_mask) >> 20)
#define cp0_status_ts(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_ts_mask) >> 21)
#define cp0_status_bev(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_bev_mask) >> 22)
#define cp0_status_res2(cpu) (((cpu)->cp0[cp0_Status].val & cp0_status_res2_mask) >> 23)
#define cp0_status_re(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_re_mask) >> 25)
#define cp0_status_fr(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_fr_mask) >> 26)
#define cp0_status_rp(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_rp_mask) >> 27)
#define cp0_status_cu0(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_cu0_mask) >> 28)
#define cp0_status_cu1(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_cu1_mask) >> 29)
#define cp0_status_cu2(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_cu2_mask) >> 30)
#define cp0_status_cu3(cpu)  (((cpu)->cp0[cp0_Status].val & cp0_status_cu3_mask) >> 31)
#define cp0_status_cu(cpu)   (((cpu)->cp0[cp0_Status].val & cp0_status_cu_mask) >> 28)

#define cp0_entryhi_asid_mask  UINT64_C(0x00000000000000ff)
#define cp0_entryhi_res1_mask  UINT64_C(0x0000000000001f00)
#define cp0_entryhi_vpn2_mask  UINT64_C(0xc00000ffffffe000)

#define cp0_entryhi_asid_shift  0
#define cp0_entryhi_res1_shift  8
#define cp0_entryhi_vpn2_shift  13

#define cp0_entryhi_asid(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_asid_mask) >> cp0_entryhi_asid_shift)
#define cp0_entryhi_res1(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_res1_mask) >> cp0_entryhi_res1_shift)
#define cp0_entryhi_vpn2(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_vpn2_mask) >> cp0_entryhi_vpn2_shift)

#define cp0_entrylo_g_mask     UINT32_C(0x00000001)
#define cp0_entrylo_v_mask     UINT32_C(0x00000002)
#define cp0_entrylo_d_mask     UINT32_C(0x00000004)
#define cp0_entrylo_c_mask     UINT32_C(0x00000038)
#define cp0_entrylo_pfn_mask   UINT32_C(0x3fffffc0)
#define cp0_entrylo_res1_mask  UINT32_C(0xc0000000)

#define cp0_entrylo_g_shift     0
#define cp0_entrylo_v_shift     1
#define cp0_entrylo_d_shift     2
#define cp0_entrylo_c_shift     3
#define cp0_entrylo_pfn_shift   6
#define cp0_entrylo_res1_shift  30

#define cp0_entrylo0_g(cpu)     (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_g_mask) >> cp0_entrylo_g_shift)
#define cp0_entrylo0_v(cpu)     (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_v_mask) >> cp0_entrylo_v_shift)
#define cp0_entrylo0_d(cpu)     (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_d_mask) >> cp0_entrylo_d_shift)
#define cp0_entrylo0_c(cpu)     (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_c_mask) >> cp0_entrylo_c_shift)
#define cp0_entrylo0_pfn(cpu)   (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_pfn_mask) >> cp0_entrylo_pfn_shift)
#define cp0_entrylo0_res1(cpu)  (((cpu)->cp0[cp0_EntryLo0].val & cp0_entrylo_res1_mask) >> cp0_entrylo_res1_shift)

#define cp0_entrylo1_g(cpu)     (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_g_mask) >> cp0_entrylo_g_shift)
#define cp0_entrylo1_v(cpu)     (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_v_mask) >> cp0_entrylo_v_shift)
#define cp0_entrylo1_d(cpu)     (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_d_mask) >> cp0_entrylo_d_shift)
#define cp0_entrylo1_c(cpu)     (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_c_mask) >> cp0_entrylo_c_shift)
#define cp0_entrylo1_pfn(cpu)   (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_pfn_mask) >> cp0_entrylo_pfn_shift)
#define cp0_entrylo1_res1(cpu)  (((cpu)->cp0[cp0_EntryLo1].val & cp0_entrylo_res1_mask) >> cp0_entrylo_res1_shift)

#define cp0_wired_w_mask     UINT32_C(0x0000001f)
#define cp0_wired_res1_mask  UINT32_C(0xffffffe0)

#define cp0_wired_w_shift     0
#define cp0_wired_res1_shift  6

#define cp0_wired_w(cpu)     (((cpu)->cp0[cp0_Wired].val & cp0_wired_w_mask) >> cp0_wired_w_shift)
#define cp0_wired_res1(cpu)  (((cpu)->cp0[cp0_Wired].val & cp0_wired_res1_mask) >> cp0_wired_res1_shift)

#define cp0_context_res1_mask     UINT64_C(0x000000000000000f)
#define cp0_context_badvpn2_mask  UINT64_C(0x00000000007ffff0)
#define cp0_context_ptebase_mask  UINT64_C(0xffffffffff800000)

#define cp0_context_res1_shift     0
#define cp0_context_badvpn2_shift  4
#define cp0_context_ptebase_shift  23
#define cp0_context_addr_shift     9

#define cp0_context_res1(cpu)     (((cpu)->cp0[cp0_Context].val & cp0_context_res1_mask) >> cp0_context_res1_shift)
#define cp0_context_badvpn2(cpu)  (((cpu)->cp0[cp0_Context].val & cp0_context_badvpn2_mask) >> cp0_context_badvpn2_shift)
#define cp0_context_ptebase(cpu)  (((cpu)->cp0[cp0_Context].val & cp0_context_ptebase_mask) >> cp0_context_ptebase_shift)

#define cp0_pagemask_res1_mask  UINT32_C(0x00001fff)
#define cp0_pagemask_mask_mask  UINT32_C(0x01ffe000)
#define cp0_pagemask_res2_mask  UINT32_C(0xfe000000)

#define cp0_pagemask_res1_shift  0
#define cp0_pagemask_mask_shift  13
#define cp0_pagemask_res2_shift  25

#define cp0_pagemask_res1(cpu)  (((cpu)->cp0[cp0_PageMask].val & cp0_pagemask_res1_mask) >> cp0_pagemask_res1_shift)
#define cp0_pagemask_mask(cpu)  (((cpu)->cp0[cp0_PageMask].val & cp0_pagemask_mask_mask) >> cp0_pagemask_mask_shift)
#define cp0_pagemask_res2(cpu)  (((cpu)->cp0[cp0_PageMask].val & cp0_pagemask_res2_mask) >> cp0_pagemask_res2_shift)

#define cp0_cause_res1_mask     UINT32_C(0x00000003)
#define cp0_cause_exccode_mask  UINT32_C(0x0000007c)
#define cp0_cause_res2_mask     UINT32_C(0x00000080)
#define cp0_cause_ip0_mask      UINT32_C(0x00000100)
#define cp0_cause_ip1_mask      UINT32_C(0x00000200)
#define cp0_cause_ip_mask       UINT32_C(0x0000ff00)
#define cp0_cause_res3_mask     UINT32_C(0x0fff0000)
#define cp0_cause_ce_mask       UINT32_C(0x30000000)
#define cp0_cause_bd_mask       UINT32_C(0x80000000)
#define cp0_cause_res4_mask     UINT32_C(0x40000000)

#define cp0_cause_ce_cu1  UINT32_C(0x10000000)
#define cp0_cause_ce_cu2  UINT32_C(0x20000000)
#define cp0_cause_ce_cu3  UINT32_C(0x30000000)

#define cp0_cause_res1_shift     0
#define cp0_cause_exccode_shift  2
#define cp0_cause_res2_shift     7
#define cp0_cause_ip_shift       8
#define cp0_cause_ip0_shift      8
#define cp0_cause_ip1_shift      9
#define cp0_cause_ip2_shift      10
#define cp0_cause_ip3_shift      11
#define cp0_cause_ip4_shift      12
#define cp0_cause_ip5_shift      13
#define cp0_cause_ip6_shift      14
#define cp0_cause_ip7_shift      15
#define cp0_cause_res3_shift     16
#define cp0_cause_ce_shift       28
#define cp0_cause_res4_shift     30
#define cp0_cause_bd_shift       31

#define cp0_cause_res1(cpu)     (((cpu)->cp0[cp0_Cause].val & cp0_cause_res1_mask) >> cp0_cause_res1_shift)
#define cp0_cause_exccode(cpu)  (((cpu)->cp0[cp0_Cause].val & cp0_cause_exccode_mask) >> cp0_cause_exccode_shift)
#define cp0_cause_res2(cpu)     (((cpu)->cp0[cp0_Cause].val & cp0_cause_res2_mask) >> cp0_cause_res2_shift)
#define cp0_cause_ip(cpu)       (((cpu)->cp0[cp0_Cause].val & cp0_cause_ip_mask) >> cp0_cause_ip_shift)
#define cp0_cause_res3(cpu)     (((cpu)->cp0[cp0_Cause].val & cp0_cause_res3_mask) >> cp0_cause_res3_shift)
#define cp0_cause_ce(cpu)       (((cpu)->cp0[cp0_Cause].val & cp0_cause_ce_mask) >> cp0_cause_ce_shift)
#define cp0_cause_res4(cpu)     (((cpu)->cp0[cp0_Cause].val & cp0_cause_res4_mask) >> cp0_cause_res4_shift)
#define cp0_cause_bd(cpu)       (((cpu)->cp0[cp0_Cause].val & cp0_cause_bd_mask) >> cp0_cause_bd_shift)

#define cp0_prid_rev_mask  UINT32_C(0x000000ff)
#define cp0_prid_imp_mask  UINT32_C(0x0000ff00)
#define cp0_prid_res_mask  UINT32_C(0xffff0000)

#define cp0_prid_rev_shift  0
#define cp0_prid_imp_shift  8
#define cp0_prid_res_shift  16

#define cp0_prid_rev(cpu)  (((cpu)->cp0[cp0_PRId].val & cp0_prid_rev_mask) >> cp0_prid_rev_shift)
#define cp0_prid_imp(cpu)  (((cpu)->cp0[cp0_PRId].val & cp0_prid_imp_mask) >> cp0_prid_imp_shift)
#define cp0_prid_res(cpu)  (((cpu)->cp0[cp0_PRId].val & cp0_prid_res_mask) >> cp0_prid_res_shift)

#define cp0_config_k0_mask   UINT32_C(0x00000007)
#define cp0_config_cu_mask   UINT32_C(0x00000008)
#define cp0_config_db_mask   UINT32_C(0x00000010)
#define cp0_config_b_mask    UINT32_C(0x00000020)
#define cp0_config_dc_mask   UINT32_C(0x000003c0)
#define cp0_config_ic_mask   UINT32_C(0x00003c00)
#define cp0_config_res_mask  UINT32_C(0x00001000)
#define cp0_config_eb_mask   UINT32_C(0x00002000)
#define cp0_config_em_mask   UINT32_C(0x00004000)
#define cp0_config_be_mask   UINT32_C(0x00008000)
#define cp0_config_sm_mask   UINT32_C(0x00010000)
#define cp0_config_sc_mask   UINT32_C(0x00020000)
#define cp0_config_ew_mask   UINT32_C(0x000c0000)
#define cp0_config_sw_mask   UINT32_C(0x00100000)
#define cp0_config_ss_mask   UINT32_C(0x00200000)
#define cp0_config_sb_mask   UINT32_C(0x00c00000)
#define cp0_config_ep_mask   UINT32_C(0x0f000000)
#define cp0_config_ec_mask   UINT32_C(0x70000000)
#define cp0_config_cm_mask   UINT32_C(0x80000000)

#define cp0_config_k0_shift   0
#define cp0_config_cu_shift   3
#define cp0_config_db_shift   4
#define cp0_config_b_shift    5
#define cp0_config_dc_shift   6
#define cp0_config_ic_shift   9
#define cp0_config_res_shift  12
#define cp0_config_eb_shift   13
#define cp0_config_em_shift   14
#define cp0_config_be_shift   15
#define cp0_config_sm_shift   16
#define cp0_config_sc_shift   17
#define cp0_config_ew_shift   18
#define cp0_config_sw_shift   20
#define cp0_config_ss_shift   21
#define cp0_config_sb_shift   22
#define cp0_config_ep_shift   24
#define cp0_config_ec_shift   28
#define cp0_config_cm_shift   31

#define cp0_config_k0(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_k0_mask) >> cp0_config_k0_shift)
#define cp0_config_cu(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_cu_mask) >> cp0_config_cu_shift)
#define cp0_config_db(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_db_mask) >> cp0_config_db_shift)
#define cp0_config_b(cpu)   (((cpu)->cp0[cp0_Config].val & cp0_config_b_mask) >> cp0_config_b_shift)
#define cp0_config_dc(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_dc_mask) >> cp0_config_dc_shift)
#define cp0_config_ic(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_ic_mask) >> cp0_config_ic_shift)
#define cp0_config_res(cpu) (((cpu)->cp0[cp0_Config].val & cp0_config_res_mask) >>cp0_config_res_shift)
#define cp0_config_eb(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_eb_mask) >> cp0_config_eb_shift)
#define cp0_config_em(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_em_mask) >> cp0_config_em_shift)
#define cp0_config_be(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_be_mask) >> cp0_config_be_shift)
#define cp0_config_sm(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_sm_mask) >> cp0_config_sm_shift)
#define cp0_config_sc(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_sc_mask) >> cp0_config_sc_shift)
#define cp0_config_ew(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_ew_mask) >> cp0_config_ew_shift)
#define cp0_config_sw(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_sw_mask) >> cp0_config_sw_shift)
#define cp0_config_ss(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_ss_mask) >> cp0_config_ss_shift)
#define cp0_config_sb(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_sb_mask) >> cp0_config_sb_shift)
#define cp0_config_ep(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_ep_mask) >> cp0_config_ep_shift)
#define cp0_config_ec(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_ec_mask) >> cp0_config_ec_shift)
#define cp0_config_cm(cpu)  (((cpu)->cp0[cp0_Config].val & cp0_config_cm_mask) >> cp0_config_cm_shift)

#define cp0_watchlo_w_mask       UINT32_C(0x00000001)
#define cp0_watchlo_r_mask       UINT32_C(0x00000002)
#define cp0_watchlo_res_mask     UINT32_C(0x00000004)
#define cp0_watchlo_paddr0_mask  UINT32_C(0xfffffff8)

#define cp0_watchlo_w_shift       0
#define cp0_watchlo_r_shift       1
#define cp0_watchlo_res_shift     2
#define cp0_watchlo_paddr0_shift  3

#define cp0_watchlo_w(cpu)       (((cpu)->cp0[cp0_WatchLo].val & cp0_watchlo_w_mask) >> cp0_watchlo_w_shift)
#define cp0_watchlo_r(cpu)       (((cpu)->cp0[cp0_WatchLo].val & cp0_watchlo_r_mask) >> cp0_watchlo_r_shift)
#define cp0_watchlo_res(cpu)     (((cpu)->cp0[cp0_WatchLo].val & cp0_watchlo_res_mask) >> cp0_watchlo_res_shift)
#define cp0_watchlo_paddr0(cpu)  (((cpu)->cp0[cp0_WatchLo].val & cp0_watchlo_paddr0_mask) >> cp0_watchlo_paddr0_shift)

#define cp0_watchhi_paddr1_mask  UINT32_C(0x0000000f)
#define cp0_watchhi_res_mask     UINT32_C(0xfffffff0)

#define cp0_watchhi_paddr1_shift  0
#define cp0_watchhi_res_shift     4

#define cp0_watchhi_paddr1(cpu)  (((cpu)->cp0[cp0_WatchHi].val & cp0_watchhi_paddr1_mask) >> cp0_watchhi_paddr1_shift)
#define cp0_watchhi_res(cpu)     (((cpu)->cp0[cp0_WatchHi].val & cp0_watchhi_res_mask) >> cp0_watchhi_res_shift)

#define cp0_ecc_ecc_mask  UINT32_C(0x000000ff)
#define cp0_ecc_res_mask  UINT32_C(0xffffff00)

#define cp0_ecc_ecc_shift  0
#define cp0_ecc_res_shift  8

#define cp0_ecc_ecc(cpu)  (((cpu)->cp0[cp0_ECC].val & cp0_ecc_ecc_mask) >> cp0_ecc_ecc_shift)
#define cp0_ecc_res(cpu)  (((cpu)->cp0[cp0_ECC].val & cp0_ecc_res_mask) >> cp0_ecc_res_shift)

#define cp0_xcontext_res1_mask     UINT64_C(0x000000000000000f)
#define cp0_xcontext_badvpn2_mask  UINT64_C(0x000000007ffffff8)
#define cp0_xcontext_r_mask        UINT64_C(0x0000000180000000)
#define cp0_xcontext_ptebase_mask  UINT64_C(0xfffffffe00000000)

#define cp0_xcontext_res1_shift     0
#define cp0_xcontext_badvpn2_shift  4
#define cp0_xcontext_r_shift        31
#define cp0_xcontext_ptebase_shift  33
#define cp0_xcontext_addr_shift     9

#define cp0_xcontext_res1(cpu)     (((cpu)->cp0[cp0_XContext].val & cp0_xcontext_res1_mask) >> cp0_xcontext_res1_shift)
#define cp0_xcontext_badvpn2(cpu)  (((cpu)->cp0[cp0_XContext].val & cp0_xcontext_badvpn2_mask) >> cp0_xcontext_badvpn2_shift)
#define cp0_xcontext_r(cpu)        (((cpu)->cp0[cp0_XContext].val & cp0_xcontext_r_mask) >> cp0_xcontext_r_shift)
#define cp0_xcontext_ptebase(cpu)  (((cpu)->cp0[cp0_XContext].val & cp0_xcontext_ptebase_mask) >> cp0_xcontext_ptebase_shift)

#define cp0_errorepc(cpu)  ((cpu)->cp0[cp0_ErrorEPC])

#define cp0_index(cpu)     ((cpu)->cp0[cp0_Index])
#define cp0_random(cpu)    ((cpu)->cp0[cp0_Random])
#define cp0_entrylo0(cpu)  ((cpu)->cp0[cp0_EntryLo0])
#define cp0_entrylo1(cpu)  ((cpu)->cp0[cp0_EntryLo1])
#define cp0_context(cpu)   ((cpu)->cp0[cp0_Context])
#define cp0_pagemask(cpu)  ((cpu)->cp0[cp0_PageMask])
#define cp0_wired(cpu)     ((cpu)->cp0[cp0_Wired])
#define cp0_badvaddr(cpu)  ((cpu)->cp0[cp0_BadVAddr])
#define cp0_count(cpu)     ((cpu)->cp0[cp0_Count])
#define cp0_entryhi(cpu)   ((cpu)->cp0[cp0_EntryHi])
#define cp0_compare(cpu)   ((cpu)->cp0[cp0_Compare])
#define cp0_cause(cpu)     ((cpu)->cp0[cp0_Cause])
#define cp0_config(cpu)    ((cpu)->cp0[cp0_Config])
#define cp0_prid(cpu)      ((cpu)->cp0[cp0_PRId])
#define cp0_status(cpu)    ((cpu)->cp0[cp0_Status])
#define cp0_epc(cpu)       ((cpu)->cp0[cp0_EPC])
#define cp0_lladdr(cpu)    ((cpu)->cp0[cp0_LLAddr])
#define cp0_watchlo(cpu)   ((cpu)->cp0[cp0_WatchLo])
#define cp0_watchhi(cpu)   ((cpu)->cp0[cp0_WatchHi])
#define cp0_ecc(cpu)       ((cpu)->cp0[cp0_ECC])
#define cp0_xcontext(cpu)  ((cpu)->cp0[cp0_XContext])
#define cp0_taglo(cpu)     ((cpu)->cp0[cp0_TagLo])
#define cp0_taghi(cpu)     ((cpu)->cp0[cp0_TagHi])

/** cp0 Masks */
#define cp0_SR_EXLMask  UINT32_C(0x00000002)

/** TLB entity definition */
typedef struct {
	ptr36_t pfn;   /**< Physical page number (shifted << 12) */
	uint8_t cohh;  /**< Coherency number */
	bool dirty;    /**< Dirty bit */
	bool valid;    /**< Valid bit */
} tlb_rec_t;

typedef struct {
	uint32_t mask;    /**< Enhanced mask */
	uint32_t vpn2;    /**< Virtual page no (shifted << 7) */
	bool global;      /**< Global bit */
	uint8_t asid;     /**< Address Space ID */
	tlb_rec_t pg[2];  /**< Subpages */
} tlb_entry_t;

typedef enum {
	BRANCH_NONE = 0,
	BRANCH_PASSED = 1,
	BRANCH_COND = 2
} branch_state_t;

struct frame;
struct r4k_cpu;
union r4k_instr;

/** Instruction implementation */
typedef exc_t (*instr_fnc_t)(struct r4k_cpu *, union r4k_instr);

/** Main processor structure */
typedef struct r4k_cpu {
	/* Basic run-time support */
	unsigned int procno;
	bool stdby;
	struct frame *frame;
	instr_fnc_t trans[FRAME_SIZE/sizeof(instr_fnc_t)];
	
	/* Standard registers */
	reg64_t regs[R4K_REG_COUNT];
	reg64_t cp0[R4K_REG_COUNT];
	uint64_t fpregs[R4K_REG_COUNT];
	reg64_t loreg;
	reg64_t hireg;
	
	/* Program counter */
	ptr64_t pc;
	ptr64_t pc_next;
	
	/* TLB structures */
	tlb_entry_t tlb[TLB_ENTRIES];
	unsigned int tlb_hint;
	
	/* Old registers (for debug info) */
	reg64_t old_regs[R4K_REG_COUNT];
	reg64_t old_cp0[R4K_REG_COUNT];
	reg64_t old_loreg;
	reg64_t old_hireg;
	
	ptr64_t excaddr;
	branch_state_t branch;
	
	/* LL and SC track support */
	bool llbit;      /**< Track the address flag */
	ptr36_t lladdr;  /**< Physical tracked address */
	
	/* Watch support */
	ptr36_t waddr;
	ptr64_t wexcaddr;
	bool wpending;
	
	/* Statistics */
	uint64_t k_cycles;
	uint64_t u_cycles;
	uint64_t w_cycles;
	
	uint64_t tlb_refill;
	uint64_t tlb_invalid;
	uint64_t tlb_modified;
	uint64_t intr[INTR_COUNT];
	
	/* breakpoints */
	list_t bps;
} r4k_cpu_t;

/** Instruction decoding structure */
typedef union r4k_instr {
	uint32_t val;
#ifdef WORDS_BIGENDIAN
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int imm : 16;
	} i;
	struct {
		unsigned int opcode : 6;
		unsigned int target : 26;
	} j;
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int sa : 5;
		unsigned int func : 6;
	} r;
	struct {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int data : 24;
		unsigned int func : 8;
	} cop;
	struct {
		unsigned int opcode : 6;
		unsigned int code : 20;
		unsigned int func : 6;
	} sys;
#else
	struct {
		unsigned int imm : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} i;
	struct {
		unsigned int target : 26;
		unsigned int opcode : 6;
	} j;
	struct {
		unsigned int func : 6;
		unsigned int sa : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} r;
	struct {
		unsigned int func : 8;
		unsigned int data : 24;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	} cop;
	struct {
		unsigned int func : 6;
		unsigned int code : 20;
		unsigned int opcode : 6;
	} sys;
#endif
} r4k_instr_t;



/** Opcode numbers
 *
 */
typedef enum {
	/* 0 */
	opcSPECIAL = 0,
	opcREGIMM = 1,
	opcJ = 2,
	opcJAL = 3,
	opcBEQ = 4,
	opcBNE = 5,
	opcBLEZ = 6,
	opcBGTZ = 7,
	
	/* 8 */
	opcADDI = 8,
	opcADDIU = 9,
	opcSLTI = 10,
	opcSLTIU = 11,
	opcANDI = 12,
	opcORI = 13,
	opcXORi = 14,
	opcLUI = 15,
	
	/* 16 */
	opcCOP0 = 16,
	opcCOP1 = 17,
	opcCOP2 = 18,
	/* opcode 19 unused */
	opcBEQL = 20,
	opcBNEL = 21,
	opcBLEZL = 22,
	opcBGTZL = 23,
	
	/* 24 */
	opcDADDI = 24,
	opcDADDIU = 25,
	opcLDL = 26,
	opcLDR = 27,
	/* opcode 28 unused */
	/* opcode 29 unused */
	/* opcode 30 unused */
	/* opcode 31 unused */
	
	/* 32 */
	opcLB = 32,
	opcLH = 33,
	opcLWL = 34,
	opcLW = 35,
	opcLBU = 36,
	opcLHU = 37,
	opcLWR = 38,
	opcLWU = 39,
	
	/* 40 */
	opcSB = 40,
	opcSH = 41,
	opcSWL = 42,
	opcSW = 43,
	opcSDL = 44,
	opcSDR = 45,
	opcSWR = 46,
	opcCACHE = 47,
	
	/* 48 */
	opcLL = 48,
	opcLWC1 = 49,
	opcLWC2 = 50,
	/* opcode 51 unused */
	opcLDD = 52,
	opcLDC1 = 53,
	opcLDC2 = 54,
	opcLD = 55,
	
	/* 56 */
	opcSC = 56,
	opcSWC1 = 57,
	opcSWC2 = 58,
	/* opcode 59 unused */
	opcSCD = 60,
	opcSDC1 = 61,
	opcSDC2 = 62,
	opcSD = 63
} instr_opcode_t;

/** Function numbers
 *
 * For opcSPECIAL instructions.
 *
 */
typedef enum {
	/* 0 */
	funcSLL = 0,
	/* function 1 unused */
	funcSRL = 2,
	funcSRA = 3,
	funcSLLV = 4,
	/* function 5 unused */
	funcSRLV = 6,
	funcSRAV = 7,
	
	/* 8 */
	funcJR = 8,
	funcJALR = 9,
	/* function 10 unused */
	/* function 11 unused */
	funcSYSCALL = 12,
	funcBREAK = 13,
	/* function 14 unused */
	funcSYNC = 15,
	
	/* 16 */
	funcMFHI = 16,
	funcMTHI = 17,
	funcMFLO = 18,
	funcMTLO = 19,
	funcDSLLV = 20,
	/* function 21 unused */
	funcDSRLV = 22,
	funcDSRAV = 23,
	
	/* 24 */
	funcMULT = 24,
	funcMULTU = 25,
	funcDIV = 26,
	funcDIVU = 27,
	funcDMULT = 28,
	funcDMULTU = 29,
	funcDDIV = 30,
	funcDDIVU = 31,
	
	/* 32 */
	funcADD = 32,
	funcADDU = 33,
	funcSUB = 34,
	funcSUBU = 35,
	funcAND = 36,
	funcOR = 37,
	funcXOR = 38,
	funcNOR = 39,
	
	/* 40 */
	/* function 40 unused */
	func_XINT = 41,
	funcSLT = 42,
	funcSLTU = 43,
	funcDADD = 44,
	funcDADDU = 45,
	funcDSUB = 46,
	funcDSUBu = 47,
	
	/* 48 */
	funcTGE = 48,
	funcTGEU = 49,
	funcTLT = 50,
	funcTLTU = 51,
	funcTEQ = 52,
	/* function 53 unused */
	funcTNE = 54,
	/* function 55 unused */
	
	/* 56 */
	funcDSLL = 56,
	/* function 57 unused */
	funcDSRL = 58,
	funcDSRA = 59,
	funcSLL32 = 60,
	/* function 61 unused */
	funcDSRL32 = 62,
	funcDSRA32 = 63
} instr_func_t;

/** Register rt numbers
 *
 * For opcREGIMM instructions.
 *
 */
typedef enum {
	/* 0 */
	rtBLTZ = 0,
	rtBGEZ = 1,
	rtBLTZL = 2,
	rtBGEZL = 3,
	/* rt 4 unused */
	/* rt 5 unused */
	/* rt 6 unused */
	/* rt 7 unused */
	
	/* 8 */
	rtTGEI = 8,
	rtTGEIU = 9,
	rtTLTI = 10,
	rtTLTIU = 11,
	rtTEQI = 12,
	/* rt 13 unused */
	rtTNEI = 14,
	/* rt 15 unused */
	
	/* 16 */
	rtBLTZAL = 16,
	rtBGEZAL = 17,
	rtBLTZALL = 18,
	rtBGEZALL = 19
} instr_rt_t;

typedef enum {
	/* 0 */
	cop0rsMFC0 = 0,
	cop0rsDMFC0 = 1,
	/* rs 2 unused */
	/* rs 3 unused */
	cop0rsMTC0 = 4,
	cop0rsDMTC0 = 5,
	/* rs 6 unused */
	/* rs 7 unused */
	
	/* 8 */
	cop0rsBC = 8,
	/* rs 9 unused */
	/* rs 10 unused */
	/* rs 11 unused */
	/* rs 12 unused */
	/* rs 13 unused */
	/* rs 14 unused */
	/* rs 15 unused */
	
	/* 16 */
	cop0rsCO = 16
} instr_cop0rs_t;

typedef enum {
	/* 0 */
	cop1rsMFC1 = 0,
	cop1rsDMFC1 = 1,
	cop1rsCFC1 = 2,
	/* rs 3 unused */
	cop1rsMTC1 = 4,
	cop1rsDMTC1 = 5,
	cop1rsCTC1 = 6,
	/* rs 7 unused */
	
	/* 8 */
	cop1rsBC = 8
} instr_cop1rs_t;

typedef enum {
	/* 0 */
	cop2rsMFC2 = 0,
	/* rs 1 unused */
	cop2rsCFC2 = 2,
	/* rs 3 unused */
	/* rs 4 unused */
	/* rs 5 unused */
	cop2rsCTC2 = 6,
	/* rs 7 unused */
	
	/* 8 */
	cop2rsBC = 8
} instr_cop2rs_t;

typedef enum {
	/* 0 */
	cop0rtBC0F = 0,
	cop0rtBC0T = 1,
	cop0rtBC0FL = 2,
	cop0rtBC0TL = 3
} instr_cop0rt_t;

typedef enum {
	/* 0 */
	cop1rtBC1F = 0,
	cop1rtBC1T = 1,
	cop1rtBC1FL = 2,
	cop1rtBC1TL = 3
} instr_cop1rt_t;

typedef enum {
	/* 0 */
	cop2rtBC2F = 0,
	cop2rtBC2T = 1,
	cop2rtBC2FL = 2,
	cop2rtBC2TL = 3
} instr_cop2rt_t;

typedef enum {
	/* 0 */
	/* function 0 unused */
	cop0funcTLBR = 1,
	cop0funcTLBWI = 2,
	/* function 3 unused */
	/* function 4 unused */
	/* function 5 unused */
	cop0funcTLBWR = 6,
	/* function 7 unused */
	
	/* 8 */
	cop0funcTLBP = 8,
	/* function 9 unused */
	/* function 10 unused */
	/* function 11 unused */
	/* function 12 unused */
	/* function 13 unused */
	/* function 14 unused */
	/* function 15 unused */
	
	/* 16 */
	cop0funcERET = 16
} instr_cop0func_t;

/** Instruction decoding tables and mnemonics
 *
 */
typedef void (*mnemonics_fnc_t)(ptr64_t, r4k_instr_t, string_t *, string_t *);

/** Register and coprocessor names */
extern char *reg_name[R4K_REG_VARIANTS][R4K_REG_COUNT];
extern char *cp0_name[R4K_REG_VARIANTS][R4K_REG_COUNT];
extern char *cp1_name[R4K_REG_VARIANTS][R4K_REG_COUNT];
extern char *cp2_name[R4K_REG_VARIANTS][R4K_REG_COUNT];
extern char *cp3_name[R4K_REG_VARIANTS][R4K_REG_COUNT];

/** Decode instruction mnemonics */
extern mnemonics_fnc_t decode_mnemonics(r4k_instr_t);

/** Basic CPU routines */
extern void r4k_cpu_init(r4k_cpu_t *cpu, unsigned int procno);
extern void r4k_cpu_set_pc(r4k_cpu_t *cpu, ptr64_t value);
extern void r4k_cpu_step(r4k_cpu_t *cpu);

/** Addresing function */
extern exc_t convert_addr(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write,
    bool noisy);

/** Virtual memory access */
extern exc_t cpu_read_mem32(r4k_cpu_t *cpu, ptr64_t addr, uint32_t *value,
    bool noisy);

/** Interrupts */
extern void r4k_cpu_interrupt_up(r4k_cpu_t *cpu, unsigned int no);
extern void r4k_cpu_interrupt_down(r4k_cpu_t *cpu, unsigned int no);

#endif
