#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "debug.h"
#include "cpu.h"
#include "../../../assert.h"
#include "../../../env.h"
#include "../../../main.h"
#include "../../../utils.h"


#define CP0_PM_ITEMS  7
#define REG_BUF       1024

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
	"  08 BadVAddr\t%08X\n",
	"  09 Count\t%08X\n",
	"  0a EntryHi\t%08X  asid: %02X res: %x vpn2: %05X\n",
	"  0b Compare\t%08X\n",
	"  0c Status\t%08X  ie: %x exl: %x erl: %x ksu: %x "
		"ux: %x sx: %x kx: %x\n\t\t\t  im: %02X de: %x "
		"ce: %x ch: %x res1: %x sr: %x ts: %x\n\t\t\t  "
		"bev: %x res2: %x re: %x fr: %x rp: %x cu: %x\n",
	"  0d Cause\t%08X  res1: %x exccode: %02X (%s) res2: %x "
		"ip: %02X res3: %02X\n\t\t\t  ce: %d res4: %d bd: %d\n",
	"  0e EPC\t%08X\n",
	"  0f PRId\t%08X  rev: %02X imp: %02X res: %04X\n",
	"  10 Config\t%08X  k0: %x cu: %x db: %x b: %x dc: %x "
		"ic: %x res: %x eb: %x\n\t\t\t  em: %x be: %x sm: %x sc: %x "
		"ew: %x sw: %x ss: %x sb: %x\n\t\t\t  ep: %x ec: %x cm: %x\n",
	"  11 LLAddr\t%08X\n",
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


static char *cp0_cause_exccode_str[] = {
	"Interrupt", "TLB Modification", "TLB Exception (Load)", "TLB Exception (Store)",
	"Address Error (Load)", "Address Error (Store)", "Bus Error (Code)", "Bus Error (Data)",
	"System Call", "Breakpoint", "Reserved Instruction", "Coprocessor Unusable",
	"Arithmetic Overflow", "Trap", "Virtual Coherency (Code)", "Floating Point",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved", "Reserved", "Watch",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved", "Reserved", "Virtual Coherency (Data)"
};

void r4k_reg_dump(r4k_cpu_t *cpu)
{
	printf("processor %u\n", cpu->procno);

	unsigned int i;
	for (i = 0; i < 30; i += 5) {
		printf(" %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "  %3s %16" PRIx64 "\n",
		    regname[i],     cpu->regs[i].val,
		    regname[i + 1], cpu->regs[i + 1].val,
		    regname[i + 2], cpu->regs[i + 2].val,
		    regname[i + 3], cpu->regs[i + 3].val,
		    regname[i + 4], cpu->regs[i + 4].val);
	}

	printf(" %3s %16" PRIx64 "  %3s %16" PRIx64 "   pc %16" PRIx64 "   lo %16" PRIx64 "   hi %16" PRIx64 "\n",
	    regname[i],     cpu->regs[i].val,
	    regname[i + 1], cpu->regs[i + 1].val,
	    cpu->pc.ptr, cpu->loreg.val, cpu->hireg.val);
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

void r4k_tlb_dump(r4k_cpu_t *cpu)
{
	printf(" [             general             ][    subp 0     ][     subp 1    ]\n"
	    "  no    vpn      mask        g asid  v d   pfn     c  v d   pfn     c\n");

	unsigned int i;
	for (i = 0; i < 48; i++) {
		tlb_entry_t *e = &(cpu->tlb[i]);

		printf("  %02x  %08" PRIx32 " %08" PRIx32 ":%-4s %u  %02x   %u %u %09" PRIx64 " %1x  %u %u %09" PRIx64 " %1x\n",
		    i, e->vpn2, e->mask,
		    get_pagemask_name((~e->mask) >> cp0_pagemask_mask_shift),
		    e->global, e->asid, e->pg[0].valid, e->pg[0].dirty,
		    e->pg[0].pfn, e->pg[0].cohh, e->pg[1].valid,
		    e->pg[1].dirty, e->pg[1].pfn, e->pg[1].cohh);
	}
}

static void r4k_cp0_dump_reg(r4k_cpu_t *cpu, unsigned int reg)
{
	const char *s = cp0_dump_str[reg];

	switch (reg) {
	case cp0_Index:
		printf(s,
		    cp0_index(cpu),
		    cp0_index_index(cpu), cp0_index_res(cpu), cp0_index_p(cpu));
		break;
	case cp0_Random:
		printf(s,
		    cp0_random(cpu), cp0_random_random(cpu), cp0_random_res(cpu));
		break;
	case cp0_EntryLo0:
		printf(s,
		    cp0_entrylo0(cpu),
		    cp0_entrylo0_g(cpu), cp0_entrylo0_v(cpu),
		    cp0_entrylo0_d(cpu), cp0_entrylo0_c(cpu),
		    cp0_entrylo0_pfn(cpu), cp0_entrylo0_res1(cpu));
		break;
	case cp0_EntryLo1:
		printf(s,
		    cp0_entrylo1(cpu),
		    cp0_entrylo1_g(cpu), cp0_entrylo1_v(cpu),
		    cp0_entrylo1_d(cpu), cp0_entrylo1_c(cpu),
		    cp0_entrylo1_pfn(cpu), cp0_entrylo1_res1(cpu));
		break;
	case cp0_Context:
		printf(s,
		    cp0_context(cpu),
		    cp0_context_res1(cpu),
		    cp0_context_badvpn2(cpu),
		    cp0_context_ptebase(cpu));
		break;
	case cp0_PageMask:
		printf(s,
		    cp0_pagemask(cpu),
		    cp0_pagemask_res1(cpu),
		    cp0_pagemask_mask(cpu),
		    get_pagemask_name(cp0_pagemask_mask(cpu)),
		    cp0_pagemask_res2(cpu));
		break;
	case cp0_Wired:
		printf(s,
		    cp0_wired(cpu), cp0_wired_w(cpu), cp0_wired_res1(cpu));
		break;
	case cp0_BadVAddr:
		printf(s, cp0_badvaddr(cpu));
		break;
	case cp0_Count:
		printf(s, cp0_count(cpu));
		break;
	case cp0_EntryHi:
		printf(s,
		    cp0_entryhi(cpu), cp0_entryhi_asid(cpu),
		    cp0_entryhi_res1(cpu), cp0_entryhi_vpn2(cpu));
		break;
	case cp0_Compare:
		printf(s, cp0_compare(cpu));
		break;
	case cp0_Status:
		printf(s,
		    cp0_status(cpu),
		    cp0_status_ie(cpu), cp0_status_exl(cpu), cp0_status_erl(cpu),
		    cp0_status_ksu(cpu), cp0_status_ux(cpu), cp0_status_sx(cpu),
		    cp0_status_kx(cpu), cp0_status_im(cpu), cp0_status_de(cpu),
		    cp0_status_ce(cpu), cp0_status_ch(cpu), cp0_status_res1(cpu),
		    cp0_status_sr(cpu), cp0_status_ts(cpu), cp0_status_bev(cpu),
		    cp0_status_res2(cpu), cp0_status_re(cpu), cp0_status_fr(cpu),
		    cp0_status_rp(cpu), cp0_status_cu(cpu));
		break;
	case cp0_Cause:
		printf(s,
		    cp0_cause(cpu), cp0_cause_res1(cpu),
		    cp0_cause_exccode(cpu), cp0_cause_exccode_str[cp0_cause_exccode(cpu)],
		    cp0_cause_res2(cpu), cp0_cause_ip(cpu), cp0_cause_res3(cpu),
		    cp0_cause_ce(cpu), cp0_cause_res4(cpu), cp0_cause_bd(cpu));
		break;
	case cp0_EPC:
		printf(s, cp0_epc(cpu));
		break;
	case cp0_PRId:
		printf(s,
		    cp0_prid(cpu), cp0_prid_rev(cpu),
		    cp0_prid_imp(cpu), cp0_prid_res(cpu));
		break;
	case cp0_Config:
		printf(s,
		    cp0_config(cpu), cp0_config_k0(cpu), cp0_config_cu(cpu),
		    cp0_config_db(cpu), cp0_config_b(cpu), cp0_config_dc(cpu),
		    cp0_config_ic(cpu), cp0_config_res(cpu), cp0_config_eb(cpu),
		    cp0_config_em(cpu), cp0_config_be(cpu), cp0_config_sm(cpu),
		    cp0_config_sc(cpu), cp0_config_ew(cpu), cp0_config_sw(cpu),
		    cp0_config_ss(cpu), cp0_config_sb(cpu), cp0_config_ep(cpu),
		    cp0_config_ec(cpu), cp0_config_cm(cpu));
		break;
	case cp0_LLAddr:
		printf(s, cp0_lladdr(cpu));
		break;
	case cp0_WatchLo:
		printf(s,
		    cp0_watchlo(cpu), cp0_watchlo_w(cpu), cp0_watchlo_r(cpu),
		    cp0_watchlo_res(cpu), cp0_watchlo_paddr0(cpu));
		break;
	case cp0_WatchHi:
		printf(s,
		    cp0_watchhi(cpu), cp0_watchhi_paddr1(cpu), cp0_watchhi_res(cpu));
		break;
	case cp0_ErrorEPC:
		printf(s, cp0_errorepc(cpu), cp0_errorepc(cpu));
		break;
	default:
		printf(s);
		break;
	}
}

void r4k_cp0_dump_all(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);

	printf("  no name       hex dump  readable dump\n");
	r4k_cp0_dump_reg(cpu, 0);
	r4k_cp0_dump_reg(cpu, 1);
	r4k_cp0_dump_reg(cpu, 2);
	r4k_cp0_dump_reg(cpu, 3);
	r4k_cp0_dump_reg(cpu, 4);
	r4k_cp0_dump_reg(cpu, 5);
	r4k_cp0_dump_reg(cpu, 6);
	r4k_cp0_dump_reg(cpu, 8);
	r4k_cp0_dump_reg(cpu, 9);
	r4k_cp0_dump_reg(cpu, 10);
	r4k_cp0_dump_reg(cpu, 11);
	r4k_cp0_dump_reg(cpu, 12);
	r4k_cp0_dump_reg(cpu, 13);
	r4k_cp0_dump_reg(cpu, 14);
	r4k_cp0_dump_reg(cpu, 15);
	r4k_cp0_dump_reg(cpu, 16);
	r4k_cp0_dump_reg(cpu, 17);
	r4k_cp0_dump_reg(cpu, 18);
	r4k_cp0_dump_reg(cpu, 19);
	r4k_cp0_dump_reg(cpu, 20);
	r4k_cp0_dump_reg(cpu, 30);
}

void r4k_cp0_dump(r4k_cpu_t *cpu, unsigned int reg)
{
	ASSERT(cpu != NULL);

	printf("  no name       hex dump  readable dump\n");
	r4k_cp0_dump_reg(cpu, reg);
}

static void idump_common(ptr64_t addr, r4k_instr_t instr, string_t *s_opc,
    string_t *s_mnemonics, string_t *s_comments)
{
	string_printf(s_opc, "%08" PRIx32, instr.val);

	mnemonics_fnc_t fnc = decode_mnemonics(instr);
	fnc(addr, instr, s_mnemonics, s_comments);
}

/** Dump instruction mnemonics
 *
 * @param cpu     If not NULL, then the dump is processor-dependent
 *                (with processor number).
 * @param addr    Virtual address of the instruction.
 * @param instr   Instruction to dump.
 * @param modregs If true, then modified registers are also dumped.
 *
 */
void r4k_idump(r4k_cpu_t *cpu, ptr64_t addr, r4k_instr_t instr, bool modregs)
{
	string_t s_cpu;
	string_t s_addr;
	string_t s_opc;
	string_t s_mnemonics;
	string_t s_comments;

	string_init(&s_cpu);
	string_init(&s_addr);
	string_init(&s_opc);
	string_init(&s_mnemonics);
	string_init(&s_comments);

	if (cpu != NULL)
		string_printf(&s_cpu, "cpu%u", cpu->procno);

	string_printf(&s_addr, "%#018" PRIx64, addr.ptr);
	idump_common(addr, instr, &s_opc, &s_mnemonics, &s_comments);

	if (cpu != NULL)
		printf("%-5s ", s_cpu.str);

	if (iaddr)
		printf("%-18s ", s_addr.str);

	if (iopc)
		printf("%-8s ", s_opc.str);

	printf("%s\n", s_mnemonics.str);

	// FIXME print comments

	string_done(&s_cpu);
	string_done(&s_addr);
	string_done(&s_opc);
	string_done(&s_mnemonics);
	string_done(&s_comments);
}

/** Dump instruction mnemonics
 *
 * @param addr  Physical address of the instruction.
 * @param instr Instruction to dump.
 *
 */
void r4k_idump_phys(ptr36_t addr, r4k_instr_t instr)
{
	string_t s_addr;
	string_t s_iopc;
	string_t s_mnemonics;
	string_t s_comments;

	string_init(&s_addr);
	string_init(&s_iopc);
	string_init(&s_mnemonics);
	string_init(&s_comments);

	if (iaddr)
		string_printf(&s_addr, "%#011" PRIx64 "  ", addr);

	ptr64_t vaddr;
	vaddr.ptr = addr;

	idump_common(vaddr, instr, &s_iopc, &s_mnemonics, &s_comments);

	if (!iopc)
		string_clear(&s_iopc);

	const char *comment_sep = string_is_empty(&s_comments) ? "" : " # ";
	const char *iopc_sep_after = iopc ? "  " : "";

	printf("  %s%s  %s%-20s%s%s\n",
	    s_addr.str,
	    s_iopc.str, iopc_sep_after,
	    s_mnemonics.str,
	    comment_sep, s_comments.str);

	string_done(&s_addr);
	string_done(&s_iopc);
	string_done(&s_mnemonics);
	string_done(&s_comments);
}

/** Write info about changed registers
 *
 * Each modified register is included to the output.
 *
 */
char *r4k_modified_regs_dump(r4k_cpu_t *cpu)
{
	unsigned int i;
	char *s1;
	char *s2;
	char *s3;
	char sc1[REG_BUF];
	char sc2[REG_BUF];

	char *sx = safe_malloc(REG_BUF);
	size_t size = REG_BUF;

	sc1[0] = 0;
	sc2[0] = 0;
	s1 = sc1;
	s2 = sc2;

	/* Test for general registers */
	for (i = 0; i < 32; i++)
		if (cpu->regs[i].val != cpu->old_regs[i].val) {
			snprintf(s1, size, "%s, %s: %#" PRIx64 "->%#" PRIx64,
			    s2, regname[i], cpu->old_regs[i].val, cpu->regs[i].val);

			s3 = s1;
			s1 = s2;
			s2 = s3;
			cpu->old_regs[i] = cpu->regs[i];
		}

	/* Test for cp0 */
	for (i = 0; i < 32; i++)
		if ((cpu->cp0[i].val != cpu->old_cp0[i].val) && (i != cp0_Random) && (i != cp0_Count)) {
			if (cp0name == cp0_name[2])
				snprintf(s1, size, "%s, cp0_%s: %#" PRIx64 "->%#" PRIx64,
				    s2, cp0name[i], cpu->old_cp0[i].val, cpu->cp0[i].val);
			else
				snprintf(s1, size, "%s, cp0[%u]: %#" PRIx64 "->%#" PRIx64,
				    s2, i, cpu->old_cp0[i].val, cpu->cp0[i].val);

			s3 = s1;
			s1 = s2;
			s2 = s3;
			cpu->old_cp0[i] = cpu->cp0[i];
		}

	/* Test for loreg */
	if (cpu->loreg.val != cpu->old_loreg.val) {
		snprintf(s1, size, "%s, loreg: %#" PRIx64 "->%#" PRIx64,
		    s2, cpu->old_loreg.val, cpu->loreg.val);

		s3 = s1;
		s1 = s2;
		s2 = s3;
		cpu->old_loreg = cpu->loreg;
	}

	/* Test for hireg */
	if (cpu->hireg.val != cpu->old_hireg.val) {
		snprintf(s1, size, "%s, hireg: %#" PRIx64 "->%#" PRIx64,
		    s2, cpu->old_hireg.val, cpu->hireg.val);

		s3 = s1;
		s1 = s2;
		s2 = s3;
		cpu->old_hireg = cpu->hireg;
	}

	if (*s2 == 0)
		*sx = 0;
	else
		strcpy(sx, s2 + 2);

	return sx;
}