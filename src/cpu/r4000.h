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

#ifndef R4000_H_
#define R4000_H_

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "../list.h"
#include "instr.h"

#define FRAME_WIDTH   12
#define FRAME_SIZE    (1 << FRAME_WIDTH)
#define FRAME_MASK    (FRAME_SIZE - 1)

#define TLB_ENTRIES   48
#define REG_COUNT     32
#define INTR_COUNT    8
#define TLB_PHYSMASK  UINT64_C(0x780000000)

#define FRAMES2SIZE(frames) \
	(((len36_t) (frames)) << FRAME_WIDTH)

#define SIZE2FRAMES(size) \
	((size) >> FRAME_WIDTH)

#define FRAME2ADDR(frame) \
	(((ptr36_t) (frame)) << FRAME_WIDTH)

#define ADDR2FRAME(addr) \
	((addr) >> FRAME_WIDTH)

#define INSTR2ADDR(instr) \
	((instr) << 2)

#define ADDR2INSTR(addr) \
	((addr) >> 2)

#define INSTRS2SIZE(instrs) \
	((instrs) << 2)

#define SIZE2INSTRS(size) \
	((size) >> 2)

#define DEFAULT_MEMORY_VALUE  UINT64_C(0xffffffffffffffff)

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

#define cp0_entryhi_asid_mask  UINT32_C(0x000000ff)
#define cp0_entryhi_res1_mask  UINT32_C(0x00001f00)
#define cp0_entryhi_vpn2_mask  UINT32_C(0xffffe000)

#define cp0_entryhi_asid_shift  0
#define cp0_entryhi_res1_shift  8
#define cp0_entryhi_vpn2_shift  13

#define cp0_entryhi_asid(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_asid_mask) >> 0)
#define cp0_entryhi_res1(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_res1_shift) >> 8)
#define cp0_entryhi_vpn2(cpu)  (((cpu)->cp0[cp0_EntryHi].val & cp0_entryhi_vpn2_shift) >> 13)

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

#define cp0_context_res1_mask     UINT32_C(0x0000000f)
#define cp0_context_badvpn2_mask  UINT32_C(0x007ffff0)
#define cp0_context_ptebase_mask  UINT32_C(0xff800000)

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

#define cp0_xcontext_res1_mask     UINT32_C(0x0000000f)
#define cd0_xcontext_badvpn2_mask  UINT32_C(0x00000000)
#define cp0_xcontext_r_mask        UINT32_C(0x00000000)
#define cp0_xcontext_ptebase_mask  UINT32_C(0x00000000)

#define cp0_xcontext_res1_shift     0
#define cp0_xcontext_badvpn2_shift  4
#define cp0_xcontext_r_shift        31
#define cp0_xcontext_ptebase_shift  33

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
#define cp0_taglo(cpu)     ((cpu)->cp0[cp0_TagLo])
#define cp0_taghi(cpu)     ((cpu)->cp0[cp0_TagHi])

/** cp0 Masks */
#define cp0_SR_EXLMask  UINT32_C(0x00000002)

/** TLB entity definition */
typedef struct {
	ptr36_t pfn;   /**< Physical page no (shifted << 12) */
	uint8_t cohh;  /**< Coherency number */
	bool dirty;    /**< Dirty bit */
	bool valid;    /**< Valid bit */
} tlb_ent_value_t;

typedef struct {
	uint32_t mask;          /**< Enhanced mask */
	uint32_t vpn2;          /**< Virtual page no (shifted << 7) */
	bool global;            /**< Global bit */
	uint8_t asid;           /**< Address Space ID */
	tlb_ent_value_t pg[2];  /**< Subpages */
} tlb_entry_t;

typedef enum {
	BRANCH_NONE = 0,
	BRANCH_PASSED = 1,
	BRANCH_COND = 2
} branch_state_t;

struct frame;

/** Main processor structure */
typedef struct {
	/* Basic run-time support */
	unsigned int procno;
	bool stdby;
	struct frame *frame;
	
	/* Standard registers */
	reg64_t regs[REG_COUNT];
	reg64_t cp0[REG_COUNT];
	uint64_t fpregs[REG_COUNT];
	reg64_t loreg;
	reg64_t hireg;
	
	/* Program counter */
	ptr64_t pc;
	ptr64_t pc_next;
	
	/* TLB structures */
	tlb_entry_t tlb[TLB_ENTRIES];
	unsigned int tlb_hint;
	
	/* Old registers (for debug info) */
	reg64_t old_regs[REG_COUNT];
	reg64_t old_cp0[REG_COUNT];
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
} cpu_t;

/** Instruction implementation */
typedef exc_t (*instr_fnc_t)(cpu_t *, instr_t);

typedef enum {
	MEMT_NONE = 0,  /**< Uninitialized */
	MEMT_MEM  = 1,  /**< Generic */
	MEMT_FMAP = 2   /**< File mapped */
} physmem_type_t;

typedef struct {
	/* Memory area type */
	physmem_type_t type;
	bool writable;
	
	/* Starting physical frame */
	pfn_t start;
	
	/* Number of physical frames */
	pfn_t count;
	
	/* Memory content */
	uint8_t *data;
	
	/* Binary translated memory content */
	instr_fnc_t *trans;
} physmem_area_t;

typedef struct frame {
	/* Physical memory area containing the frame */
	physmem_area_t *area;
	
	/* Frame data (with displacement) */
	uint8_t *data;
	
	/* Binary translated instructions (with displacement) */
	instr_fnc_t *trans;
	
	/* Binary translation valid flag */
	bool valid;
} frame_t;

/** Physical memory management */
extern void physmem_wire(physmem_area_t *area);
extern void physmem_unwire(physmem_area_t *area);

/** Basic CPU routines */
extern void cpu_init(cpu_t *cpu, unsigned int procno);
extern void cpu_set_pc(cpu_t *cpu, ptr64_t value);
extern void cpu_step(cpu_t *cpu);

/** Physical memory access */
extern uint8_t physmem_read8(cpu_t *cpu, ptr36_t addr, bool protected);
extern uint16_t physmem_read16(cpu_t *cpu, ptr36_t addr, bool protected);
extern uint32_t physmem_read32(cpu_t *cpu, ptr36_t addr, bool protected);
extern uint64_t physmem_read64(cpu_t *cpu, ptr36_t addr, bool protected);

extern bool physmem_write8(cpu_t *cpu, ptr36_t addr, uint8_t val,
    bool protected);
extern bool physmem_write16(cpu_t *cpu, ptr36_t addr, uint16_t val,
    bool protected);
extern bool physmem_write32(cpu_t *cpu, ptr36_t addr, uint32_t val,
    bool protected);
extern bool physmem_write64(cpu_t *cpu, ptr36_t addr, uint64_t val,
    bool protected);

/** Addresing function */
extern exc_t convert_addr(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write,
    bool noisy);

/** Virtual memory access */
extern exc_t cpu_read_mem32(cpu_t *cpu, ptr64_t addr, uint32_t *value,
    bool noisy);

/** Interrupts */
extern void cpu_interrupt_up(cpu_t *cpu, unsigned int no);
extern void cpu_interrupt_down(cpu_t *cpu, unsigned int no);

#endif
