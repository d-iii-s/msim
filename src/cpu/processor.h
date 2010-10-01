/*
 * Copyright (c) 2000-2008 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Processor simulation
 *
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "../mtypes.h"
#include "../list.h"
#include "instr.h"


/* cp0 registers */
enum cp0_regs {
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
};


#define cp0_index_index_mask 0x0000003fU
#define cp0_index_res_mask   0x7fffffc0U
#define cp0_index_p_mask     0x80000000U

#define cp0_index_index_shift 0
#define cp0_index_res_shift   6
#define cp0_index_p_shift     31

#define cp0_index_index \
	((pr->cp0[cp0_Index] \
	& cp0_index_index_mask) >> cp0_index_index_shift)
#define cp0_index_res \
	((pr->cp0[cp0_Index] \
	& cp0_index_res_mask) >> cp0_index_res_shift)
#define cp0_index_p \
	((pr->cp0[cp0_Index] \
	& cp0_index_p_mask) >> cp0_index_p_shift)


#define cp0_random_random_mask 0x0000003fU
#define cp0_random_res_mask    0xffffffc0U

#define cp0_random_random_shift 0
#define cp0_random_res_shift    6

#define cp0_random_random \
	((pr->cp0[ cp0_Random] \
	& cp0_random_random_mask) >> cp0_random_random_shift)
#define cp0_random_res \
	((pr->cp0[ cp0_Random] \
	& cp0_random_res_mask) >> cp0_random_res_shift)


#define cp0_status_ie_mask   0x00000001U
#define cp0_status_exl_mask  0x00000002U
#define cp0_status_erl_mask  0x00000004U
#define cp0_status_ksu_mask  0x00000018U
#define cp0_status_ux_mask   0x00000020U
#define cp0_status_sx_mask   0x00000040U
#define cp0_status_kx_mask   0x00000080U
#define cp0_status_im_mask   0x0000ff00U
#define cp0_status_de_mask   0x00010000U
#define cp0_status_ce_mask   0x00020000U
#define cp0_status_ch_mask   0x00040000U
#define cp0_status_res1_mask 0x00080000U
#define cp0_status_sr_mask   0x00100000U
#define cp0_status_ts_mask   0x00200000U
#define cp0_status_bev_mask  0x00400000U
#define cp0_status_res2_mask 0x01800000U
#define cp0_status_re_mask   0x02000000U
#define cp0_status_fr_mask   0x04000000U
#define cp0_status_rp_mask   0x08000000U
#define cp0_status_cu0_mask  0x10000000U
#define cp0_status_cu1_mask  0x20000000U
#define cp0_status_cu2_mask  0x40000000U
#define cp0_status_cu3_mask  0x80000000U
#define cp0_status_cu_mask   0xf0000000U

#define cp0_status_ie_shift   0
#define cp0_status_exl_shift  1
#define cp0_status_erl_shift  2
#define cp0_status_ksu_shift  3
#define cp0_status_ux_shift   5
#define cp0_status_sx_shift   6
#define cp0_status_kx_shift   7
#define cp0_status_im_shift   8
#define cp0_status_de_shift   16
#define cp0_status_ce_shift   17
#define cp0_status_ch_shift   18
#define cp0_status_res1_shift 19
#define cp0_status_sr_shift   20
#define cp0_status_ts_shift   21
#define cp0_status_bev_shift  22
#define cp0_status_res2_shift 23
#define cp0_status_re_shift   25
#define cp0_status_fr_shift   26
#define cp0_status_rp_shift   27
#define cp0_status_cu0_shift  28
#define cp0_status_cu1_shift  29
#define cp0_status_cu2_shift  30
#define cp0_status_cu3_shift  31
#define cp0_status_cu_shift   28

#define cp0_status_ie   ((pr->cp0[cp0_Status] & cp0_status_ie_mask) >> 0)
#define cp0_status_exl  ((pr->cp0[cp0_Status] & cp0_status_exl_mask) >> 1)
#define cp0_status_erl  ((pr->cp0[cp0_Status] & cp0_status_erl_mask) >> 2)
#define cp0_status_ksu  ((pr->cp0[cp0_Status] & cp0_status_ksu_mask) >> 3)
#define cp0_status_ux   ((pr->cp0[cp0_Status] & cp0_status_ux_mask) >> 5)
#define cp0_status_sx   ((pr->cp0[cp0_Status] & cp0_status_sx_mask) >> 6)
#define cp0_status_kx   ((pr->cp0[cp0_Status] & cp0_status_kx_mask) >> 7)
#define cp0_status_im   ((pr->cp0[cp0_Status] & cp0_status_im_mask) >> 8)
#define cp0_status_de   ((pr->cp0[cp0_Status] & cp0_status_de_mask) >> 16)
#define cp0_status_ce   ((pr->cp0[cp0_Status] & cp0_status_ce_mask) >> 17)
#define cp0_status_ch   ((pr->cp0[cp0_Status] & cp0_status_ch_mask) >> 18)
#define cp0_status_res1 ((pr->cp0[cp0_Status] & cp0_status_res1_mask) >> 19)
#define cp0_status_sr   ((pr->cp0[cp0_Status] & cp0_status_sr_mask) >> 20)
#define cp0_status_ts   ((pr->cp0[cp0_Status] & cp0_status_ts_mask) >> 21)
#define cp0_status_bev  ((pr->cp0[cp0_Status] & cp0_status_bev_mask) >> 22)
#define cp0_status_res2 ((pr->cp0[cp0_Status] & cp0_status_res2_mask) >> 23)
#define cp0_status_re   ((pr->cp0[cp0_Status] & cp0_status_re_mask) >> 25)
#define cp0_status_fr   ((pr->cp0[cp0_Status] & cp0_status_fr_mask) >> 26)
#define cp0_status_rp   ((pr->cp0[cp0_Status] & cp0_status_rp_mask) >> 27)
#define cp0_status_cu0  ((pr->cp0[cp0_Status] & cp0_status_cu0_mask) >> 28)
#define cp0_status_cu1  ((pr->cp0[cp0_Status] & cp0_status_cu1_mask) >> 29)
#define cp0_status_cu2  ((pr->cp0[cp0_Status] & cp0_status_cu2_mask) >> 30)
#define cp0_status_cu3  ((pr->cp0[cp0_Status] & cp0_status_cu3_mask) >> 31)
#define cp0_status_cu   ((pr->cp0[cp0_Status] & cp0_status_cu_mask) >> 28)

#define cp0_entryhi_asid_mask 0x000000ffU
#define cp0_entryhi_res1_mask 0x00001f00U
#define cp0_entryhi_vpn2_mask 0xffffe000U

#define cp0_entryhi_asid_shift 0
#define cp0_entryhi_res1_shift 8
#define cp0_entryhi_vpn2_shift 13

#define cp0_entryhi_asid ((pr->cp0[cp0_EntryHi] & cp0_entryhi_asid_mask) >> 0)
#define cp0_entryhi_res1 ((pr->cp0[cp0_EntryHi] & cp0_entryhi_res1_shift) >> 8)
#define cp0_entryhi_vpn2 ((pr->cp0[cp0_EntryHi] & cp0_entryhi_vpn2_shift) >> 13)

#define cp0_entrylo_g_mask    0x00000001U
#define cp0_entrylo_v_mask    0x00000002U
#define cp0_entrylo_d_mask    0x00000004U
#define cp0_entrylo_c_mask    0x00000038U
#define cp0_entrylo_pfn_mask  0x3fffffc0U
#define cp0_entrylo_res1_mask 0xc0000000U

#define cp0_entrylo_g_shift    0
#define cp0_entrylo_v_shift    1
#define cp0_entrylo_d_shift    2
#define cp0_entrylo_c_shift    3
#define cp0_entrylo_pfn_shift  6
#define cp0_entrylo_res1_shift 30

#define cp0_entrylo0_g \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_g_mask) >> cp0_entrylo_g_shift)
#define cp0_entrylo0_v \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_v_mask) >> cp0_entrylo_v_shift)
#define cp0_entrylo0_d \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_d_mask) >> cp0_entrylo_d_shift)
#define cp0_entrylo0_c \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_c_mask) >> cp0_entrylo_c_shift)
#define cp0_entrylo0_pfn \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_pfn_mask) >> cp0_entrylo_pfn_shift)
#define cp0_entrylo0_res1 \
	((pr->cp0[cp0_EntryLo0] \
	& cp0_entrylo_res1_mask) >> cp0_entrylo_res1_shift)

#define cp0_entrylo1_g \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_g_mask) >> cp0_entrylo_g_shift)
#define cp0_entrylo1_v \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_v_mask) >> cp0_entrylo_v_shift)
#define cp0_entrylo1_d \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_d_mask) >> cp0_entrylo_d_shift)
#define cp0_entrylo1_c \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_c_mask) >> cp0_entrylo_c_shift)
#define cp0_entrylo1_pfn \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_pfn_mask) >> cp0_entrylo_pfn_shift)
#define cp0_entrylo1_res1 \
	((pr->cp0[cp0_EntryLo1] \
	& cp0_entrylo_res1_mask) >> cp0_entrylo_res1_shift)

#define cp0_wired_w_mask    0x0000001fU
#define cp0_wired_res1_mask 0xffffffe0U

#define cp0_wired_w_shift    0
#define cp0_wired_res1_shift 6

#define cp0_wired_w \
	((pr->cp0[cp0_Wired] & \
	cp0_wired_w_mask) >> cp0_wired_w_shift)
#define cp0_wired_res1 \
	((pr->cp0[cp0_Wired] & \
	cp0_wired_res1_mask) >> cp0_wired_res1_shift)

#define cp0_context_res1_mask    0x0000000fU
#define cp0_context_badvpn2_mask 0x007ffff0U
#define cp0_context_ptebase_mask 0xff800000U

#define cp0_context_res1_shift    0
#define cp0_context_badvpn2_shift 4
#define cp0_context_ptebase_shift 23
#define cp0_context_addr_shift    9

#define cp0_context_res1 \
	((pr->cp0[cp0_Context] \
	& cp0_context_res1_mask) >> cp0_context_res1_shift)
#define cp0_context_badvpn2 \
	((pr->cp0[cp0_Context] \
	& cp0_context_badvpn2_mask) >> cp0_context_badvpn2_shift)
#define cp0_context_ptebase \
	((pr->cp0[cp0_Context] \
	& cp0_context_ptebase_mask) >> cp0_context_ptebase_shift)
	
#define cp0_pagemask_res1_mask 0x00001fffU
#define cp0_pagemask_mask_mask 0x01ffe000U
#define cp0_pagemask_res2_mask 0xfe000000U

#define cp0_pagemask_res1_shift 0
#define cp0_pagemask_mask_shift 13
#define cp0_pagemask_res2_shift 25

#define cp0_pagemask_res1 \
	((pr->cp0[cp0_PageMask] \
	& cp0_pagemask_res1_mask) >> cp0_pagemask_res1_shift)
#define cp0_pagemask_mask \
	((pr->cp0[cp0_PageMask] \
	& cp0_pagemask_mask_mask) >> cp0_pagemask_mask_shift)
#define cp0_pagemask_res2 \
	((pr->cp0[cp0_PageMask] \
	& cp0_pagemask_res2_mask) >> cp0_pagemask_res2_shift)

#define cp0_badvaddr_badvaddr cp0_badvaddr
#define cp0_count_count       cp0_count
#define cp0_compare_compare   cp0_compare
#define cp0_epc_epc           cp0_epc

#define cp0_cause_res1_mask    0x00000003U
#define cp0_cause_exccode_mask 0x0000007cU
#define cp0_cause_res2_mask    0x00000080U
#define cp0_cause_ip0_mask     0x00000100U
#define cp0_cause_ip1_mask     0x00000200U
#define cp0_cause_ip_mask      0x0000ff00U
#define cp0_cause_res3_mask    0x0fff0000U
#define cp0_cause_ce_mask      0x30000000U
#define cp0_cause_bd_mask      0x80000000U
#define cp0_cause_res4_mask    0x40000000U

#define cp0_cause_ce_cu1 0x10000000U
#define cp0_cause_ce_cu2 0x20000000U
#define cp0_cause_ce_cu3 0x30000000U

#define cp0_cause_res1_shift    0
#define cp0_cause_exccode_shift 2
#define cp0_cause_res2_shift    7
#define cp0_cause_ip_shift      8
#define cp0_cause_ip0_shift     8
#define cp0_cause_ip1_shift     9
#define cp0_cause_ip2_shift     10
#define cp0_cause_ip3_shift     11
#define cp0_cause_ip4_shift     12
#define cp0_cause_ip5_shift     13
#define cp0_cause_ip6_shift     14
#define cp0_cause_ip7_shift     15
#define cp0_cause_res3_shift    16
#define cp0_cause_ce_shift      28
#define cp0_cause_res4_shift    30
#define cp0_cause_bd_shift      31

#define cp0_cause_res1 \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_res1_mask) >> cp0_cause_res1_shift)
#define cp0_cause_exccode \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_exccode_mask) >> cp0_cause_exccode_shift)
#define cp0_cause_res2 \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_res2_mask) >> cp0_cause_res2_shift)
#define cp0_cause_ip \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_ip_mask) >> cp0_cause_ip_shift)
#define cp0_cause_res3 \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_res3_mask) >> cp0_cause_res3_shift)
#define cp0_cause_ce \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_ce_mask) >> cp0_cause_ce_shift)
#define cp0_cause_res4 \
	((pr->cp0[cp0_Cause] & cp0_cause_res4_mask) >> cp0_cause_res4_shift)
#define cp0_cause_bd \
	((pr->cp0[cp0_Cause] \
	& cp0_cause_bd_mask) >> cp0_cause_bd_shift)

#define cp0_prid_rev_mask 0x000000ffU
#define cp0_prid_imp_mask 0x0000ff00U
#define cp0_prid_res_mask 0xffff0000U

#define cp0_prid_rev_shift 0
#define cp0_prid_imp_shift 8
#define cp0_prid_res_shift 16

#define cp0_prid_rev \
	((pr->cp0[cp0_PRId] \
	& cp0_prid_rev_mask) >> cp0_prid_rev_shift)
#define cp0_prid_imp \
	((pr->cp0[cp0_PRId] \
	& cp0_prid_imp_mask) >> cp0_prid_imp_shift)
#define cp0_prid_res \
	((pr->cp0[cp0_PRId] \
	& cp0_prid_res_mask) >> cp0_prid_res_shift)

#define cp0_config_k0_mask  0x00000007U
#define cp0_config_cu_mask  0x00000008U
#define cp0_config_db_mask  0x00000010U
#define cp0_config_b_mask   0x00000020U
#define cp0_config_dc_mask  0x000003c0U
#define cp0_config_ic_mask  0x00003c00U
#define cp0_config_res_mask 0x00001000U
#define cp0_config_eb_mask  0x00002000U
#define cp0_config_em_mask  0x00004000U
#define cp0_config_be_mask  0x00008000U
#define cp0_config_sm_mask  0x00010000U
#define cp0_config_sc_mask  0x00020000U
#define cp0_config_ew_mask  0x000c0000U
#define cp0_config_sw_mask  0x00100000U
#define cp0_config_ss_mask  0x00200000U
#define cp0_config_sb_mask  0x00c00000U
#define cp0_config_ep_mask  0x0f000000U
#define cp0_config_ec_mask  0x70000000U
#define cp0_config_cm_mask  0x80000000U

#define cp0_config_k0_shift  0
#define cp0_config_cu_shift  3
#define cp0_config_db_shift  4
#define cp0_config_b_shift   5
#define cp0_config_dc_shift  6
#define cp0_config_ic_shift  9
#define cp0_config_res_shift 12
#define cp0_config_eb_shift  13
#define cp0_config_em_shift  14
#define cp0_config_be_shift  15
#define cp0_config_sm_shift  16
#define cp0_config_sc_shift  17
#define cp0_config_ew_shift  18
#define cp0_config_sw_shift  20
#define cp0_config_ss_shift  21
#define cp0_config_sb_shift  22
#define cp0_config_ep_shift  24
#define cp0_config_ec_shift  28
#define cp0_config_cm_shift  31

#define cp0_config_k0 \
	((pr->cp0[cp0_Config] \
	& cp0_config_k0_mask) >> cp0_config_k0_shift)
#define cp0_config_cu \
	((pr->cp0[cp0_Config] \
	& cp0_config_cu_mask) >> cp0_config_cu_shift)
#define cp0_config_db \
	((pr->cp0[cp0_Config] \
	& cp0_config_db_mask) >> cp0_config_db_shift)
#define cp0_config_b \
	((pr->cp0[cp0_Config] \
	& cp0_config_b_mask) >> cp0_config_b_shift)
#define cp0_config_dc \
	((pr->cp0[cp0_Config] \
	& cp0_config_dc_mask) >> cp0_config_dc_shift)
#define cp0_config_ic \
	((pr->cp0[cp0_Config] \
	& cp0_config_ic_mask) >> cp0_config_ic_shift)
#define cp0_config_res \
	((pr->cp0[cp0_Config] \
	& cp0_config_res_mask) >>cp0_config_res_shift)
#define cp0_config_eb \
	((pr->cp0[cp0_Config] \
	& cp0_config_eb_mask) >> cp0_config_eb_shift)
#define cp0_config_em \
	((pr->cp0[cp0_Config] \
	& cp0_config_em_mask) >> cp0_config_em_shift)
#define cp0_config_be \
	((pr->cp0[cp0_Config] \
	& cp0_config_be_mask) >> cp0_config_be_shift)
#define cp0_config_sm \
	((pr->cp0[cp0_Config] \
	& cp0_config_sm_mask) >> cp0_config_sm_shift)
#define cp0_config_sc \
	((pr->cp0[cp0_Config] \
	& cp0_config_sc_mask) >> cp0_config_sc_shift)
#define cp0_config_ew \
	((pr->cp0[cp0_Config] \
	& cp0_config_ew_mask) >> cp0_config_ew_shift)
#define cp0_config_sw \
	((pr->cp0[cp0_Config] \
	& cp0_config_sw_mask) >> cp0_config_sw_shift)
#define cp0_config_ss \
	((pr->cp0[cp0_Config] \
	& cp0_config_ss_mask) >> cp0_config_ss_shift)
#define cp0_config_sb \
	((pr->cp0[cp0_Config] \
	& cp0_config_sb_mask) >> cp0_config_sb_shift)
#define cp0_config_ep \
	((pr->cp0[cp0_Config] \
	& cp0_config_ep_mask) >> cp0_config_ep_shift)
#define cp0_config_ec \
	((pr->cp0[cp0_Config] \
	& cp0_config_ec_mask) >> cp0_config_ec_shift)
#define cp0_config_cm \
	((pr->cp0[cp0_Config] \
	& cp0_config_cm_mask) >> cp0_config_cm_shift)

#define cp0_lladdr_lladdr cp0_lladdr

#define cp0_watchlo_w_mask      0x00000001U
#define cp0_watchlo_r_mask      0x00000002U
#define cp0_watchlo_res_mask    0x00000004U
#define cp0_watchlo_paddr0_mask 0xfffffff8U

#define cp0_watchlo_w_shift      0
#define cp0_watchlo_r_shift      1
#define cp0_watchlo_res_shift    2
#define cp0_watchlo_paddr0_shift 3

#define cp0_watchlo_w \
	((pr->cp0[cp0_WatchLo] \
	& cp0_watchlo_w_mask) >> cp0_watchlo_w_shift)
#define cp0_watchlo_r \
	((pr->cp0[cp0_WatchLo] \
	& cp0_watchlo_r_mask) >> cp0_watchlo_r_shift)
#define cp0_watchlo_res \
	((pr->cp0[cp0_WatchLo] \
	& cp0_watchlo_res_mask) >> cp0_watchlo_res_shift)
#define cp0_watchlo_paddr0 \
	((pr->cp0[cp0_WatchLo] \
	& cp0_watchlo_paddr0_mask) >> cp0_watchlo_paddr0_shift)

#define cp0_watchhi_paddr1_mask 0x0000000fU
#define cp0_watchhi_res_mask    0xfffffff0U

#define cp0_watchhi_paddr1_shift 0
#define cp0_watchhi_res_shift    4

#define cp0_watchhi_paddr1 \
	((pr->cp0[cp0_WatchHi] \
	& cp0_watchhi_paddr1_mask) >> cp0_watchhi_paddr1_shift)
#define cp0_watchhi_res	\
	((pr->cp0[cp0_WatchHi] \
	& cp0_watchhi_res_mask) >> cp0_watchhi_res_shift)

#define cp0_ecc_ecc_mask 0x000000ffU
#define cp0_ecc_res_mask 0xffffff00U

#define cp0_ecc_ecc_shift 0
#define cp0_ecc_res_shift 8

#define cp0_ecc_ecc \
	((pr->cp0[cp0_ECC] \
	& cp0_ecc_ecc_mask) >> cp0_ecc_ecc_shift)
#define cp0_ecc_res \
	((pr->cp0[cp0_ECC] \
	& cp0_ecc_res_mask) >> cp0_ecc_res_shift)

#define cp0_xcontext_res1_mask    0x0000000fU
#define cd0_xcontext_badvpn2_mask 0x00000000U
#define cp0_xcontext_r_mask       0x00000000U
#define cp0_xcontext_ptebase_mask 0x00000000U

#define cp0_xcontext_res1_shift    0
#define cp0_xcontext_badvpn2_shift 4
#define cp0_xcontext_r_shift       31
#define cp0_xcontext_ptebase_shift 33

#define cp0_xcontext_res1 \
	((pr->cp0[cp0_XContext] \
	& cp0_xcontext_res1_mask) >> cp0_xcontext_res1_shift)
#define cp0_xcontext_badvpn2 \
	((pr->cp0[cp0_XContext] \
	& cp0_xcontext_badvpn2_mask) >> cp0_xcontext_badvpn2_shift)
#define cp0_xcontext_r \
	((pr->cp0[cp0_XContext] \
	& cp0_xcontext_r_mask) >> cp0_xcontext_r_shift)
#define cp0_xcontext_ptebase \
	((pr->cp0[cp0_XContext] \
	& cp0_xcontext_ptebase_mask) >> cp0_xcontext_ptebase_shift)

#define cp0_errorepc (pr->cp0[cp0_ErrorEPC])

#define cp0_index    (pr->cp0[cp0_Index])
#define cp0_random   (pr->cp0[cp0_Random])
#define cp0_entrylo0 (pr->cp0[cp0_EntryLo0])
#define cp0_entrylo1 (pr->cp0[cp0_EntryLo1])
#define cp0_context  (pr->cp0[cp0_Context])
#define cp0_pagemask (pr->cp0[cp0_PageMask])
#define cp0_wired    (pr->cp0[cp0_Wired])
#define cp0_badvaddr (pr->cp0[cp0_BadVAddr])
#define cp0_count    (pr->cp0[cp0_Count])
#define cp0_entryhi  (pr->cp0[cp0_EntryHi])
#define cp0_compare  (pr->cp0[cp0_Compare])
#define cp0_cause    (pr->cp0[cp0_Cause])
#define cp0_config   (pr->cp0[cp0_Config])
#define cp0_prid     (pr->cp0[cp0_PRId])
#define cp0_status   (pr->cp0[cp0_Status])
#define cp0_epc      (pr->cp0[cp0_EPC])
#define cp0_lladdr   (pr->cp0[cp0_LLAddr])
#define cp0_watchlo  (pr->cp0[cp0_WatchLo])
#define cp0_watchhi  (pr->cp0[cp0_WatchHi])
#define cp0_ecc      (pr->cp0[cp0_ECC])
#define cp0_taglo    (pr->cp0[cp0_TagLo])
#define cp0_taghi    (pr->cp0[cp0_TagHi])

/** cp0 Masks */
#define cp0_SR_EXLMask 0x00000002U

/** TLB entity definition */
typedef struct {
	uint32_t pfn;     /* physical page no (shifted << 12) */
	uint8_t cohh;     /* coherency number */
	bool dirty;       /* dirty */
	bool valid;       /* valid */
} tlb_ent_v;

typedef struct tlb_ent {
	uint32_t mask;    /* enhanced mask */
	uint32_t vpn2;    /* vitrual page no (shifted << 7) */
	bool global;      /* global bit */
	uint8_t asid;     /* address id */
	tlb_ent_v pg[2];  /* sub - pages */
	
	struct tlb_ent *next;
} tlb_ent;

/** Main processor structure */
typedef struct {
	unsigned int procno;
	
	bool stdby;
	
	/* Standard registers */
	uint32_t regs[32];
	uint32_t cp0[32];
	uint64_t fpregs[32];
	uint32_t loreg;
	uint32_t hireg;
	
	/* Program counter */
	ptr_t pc;
	ptr_t pc_next;
	
	/* TLB structures */
	tlb_ent tlb[48];
	tlb_ent *tlblist; /* for faster access */
	
	/* Old registers (for debug info) */
	uint32_t old_regs[32];
	uint32_t old_cp0[32];
	uint32_t old_loreg;
	uint32_t old_hireg;
	
	ptr_t excaddr;
	unsigned int branch;
	
	/* LL and SC track support */
	bool llbit;    /* Track the address flag */
	ptr_t lladdr;  /* Physical tracked address */
	
	/* Watch support */
	uint64_t waddr;
	ptr_t wexcaddr;
	bool wpending;
	
	/* Statistics */
	uint64_t k_cycles;
	uint64_t u_cycles;
	uint64_t w_cycles;
	
	uint64_t tlb_refill;
	uint64_t tlb_invalid;
	uint64_t tlb_modified;
	uint64_t intr[8];
	
	/* breakpoints */
	list_t bps;
} processor_t;

/** Base */
extern void processor_init(processor_t *pr, unsigned int procno);
extern void step(processor_t *pr);

/** First settings */
extern void set_general_reg(processor_t *pr, unsigned int regno, int32_t value);
extern void set_pc(processor_t *pr, ptr_t addr);
extern void update_deb(processor_t *pr);

/** Addresing function */
extern exc_t convert_addr(processor_t *pr, ptr_t *addr, bool write,
    bool fill_error_regs);

/** Reading memory */
extern exc_t read_proc_mem(processor_t *pr, ptr_t addr, len_t size,
    uint32_t *value, bool h);
extern exc_t read_proc_ins(processor_t *pr, ptr_t addr, uint32_t *value,
    bool h);

/** Interrupts - cause */
extern void proc_interrupt_up(processor_t *pr, unsigned int no);
extern void proc_interrupt_down(processor_t *pr, unsigned int no);

#endif
