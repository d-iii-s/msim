/*
 * Copyright (c) 2002-2007 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Useful debugging features
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "debug.h"
#include "../main.h"
#include "../mtypes.h"
#include "../cpu/instr.h"
#include "../cpu/processor.h"
#include "../device/machine.h"
#include "../io/output.h"
#include "../env.h"

#include "../device/dcpu.h"
#include "../device/mem.h"


#define CP0_PM_ITEMS 7
#define REG_BUF      1024


static struct {
	uint32_t no;
	char *s;
} pagemask_name[CP0_PM_ITEMS + 1] = {
	{ 0x0U,   "4k" },
	{ 0x3U,   "16k" },
	{ 0xfU,   "64k" },
	{ 0x3fU,  "256k" },
	{ 0xffU,  "1M" },
	{ 0x3ffU, "4M" },
	{ 0xfffU, "16M" },
	{ -1,     "err" }
};


static char *cp0_dump_str[] = {
	"  00 Index\t%08X  index: %02X res: %x p: %01x \n",
	"  01 Random\t%08X  random: %02X, res: %07X\n",
	"  02 EntryLo0\t%08X  g: %x v: %x d: %x c: %x pfn: %06X res: %x\n",
	"  03 EntryLo1\t%08X  g: %x v: %x d: %x c: %x pfn: %06X res: %x\n",
	"  04 Context\t%08X  res: %x badvpn2: %05X ptebase: %03X\n",
	"  05 PageMask\t%08X  res1: %04x mask: %03X (%s) res2: %02X\n",
	"  06 Wired\t%08X  wired: %x res: %07X\n",
	"  07 Reserved\n",
	"  08 BadVAddr\t%08X  badvaddr: %08X\n",
	"  09 Count\t%08X  count: %x\n",
	"  0a EntryHi\t%08X  asid: %02X res: %x vpn2: %05X\n",
	"  0b Compare\t%08X  compare: %x\n",
	"  0c Status\t%08X  ie: %x exl: %x erl: %x ksu: %x "
		"ux: %x sx: %x kx: %x\n\t\t\t  im: %02X de: %x "
		"ce: %x ch: %x res1: %x sr: %x ts: %x\n\t\t\t  "
		"bev: %x res2: %x re: %x fr: %x rp: %x cu: %x\n",
	"  0d Cause\t%08X  res1: %x exccode: %02X res2: %x "
		"ip: %02X res3: %02X\n\t\t\t  ce: %d res4: %d bd: %d\n",
	"  0e EPC\t%08X  epc: %08X\n",
	"  0f PRId\t%08X  rev: %02X imp: %02X res: %04X\n",
	"  10 Config\t%08X  k0: %x cu: %x db: %x b: %x dc: %x "
		"ic: %x res: %x eb: %x\n\t\t\t  em: %x be: %x sm: %x sc: %x "
		"ew: %x sw: %x ss: %x sb: %x\n\t\t\t  ep: %x ec: %x cm: %x\n",
	"  11 LLAddr\t%08X  lladdr: %08X\n",
	"  12 WatchLo\t%08X  w: %x r: %x res: %x paddr0: %08X\n",
	"  13 WatchHi\t%08X  res: %08X paddr1: %x\n",
	"  14 XContext\n",
	"  15 Reserved\n",
	"  16 Reserved\n",
	"  17 Reserved\n",
	"  18 Reserved\n",
	"  19 Reserved\n",
	"  1a Reserved\n",
	"  1b Reserved\n",
	"  1c Reserved\n",
	"  1d Reserved\n",
	"  1e ErrorEPC\t%08x  errorepc: %08x\n",
	"  1f Reserved\n"
};


void reg_view(processor_t *pr)
{
	unsigned int i;
	
	mprintf("processor p%i\n", pr->procno);
	
	for (i = 0; i < 30; i += 5) {
		mprintf(" %3s %08X  %3s %08X  %3s %08X  %3s %08X  %3s %08X\n", 
			regname[i],     pr->regs[i], 
			regname[i + 1], pr->regs[i + 1], 
			regname[i + 2], pr->regs[i + 2],
			regname[i + 3], pr->regs[i + 3], 
			regname[i + 4], pr->regs[i + 4]);
	}
	 
	mprintf(" %3s %08X  %3s %08X   pc %08X   lo %08X   hi %08X\n",
		regname[i],     pr->regs[i], 
		regname[i + 1], pr->regs[i + 1],
		pr->pc, pr->loreg, pr->hireg);
}


static const char *get_pagemask_name(unsigned int pm)
{
	unsigned int i;
	for (i = 0; i < CP0_PM_ITEMS; i++)
		if (pm == pagemask_name[i].no)
			return pagemask_name[i].s;
	
	/* Error */
	return pagemask_name[CP0_PM_ITEMS].s;
}


void tlb_dump(processor_t *pr)
{
	unsigned int i;
	struct tlb_ent *e;
	
	mprintf( " [             general             ][    subp 0    ][    subp 1    ]\n"
		"  no    vpn      mask        g asid  v d   pfn    c  v d   pfn    c\n");
	
	for (i = 0; i < 48; i++) {
		e = &(pr->tlb[i]);
		
		mprintf( "  %02x  %08X %08X:%-4s %d  %02x   %d %d %08X %x  %d %d %08X %1x\n",
		    i, e->vpn2, e->mask,
		    get_pagemask_name((~e->mask) >> cp0_pagemask_mask_shift),
		    e->global, e->asid, e->pg[0].valid, e->pg[0].dirty,
		    e->pg[0].pfn, e->pg[0].cohh, e->pg[1].valid,
		    e->pg[1].dirty, e->pg[1].pfn, e->pg[1].cohh);
	}
}

static void cp0_dump_reg(processor_t *pr, unsigned int reg)
{
	const char *s = cp0_dump_str[reg];
	
	switch (reg) {
	case cp0_Index:
		mprintf(s,
			cp0_index,
			cp0_index_index, cp0_index_res, cp0_index_p);
		break;
	case cp0_Random:
		mprintf(s,
			cp0_random, cp0_random_random, cp0_random_res);
		break;
	case cp0_EntryLo0:
		mprintf(s,
			cp0_entrylo0, 
			cp0_entrylo0_g,	cp0_entrylo0_v,
			cp0_entrylo0_d,	cp0_entrylo0_c,
			cp0_entrylo0_pfn, cp0_entrylo0_res1);
		break;
	case cp0_EntryLo1:
		mprintf(s,
			cp0_entrylo1, 
			cp0_entrylo1_g,	cp0_entrylo1_v,
			cp0_entrylo1_d,	cp0_entrylo1_c,
			cp0_entrylo1_pfn, cp0_entrylo1_res1);
		break;
	case cp0_Context:
		mprintf(s,
			cp0_context,
			cp0_context_res1,
			cp0_context_badvpn2,
			cp0_context_ptebase);
		break;
	case cp0_PageMask:
		mprintf(s,
			cp0_pagemask,
			cp0_pagemask_res1,
			cp0_pagemask_mask,
			get_pagemask_name( cp0_pagemask_mask),
			cp0_pagemask_res2);
		break;
	case cp0_Wired:
		mprintf(s,
			cp0_wired, cp0_wired_w, cp0_wired_res1);
		break;
	case cp0_BadVAddr:
		mprintf(s, cp0_badvaddr, cp0_badvaddr_badvaddr);
		break;
	case cp0_Count:
		mprintf(s,
			cp0_count, cp0_count_count);
		break;
	case cp0_EntryHi:
		mprintf(s,
			cp0_entryhi, cp0_entryhi_asid, cp0_entryhi_res1, cp0_entryhi_vpn2);
		break;
	case cp0_Compare:
		mprintf(s,
			cp0_compare, cp0_compare_compare);
		break;
	case cp0_Status:
		mprintf(s,
			cp0_status,
			cp0_status_ie, cp0_status_exl, cp0_status_erl,
			cp0_status_ksu,	cp0_status_ux, cp0_status_sx,
			cp0_status_kx, cp0_status_im, cp0_status_de,
			cp0_status_ce, cp0_status_ch, cp0_status_res1,
			cp0_status_sr, cp0_status_ts, cp0_status_bev,
			cp0_status_res2, cp0_status_re, cp0_status_fr,
			cp0_status_rp, cp0_status_cu);
		break;
	case cp0_Cause:
		mprintf(s,
			cp0_cause, cp0_cause_res1, cp0_cause_exccode,
			cp0_cause_res2, cp0_cause_ip, cp0_cause_res3,
			cp0_cause_ce, cp0_cause_res4, cp0_cause_bd);
		break;
	case cp0_EPC:
		mprintf(s,
			cp0_epc, cp0_epc_epc);
		break;
	case cp0_PRId:
		mprintf(s,
			cp0_prid, cp0_prid_rev, cp0_prid_imp, cp0_prid_res);
		break;
	case cp0_Config:
		mprintf(s,
			cp0_config, cp0_config_k0, cp0_config_cu,
			cp0_config_db, cp0_config_b, cp0_config_dc,
			cp0_config_ic, cp0_config_res, cp0_config_eb,
			cp0_config_em, cp0_config_be, cp0_config_sm,
			cp0_config_sc, cp0_config_ew, cp0_config_sw,
			cp0_config_ss, cp0_config_sb, cp0_config_ep,
			cp0_config_ec, cp0_config_cm);
		break;
	case cp0_LLAddr:
		mprintf(s,
			cp0_lladdr, cp0_lladdr_lladdr);
		break;
	case cp0_WatchLo:
		mprintf(s,
			cp0_watchlo, cp0_watchlo_w, cp0_watchlo_r,
			cp0_watchlo_res, cp0_watchlo_paddr0);
		break;
	case cp0_WatchHi:
		mprintf(s,
			cp0_watchhi, cp0_watchhi_paddr1, cp0_watchhi_res);
		break;
	case cp0_ErrorEPC:
		mprintf(s,
			cp0_errorepc, cp0_errorepc);
		break;
	default:
		mprintf(s);
		break;
	}
}


void cp0_dump(processor_t *pr, int reg)
{
	const int *i;
	const int vals[] =
		{0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 30, -1};
	
	mprintf("  no name       hex dump  readable dump\n");
	if (reg == -1)
		for (i = &vals[0]; *i != -1; i++)
			cp0_dump_reg(pr, *i);
	else
		cp0_dump_reg(pr, reg);
}


/** Convert an opcode to text
 *
 * @param pr If not NULL then the dump is
 *           processor-dependent (with processor number)
 *
 */
void iview(processor_t *pr, uint32_t addr, instr_info_t *ii, char *regch)
{
	char s_proc[16];
	char s_iopc[16];
	char s_addr[16];
	char s_parm[32];
	char *s_hash;
	char s_cmt[32];
	char *s_cmtx;
	
	const int imm = ii->imm;
	const char *rtn = regname[ii->rt];
	const char *rsn = regname[ii->rs];
	const char *rdn = regname[ii->rd];
	
	if (pr != NULL)
		sprintf((char *) s_proc, "%2d  ", pr->procno);
	else
		s_proc[0] = '\0';
	
	if (iaddr)
		sprintf((char *) s_addr, "%08X  ", addr);
	else
		s_addr[0] = '\0';
	
	if (iopc)
		sprintf((char *) s_iopc, "%08X  ", ii->icode);
	else
		s_iopc[0] = '\0';
	
	s_parm[0] = '\0';
	s_cmt[0] = '\0';
	
	switch (instr_names_acronym[ii->opcode].itype) {
	case ifNONE:
		break;
	case ifR4:
		sprintf(s_parm, "...");
		sprintf(s_cmt, "not implemented");
		break;
	case ifREG:
		sprintf(s_parm, "%s, %s, %s", rdn, rsn, rtn);
		break;
	case ifIMM:
		sprintf(s_parm, "%s, %s, 0x%x", rtn, rsn, imm);
		break;
	case ifIMMS:
		sprintf(s_parm, "%s, %s, 0x%x", rtn, rsn, imm);
		if (imm > 9)
			sprintf(s_cmt, "0x%x=%d", imm, imm);
		break;
	case ifIMMU:
		sprintf(s_parm, "%s, %s, 0x%x", rtn, rsn, (unsigned int) (imm & 0xffff));
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%u", imm,	(unsigned int) (imm & 0xffff));
		break;
	case ifIMMUX:
		sprintf(s_parm, "%s, %s, 0x%04x", rtn, rsn,	(unsigned int) (imm & 0xffff));
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%xh=%u", imm, (unsigned int) (imm & 0xffff));
		break;
	case ifOFF:
		sprintf(s_parm, "0x%x", imm);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%u", imm, imm);
		break;
	case ifCND:
		if (imm > 0)
			sprintf(s_parm, "%s, %s, 0x%x", rsn, rtn, imm);
		else
			sprintf(s_parm, "%s, %s, -0x%x", rsn, rtn, -imm);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%u", imm, imm);
		break;
	case ifDTS:
		sprintf(s_parm, "%s, %s, 0x%02x", rdn, rtn, ii->shift);
		if (ii->shift > 9)
			sprintf((char *) s_cmt, "0x%x=%u", ii->shift, ii->shift);
		break;
	case ifRO:
		if (imm > 0)
			sprintf(s_parm, "%s, 0x%x", rsn, imm);
		else
			sprintf(s_parm, "%s, -0x%x", rsn, -imm);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%u", imm, imm);
		break;
	case ifTD:
		sprintf(s_parm, "%s, %s", rtn, rdn);
		break;
	case ifTDX0:
		sprintf(s_parm, "%s, %s", rtn, cp0name[ii->rd]);
		break;
	case ifTDX1:
		sprintf(s_parm, "%s, %s", rtn, cp1name[ii->rd]);
		break;
	case ifTDX2:
		sprintf(s_parm, "%s, %s", rtn, cp2name[ii->rd]);
		break;
	case ifTDX3:
		sprintf(s_parm, "%s, %s", rtn, cp3name[ii->rd]);
		break;
	case ifOP:
		sprintf(s_parm, "0x%x", ii->icode & 0x01ffffff);
		break;
	case ifST:
		sprintf(s_parm, "%s, %s", rsn, rtn);
		break;
	case ifJ:
		sprintf(s_parm, "+0x%x", ii->imm);
		if (ii->imm > 9)
			sprintf((char *) s_cmt, "0x%x=%d", ii->imm, ii->imm);
		break;
	case ifDS:
		sprintf(s_parm, "%s, %s", rdn, rsn);
		break;
	case ifS:
		sprintf(s_parm, "%s", rsn);
		break;
	case ifTOB:
		if (imm)
			sprintf(s_parm, "%s, 0x%x(%s)", rtn, imm, rsn);
		else
			sprintf(s_parm, "%s, (%s)", rtn, rsn);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%d", imm, imm);
		break;
	case ifRIW:
		sprintf(s_parm, "%s, 0x%04x", rtn, imm & 0xffff);
		if ((imm & 0xffff) > 9)
			sprintf((char *) s_cmt, "0x%x=%d", imm & 0xffff, imm & 0xffff);
		break;
	case ifD:
		sprintf(s_parm, "%s", rdn);
		break;
	case ifSI:
		sprintf(s_parm, "%s, 0x%x", rsn, imm);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%d", imm, imm);
		break;
	case ifSIW:
		sprintf(s_parm, "%s, 0x%x [%u]", rsn, imm, (unsigned int) imm);
		if (imm > 9)
			sprintf((char *) s_cmt, "0x%x=%u", imm, (unsigned int) imm);
			break;
	case ifSYSCALL:
		sprintf(s_parm, "0x%x", (ii->icode >> 6) & 0xfffff);
		if ((ii->icode >> 6) > 9)
			sprintf((char *) s_cmt, "0x%x=%d",
				(ii->icode >> 6) & 0xfffff, (ii->icode >> 6) & 0xfffff);
		break;
	case ifX:
	case ifERR:
		/* Internal only */
		break;
	}
	
	if (!icmt)
		s_cmt[0] = 0;
	
	if (!regch)
		regch = "";
	
	s_hash = "";
	if ((s_cmt[0]) || (regch[0]))
		s_hash = "#";
	
	s_cmtx = "";
	if ((s_cmt[0]) && (regch[0]))
		s_cmtx = ", ";
	
	mprintf("%-4s%s%s  %-6s%-18s%-2s%s%s%s\n",
		s_proc, s_addr, s_iopc, 
		instr_names_acronym[ii->opcode].instr_text_t, s_parm,
		s_hash, s_cmt, s_cmtx, regch);
}


/** Write info about changed registers
 *
 * Each modified register is included to the output.
 *
 */
void modified_regs_dump(processor_t *pr, size_t size, char *sx)
{
	unsigned int i;
	char *s1;
	char *s2;
	char *s3;
	char sc1[REG_BUF];
	char sc2[REG_BUF]; 
	
	sc1[0] = 0;
	sc2[0] = 0;
	s1 = sc1;
	s2 = sc2;
	
	if (size > REG_BUF)
		size = REG_BUF;
	
	/* Test for general registers */
	for (i = 0; i < 32; i++)
		if (pr->regs[i] != pr->old_regs[i]) {
			snprintf(s1, size, "%s, %s: 0x%x->0x%x", s2, regname[i],
				pr->old_regs[i], pr->regs[i]);
			
			s3 = s1;
			s1 = s2;
			s2 = s3;
			pr->old_regs[i] = pr->regs[i];
		}
		
	/* Test for cp0 */
	for (i = 0; i < 32; i++)
		if ((pr->cp0[i] != pr->old_cp0[i]) && (i != cp0_Random) && (i != cp0_Count)) {
			if (cp0name == cp0_name[2])
				snprintf(s1, size, "%s, cp0_%s: 0x%08x->0x%08x", s2,
					cp0name[i], pr->old_cp0[i], pr->cp0[i]);
			else
				snprintf(s1, size, "%s, cp0[%d]: 0x%08x->0x%08x", s2,
					i, pr->old_cp0[i], pr->cp0[i]);
			
			s3 = s1;
			s1 = s2;
			s2 = s3;
			pr->old_cp0[i] = pr->cp0[i];
		}
	
	/* Test for loreg */
	if (pr->loreg != pr->old_loreg) {
		snprintf(s1, size, "%s, loreg: 0x%x->0x%x",
			s2, pr->old_loreg, pr->loreg);
			
		s3 = s1;
		s1 = s2;
		s2 = s3;
		pr->old_loreg = pr->loreg;
	}
	
	/* Test for hireg */
	if (pr->hireg != pr->old_hireg) {
		snprintf(s1, size, "%s, hireg: 0x%x->0x%x",
			s2, pr->old_hireg, pr->hireg);
		
		s3 = s1;
		s1 = s2;
		s2 = s3;
		pr->old_hireg = pr->hireg;
	}
	
	if (*s2 == 0)
		*sx = 0;
	else
		strcpy(sx, s2 + 2);
}


void dbg_print_device_info(device_s *dev)
{
	mprintf("%-10s %-10s ", dev->name, dev->type->name);
	cmd_run_by_name("info", &pars_end, dev->type->cmds, dev);
}

/** Show statistics for specified device
 *
 */
void dbg_print_device_statistics(device_s *dev)
{
	mprintf("%-10s %-10s ", dev->name, dev->type->name);
	cmd_run_by_name("stat", &pars_end, dev->type->cmds, dev);
}

void dbg_print_devices(const char* header, const char* nothing_msg,
    device_filter_t filter, void (print_function) (device_s*))
{
	device_s *device = NULL;
	bool device_found = false;
	
	mprintf(header);
	
	while (dev_next(&device, filter)) {
		device_found = true;
		print_function(device);
	}
	
	if (!device_found)
		mprintf(nothing_msg);
}
