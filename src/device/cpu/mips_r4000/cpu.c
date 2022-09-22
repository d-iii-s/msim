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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "../../../debug/breakpoint.h"
#include "../../../debug/debug.h"
#include "../../../debug/gdb.h"
#include "../../device.h"
#include "../../../assert.h"
#include "../../../endian.h"
#include "../../../env.h"
#include "../../../fault.h"
#include "../../../input.h"
#include "../../../main.h"
#include "../../../text.h"
#include "../../../utils.h"
#include "cpu.h"
#include "debug.h"
#include "../../../physmem.h"

/** Register and coprocessor names
 *
 */
char *r4k_reg_name[R4K_REG_VARIANTS][R4K_REG_COUNT] = {
	{
		"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",
		"r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
		"r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
		"r30", "r31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"0",  "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1",
		"t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3",
		"s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp",
		"fp", "ra"
	}
};

char *r4k_cp0_name[R4K_REG_VARIANTS][R4K_REG_COUNT] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"index",    "random",   "entrylo0", "entrylo1",
		"context",  "pagemask", "wired",    "res_7",
		"badvaddr", "count",    "entryhi",  "compare",
		"status",   "cause",    "epc",      "prid",
		"config",   "lladdr",   "watchlo",  "watchhi",
		"xcontext", "res_21",   "res_22",   "res_23",
		"res_24",   "res_25",   "res_26",   "res_27",
		"res_28",   "res_29",   "errorepc", "res_31"
	}
};

char *r4k_cp1_name[R4K_REG_VARIANTS][R4K_REG_COUNT] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp1_0",  "cp1_1",  "cp1_2",  "cp1_3",  "cp1_4",  "cp1_5",  "cp1_6",
		"cp1_7",  "cp1_8",  "cp1_9",  "cp1_10", "cp1_11", "cp1_12", "cp1_13",
		"cp1_14", "cp1_15", "cp1_16", "cp1_17", "cp1_18", "cp1_19", "cp1_20",
		"cp1_21", "cp1_22", "cp1_23", "cp1_24", "cp1_25", "cp1_26", "cp1_27",
		"cp1_28", "cp1_29", "cp1_30", "cp1_31"
	}
};

char *r4k_cp2_name[R4K_REG_VARIANTS][R4K_REG_COUNT] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp2_0",  "cp2_1",  "cp2_2",  "cp2_3",  "cp2_4",  "cp2_5",  "cp2_6",
		"cp2_7",  "cp2_8",  "cp2_9",  "cp2_10", "cp2_11", "cp2_12", "cp2_13",
		"cp2_14", "cp2_15", "cp2_16", "cp2_17", "cp2_18", "cp2_19", "cp2_20",
		"cp2_21", "cp2_22", "cp2_23", "cp2_24", "cp2_25", "cp2_26", "cp2_27",
		"cp2_28", "cp2_29", "cp2_30", "cp2_31"
	}
};

char *r4k_cp3_name[R4K_REG_VARIANTS][R4K_REG_COUNT] = {
	{
		"0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
		"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
		"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
		"30", "31"
	},
	{
		"$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",  "$8",  "$9",
		"$10", "$11", "$12", "$13", "$14", "$15", "$16", "$17", "$18", "$19",
		"$20", "$21", "$22", "$23", "$24", "$25", "$26", "$27", "$28", "$29",
		"$30", "$31"
	},
	{
		"cp3_0",  "cp3_1",  "cp3_2",  "cp3_3",  "cp3_4",  "cp3_5",  "cp3_6",
		"cp3_7",  "cp3_8",  "cp3_9",  "cp3_10", "cp3_11", "cp3_12", "cp3_13",
		"cp3_14", "cp3_15", "cp3_16", "cp3_17", "cp3_18", "cp3_19", "cp3_20",
		"cp3_21", "cp3_22", "cp3_23", "cp3_24", "cp3_25", "cp3_26", "cp3_27",
		"cp3_28", "cp3_29", "cp3_30", "cp3_31"
	}
};

/** Sign extensions
 *
 */

#define SBIT8   UINT8_C(0x80)
#define SBIT16  UINT16_C(0x8000)
#define SBIT32  UINT32_C(0x80000000)
#define SBIT64  UINT64_C(0x8000000000000000)

#define EXTEND_POSITIVE_16_32  UINT32_C(0x0000ffff)
#define EXTEND_NEGATIVE_16_32  UINT32_C(0xffff0000)

#define EXTEND_POSITIVE_8_64  UINT64_C(0x00000000000000ff)
#define EXTEND_NEGATIVE_8_64  UINT64_C(0xffffffffffffff00)

#define EXTEND_POSITIVE_16_64  UINT64_C(0x000000000000ffff)
#define EXTEND_NEGATIVE_16_64  UINT64_C(0xffffffffffff0000)

#define EXTEND_POSITIVE_32_64  UINT64_C(0x00000000ffffffff)
#define EXTEND_NEGATIVE_32_64  UINT64_C(0xffffffff00000000)

static uint32_t sign_extend_16_32(uint16_t val)
{
	return ((val & SBIT16) ?
	    (((uint32_t) val) | EXTEND_NEGATIVE_16_32) :
	    (((uint32_t) val) & EXTEND_POSITIVE_16_32));
}

static uint64_t sign_extend_8_64(uint8_t val)
{
	return ((val & SBIT8) ?
	    (((uint64_t) val) | EXTEND_NEGATIVE_8_64) :
	    (((uint64_t) val) & EXTEND_POSITIVE_8_64));
}

static uint64_t sign_extend_16_64(uint16_t val)
{
	return ((val & SBIT16) ?
	    (((uint64_t) val) | EXTEND_NEGATIVE_16_64) :
	    (((uint64_t) val) & EXTEND_POSITIVE_16_64));
}

static uint64_t sign_extend_32_64(uint32_t val)
{
	return ((val & SBIT32) ?
	    (((uint64_t) val) | EXTEND_NEGATIVE_32_64) :
	    (((uint64_t) val) & EXTEND_POSITIVE_32_64));
}

/** Bitfield operations
 *
 */

#define TARGET_SHIFT  2
#define TARGET_COMB   UINT64_C(0xfffffffff0000000)

/** CPU mode macros
 *
 */

#define CPU_TLB_SHUTDOWN(cpu) \
	(cp0_status_ts(cpu) == 1)

#define CPU_USER_MODE(cpu) \
	((cp0_status_ksu(cpu) == 2) && (!cp0_status_exl(cpu)) && \
	    (!cp0_status_erl(cpu)))

#define CPU_SUPERVISOR_MODE(cpu) \
	((cp0_status_ksu(cpu) == 1) && (!cp0_status_exl(cpu)) && \
	    (!cp0_status_erl(cpu)))

#define CPU_KERNEL_MODE(cpu) \
	((cp0_status_ksu(cpu) == 0) || (cp0_status_exl(cpu)) || \
	    (cp0_status_erl(cpu)))

#define CP0_USABLE(cpu) \
	((cp0_status_cu0(cpu)) || (CPU_KERNEL_MODE(cpu)))

#define CPU_64BIT_MODE(cpu) \
	(((CPU_KERNEL_MODE(cpu)) && (cp0_status_kx(cpu))) || \
	    ((CPU_SUPERVISOR_MODE(cpu)) && (cp0_status_sx(cpu))) || \
	    ((CPU_USER_MODE(cpu)) && (cp0_status_ux(cpu))))

#define CPU_64BIT_INSTRUCTION(cpu) \
	((CPU_KERNEL_MODE(cpu)) || \
	    ((CPU_SUPERVISOR_MODE(cpu)) && (cp0_status_sx(cpu))) || \
	    ((CPU_USER_MODE(cpu)) && (cp0_status_ux(cpu))))

/** Virtual memory segments
 *
 */

/** User mode segments */
#define USEG_MASK   UINT32_C(0x80000000)
#define USEG_BITS   UINT32_C(0x00000000)
#define XUSEG_MASK  UINT64_C(0xffffff0000000000)
#define XUSEG_BITS  UINT64_C(0x0000000000000000)

/** Supervisor mode segments */
#define SUSEG_MASK   USEG_MASK
#define SUSEG_BITS   USEG_BITS
#define XSUSEG_MASK  XUSEG_MASK
#define XSUSEG_BITS  XUSEG_BITS

#define SSEG_MASK   UINT32_C(0xe0000000)
#define SSEG_BITS   UINT32_C(0xc0000000)
#define XSSEG_MASK  UINT64_C(0xffffff0000000000)
#define XSSEG_BITS  UINT64_C(0xc000000000000000)

#define CSSEG_MASK  UINT64_C(0xffffffffe0000000)
#define CSSEG_BITS  UINT64_C(0xffffffffc0000000)

/** Kernel mode segments */
#define KUSEG_MASK    USEG_MASK
#define KUSEG_BITS    USEG_BITS
#define XKSUSEG_MASK  XUSEG_MASK
#define XKSUSEG_BITS  XUSEG_BITS

#define KSEG0_MASK   UINT32_C(0xe0000000)
#define KSEG0_BITS   UINT32_C(0x80000000)
#define CKSEG0_MASK  UINT64_C(0xffffffffe0000000)
#define CKSEG0_BITS  UINT64_C(0xffffffff80000000)

#define KSEG1_MASK   UINT32_C(0xe0000000)
#define KSEG1_BITS   UINT32_C(0xa0000000)
#define CKSEG1_MASK  UINT32_C(0xffffffffe0000000)
#define CKSEG1_BITS  UINT32_C(0xffffffffa0000000)

#define KSSEG_MASK   SSEG_MASK
#define KSSEG_BITS   SSEG_BITS
#define CKSSEG_MASK  CSSEG_MASK
#define CKSSEG_BITS  CSSEG_BITS

#define KSEG3_MASK   UINT32_C(0xe0000000)
#define KSEG3_BITS   UINT32_C(0xe0000000)
#define CKSEG3_MASK  UINT32_C(0xffffffffe0000000)
#define CKSEG3_BITS  UINT32_C(0xffffffffe0000000)

#define XKSSEG_MASK  UINT64_C(0xffffff0000000000)
#define XKSSEG_BITS  UINT64_C(0x4000000000000000)

#define XKPHYS_MASK  UINT64_C(0xc000000000000000)
#define XKPHYS_BITS  UINT64_C(0x8000000000000000)

#define XKSEG_MASK  UINT64_C(0xffffff0080000000)
#define XKSEG_BITS  UINT64_C(0xc000000000000000)

/** TLB lookup result */
typedef enum {
	TLBL_OK,
	TLBL_REFILL,
	TLBL_INVALID,
	TLBL_MODIFIED
} tlb_look_t;

/** Memory access modes */
typedef enum {
	AM_FETCH,
	AM_READ,
	AM_WRITE
} acc_mode_t;

/** Partial memory access shift tables
 *
 */

typedef struct {
	uint32_t mask;
	uint32_t shift;
} shift_tab_t;

static shift_tab_t shift_tab_left[] = {
	{ UINT32_C(0x00ffffff), 24 },
	{ UINT32_C(0x0000ffff), 16 },
	{ UINT32_C(0x000000ff), 8 },
	{ UINT32_C(0x00000000), 0 }
};

static shift_tab_t shift_tab_right[] = {
	{ UINT32_C(0x00000000), 0 },
	{ UINT32_C(0xff000000), 8 },
	{ UINT32_C(0xffff0000), 16 },
	{ UINT32_C(0xffffff00), 24 }
};

static shift_tab_t shift_tab_left_store[] = {
	{ UINT32_C(0xffffff00), 24 },
	{ UINT32_C(0xffff0000), 16 },
	{ UINT32_C(0xff000000), 8 },
	{ UINT32_C(0x00000000), 0 }
};

static shift_tab_t shift_tab_right_store[] = {
	{ UINT32_C(0x00000000), 0 },
	{ UINT32_C(0x000000ff), 8 },
	{ UINT32_C(0x0000ffff), 16 },
	{ UINT32_C(0x00ffffff), 24 }
};

/** Address traslation through the TLB table
 *
 * See tlb_look_t definition
 *
 */
static tlb_look_t tlb_look(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	/* Ignore TLB on shutdown */
	if (CPU_TLB_SHUTDOWN(cpu))
		return TLBL_OK;
	
	unsigned int hint = cpu->tlb_hint;
	unsigned int i;
	
	/* Look for the TBL hit */
	for (i = 0; i < TLB_ENTRIES; i++) {
		tlb_entry_t *entry = &cpu->tlb[(i + hint) % TLB_ENTRIES];
		
		/* TLB hit? */
		if ((virt.lo & entry->mask) == entry->vpn2) {
			/* Test ASID */
			if ((!entry->global) && (entry->asid != cp0_entryhi_asid(cpu)))
				continue;
			
			/* Calculate subpage */
			ptr36_t smask = (ptr36_t) (entry->mask >> 1) | TLB_PHYSMASK;
			unsigned int subpage =
			    ((virt.lo & entry->mask) < (virt.lo & smask)) ? 1 : 0;
			
			/* Test valid & dirty */
			if (!entry->pg[subpage].valid)
				return TLBL_INVALID;
			
			if ((wr) && (!entry->pg[subpage].dirty))
				return TLBL_MODIFIED;
			
			/* Make address */
			ptr36_t amask = virt.lo & (~smask);
			*phys = amask | (entry->pg[subpage].pfn & smask);
			
			/* Update optimization hint */
			cpu->tlb_hint = i;
			
			return TLBL_OK;
		}
	}
	
	return TLBL_REFILL;
}

/** Fill up cp0 registers with specified address
 *
 */
static void fill_tlb_error(r4k_cpu_t *cpu, ptr64_t addr)
{
	ASSERT(cpu != NULL);
	
	cp0_badvaddr(cpu).val = addr.ptr;
	
	cp0_context(cpu).val &= cp0_context_ptebase_mask;
	cp0_context(cpu).val |= (addr.ptr >> cp0_context_addr_shift)
	    & ~cp0_context_res1_mask;
	
	cp0_xcontext(cpu).val &= cp0_xcontext_ptebase_mask;
	cp0_xcontext(cpu).val |= (addr.ptr >> cp0_xcontext_addr_shift)
	    & ~cp0_xcontext_res1_mask;
	
	cp0_entryhi(cpu).val &= cp0_entryhi_asid_mask;
	cp0_entryhi(cpu).val |= addr.ptr & cp0_entryhi_vpn2_mask;
}

/** Fill registers as the Address error exception occures
 *
 */
static void fill_addr_error(r4k_cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if (noisy) {
		cp0_badvaddr(cpu).val = addr.ptr;
		cp0_context(cpu).val &= ~cp0_context_badvpn2_mask;    /* Undefined */
		cp0_xcontext(cpu).val &= ~cp0_xcontext_badvpn2_mask;  /* Undefined */
		cp0_entryhi(cpu).val &= ~cp0_entryhi_vpn2_mask;       /* Undefined */
	}
}

/** Search through TLB and generates apropriate exception (32 bits)
 *
 */
static r4k_exc_t tlb_hit32(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	switch (tlb_look(cpu, virt, phys, wr)) {
	case TLBL_OK:
		break;
	case TLBL_REFILL:
		if (noisy) {
			cpu->tlb_refill++;
			fill_tlb_error(cpu, virt);
		}
		
		return r4k_excTLBR;
	case TLBL_INVALID:
		if (noisy) {
			cpu->tlb_invalid++;
			fill_tlb_error(cpu, virt);
		}
		
		return r4k_excTLB;
	case TLBL_MODIFIED:
		if (noisy) {
			cpu->tlb_modified++;
			fill_tlb_error(cpu, virt);
		}
		
		return r4k_excMod;
	}
	
	return r4k_excNone;
}

/** Search through TLB and generates apropriate exception (64 bits)
 *
 */
static r4k_exc_t tlb_hit64(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	ASSERT(false);
	
	return r4k_excNone;
}

/** The user mode address conversion (32 bits)
 *
 */
static r4k_exc_t convert_addr_user32(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_USER_MODE(cpu));
	
	if ((virt.lo & USEG_MASK) == USEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The user mode address conversion (64 bits)
 *
 */
static r4k_exc_t convert_addr_user64(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_USER_MODE(cpu));
	
	if ((virt.ptr & XUSEG_MASK) == XUSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The supervisor mode address conversion (32 bits)
 *
 */
static r4k_exc_t convert_addr_supervisor32(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_SUPERVISOR_MODE(cpu));
	
	if ((virt.lo & SUSEG_MASK) == SUSEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	if ((virt.lo & SSEG_MASK) == SSEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The supervisor mode address conversion (64 bits)
 *
 */
static r4k_exc_t convert_addr_supervisor64(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_SUPERVISOR_MODE(cpu));
	
	if ((virt.ptr & XSUSEG_MASK) == XSUSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & XSSEG_MASK) == XSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & CSSEG_MASK) == CSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The kernel mode address conversion (32 bits)
 *
 */
static r4k_exc_t convert_addr_kernel32(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_KERNEL_MODE(cpu));
	
	if ((virt.lo & KUSEG_MASK) == KUSEG_BITS) {
		if (!cp0_status_erl(cpu))
			return tlb_hit32(cpu, virt, phys, wr, noisy);
		
		return r4k_excNone;
	}
	
	if ((virt.lo & KSEG0_MASK) == KSEG0_BITS) {
		*phys = virt.lo - UINT32_C(0x80000000);
		return r4k_excNone;
	}
	
	if ((virt.lo & KSEG1_MASK) == KSEG1_BITS) {
		*phys = virt.lo - UINT32_C(0xa0000000);
		return r4k_excNone;
	}
	
	if ((virt.lo & KSSEG_MASK) == KSSEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	if ((virt.lo & KSEG3_MASK) == KSEG3_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The kernel mode address conversion (64 bits)
 *
 */
static r4k_exc_t convert_addr_kernel64(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_KERNEL_MODE(cpu));
	
	if ((virt.ptr & XKSUSEG_MASK) == XKSUSEG_BITS) {
		if (!cp0_status_erl(cpu))
			return tlb_hit64(cpu, virt, phys, wr, noisy);
		
		return r4k_excNone;
	}
	
	if ((virt.ptr & XKSSEG_MASK) == XKSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & XKPHYS_MASK) == XKPHYS_BITS)
		ASSERT(false);
	
	if ((virt.ptr & XKSEG_MASK) == XKSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & CKSEG0_MASK) == CKSEG0_BITS) {
		*phys = virt.ptr - UINT64_C(0x80000000);
		return r4k_excNone;
	}
	
	if ((virt.ptr & CKSEG1_MASK) == CKSEG1_BITS) {
		*phys = virt.ptr - UINT64_C(0xa0000000);
		return r4k_excNone;
	}
	
	if ((virt.lo & CKSSEG_MASK) == CKSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.lo & CKSEG3_MASK) == CKSEG3_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return r4k_excAddrError;
}

/** The conversion of virtual addresses
 *
 * @param write Write access.
 * @param noisy Fill apropriate processor registers
 *              if the address is incorrect.
 *
 */
r4k_exc_t r4k_convert_addr(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);

	/*
	 * The manual does not explicitly specify what to do
	 * if the KSU bits are 0b11 (i.e. neither user, supervisor
	 * or kernel).
	 * For debugging purposes we deem it best to announce address
	 * error as the translation cannot be completed.
	 */

	if (CPU_64BIT_MODE(cpu)) {
		if (CPU_USER_MODE(cpu))
			return convert_addr_user64(cpu, virt, phys, write, noisy);
		
		if (CPU_SUPERVISOR_MODE(cpu))
			return convert_addr_supervisor64(cpu, virt, phys, write, noisy);

		if (CPU_KERNEL_MODE(cpu))
			convert_addr_kernel64(cpu, virt, phys, write, noisy);

		fill_addr_error(cpu, virt, noisy);
		return r4k_excAddrError;
	} else {
		if (CPU_USER_MODE(cpu))
			return convert_addr_user32(cpu, virt, phys, write, noisy);
		
		if (CPU_SUPERVISOR_MODE(cpu))
			return convert_addr_supervisor32(cpu, virt, phys, write, noisy);

		if (CPU_KERNEL_MODE(cpu))
			return convert_addr_kernel32(cpu, virt, phys, write, noisy);

		fill_addr_error(cpu, virt, noisy);
		return r4k_excAddrError;
	}
}

/** Test for correct alignment (16 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static r4k_exc_t align_test16(r4k_cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x01U) != 0) {
		fill_addr_error(cpu, addr, noisy);
		return r4k_excAddrError;
	}
	
	return r4k_excNone;
}

/** Test for correct alignment (32 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static r4k_exc_t align_test32(r4k_cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x03U) != 0) {
		fill_addr_error(cpu, addr, noisy);
		return r4k_excAddrError;
	}
	
	return r4k_excNone;
}

/** Test for correct alignment (64 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static r4k_exc_t align_test64(r4k_cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x07U) != 0) {
		fill_addr_error(cpu, addr, noisy);
		return r4k_excAddrError;
	}
	
	return r4k_excNone;
}

/** Access the virtual memory
 *
 * The operation (read/write) is specified via the wr parameter.
 * This routine does not specify the exception type.
 *
 * If the exception occures, the value does not change.
 *
 * @param mode  Memory access mode
 * @param virt  Virtual memory address
 * @param phys  Physical memory address
 * @param noisy Generate exception in case of invalid operation
 *
 */
static r4k_exc_t access_mem(r4k_cpu_t *cpu, acc_mode_t mode, ptr64_t virt,
    ptr36_t *phys, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	r4k_exc_t res = r4k_convert_addr(cpu, virt, phys, mode == AM_WRITE, noisy);
	
	/* Check for watched address */
	if (((cp0_watchlo_r(cpu)) && (mode == AM_READ))
	    || ((cp0_watchlo_w(cpu)) && (mode == AM_WRITE))) {
		
		/* The matching is done on
		   8-byte aligned addresses */
		
		if (cpu->waddr == (*phys >> 3)) {
			/*
			 * If EXL is set, the exception has to be postponed,
			 * the memory access should (probably) proceed.
			 */
			if (cp0_status_exl(cpu) == 1) {
				cpu->wpending = true;
				cpu->wexcaddr = cpu->pc;
			} else
				return r4k_excWATCH;
		}
	}
	
	return res;
}

/** Preform read operation from the virtual memory (8 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static r4k_exc_t cpu_read_mem8(r4k_cpu_t *cpu, ptr64_t addr, uint8_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	ptr36_t phys;
	r4k_exc_t res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	case r4k_excTLB:
		return r4k_excTLBL;
	case r4k_excTLBR:
		return r4k_excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read8(cpu->procno, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (16 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static r4k_exc_t cpu_read_mem16(r4k_cpu_t *cpu, ptr64_t addr, uint16_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	r4k_exc_t res = align_test16(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	case r4k_excTLB:
		return r4k_excTLBL;
	case r4k_excTLBR:
		return r4k_excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read16(cpu->procno, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (32 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
r4k_exc_t r4k_read_mem32(r4k_cpu_t *cpu, ptr64_t addr, uint32_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	r4k_exc_t res = align_test32(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	case r4k_excTLB:
		return r4k_excTLBL;
	case r4k_excTLBR:
		return r4k_excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read32(cpu->procno, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (64 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static r4k_exc_t cpu_read_mem64(r4k_cpu_t *cpu, ptr64_t addr, uint64_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	r4k_exc_t res = align_test64(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdEL;
	case r4k_excTLB:
		return r4k_excTLBL;
	case r4k_excTLBR:
		return r4k_excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read64(cpu->procno, phys, true);
	return res;
}

/** Perform write operation to the virtual memory (8 bits)
 *
 */
static r4k_exc_t cpu_write_mem8(r4k_cpu_t *cpu, ptr64_t addr, uint8_t value, bool noisy)
{
	ASSERT(cpu != NULL);
	
	ptr36_t phys;
	r4k_exc_t res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	case r4k_excTLB:
		return r4k_excTLBS;
	case r4k_excTLBR:
		return r4k_excTLBSR;
	case r4k_excMod:
		return r4k_excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write8(cpu->procno, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (16 bits)
 *
 */
static r4k_exc_t cpu_write_mem16(r4k_cpu_t *cpu, ptr64_t addr, uint16_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	r4k_exc_t res = align_test16(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	case r4k_excTLB:
		return r4k_excTLBS;
	case r4k_excTLBR:
		return r4k_excTLBSR;
	case r4k_excMod:
		return r4k_excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write16(cpu->procno, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (32 bits)
 *
 */
static r4k_exc_t cpu_write_mem32(r4k_cpu_t *cpu, ptr64_t addr, uint32_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	r4k_exc_t res = align_test32(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	case r4k_excTLB:
		return r4k_excTLBS;
	case r4k_excTLBR:
		return r4k_excTLBSR;
	case r4k_excMod:
		return r4k_excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write32(cpu->procno, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (64 bits)
 *
 */
static r4k_exc_t cpu_write_mem64(r4k_cpu_t *cpu, ptr64_t addr, uint64_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	r4k_exc_t res = align_test64(cpu, addr, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		return r4k_excAdES;
	case r4k_excTLB:
		return r4k_excTLBS;
	case r4k_excTLBR:
		return r4k_excTLBSR;
	case r4k_excMod:
		return r4k_excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write64(cpu->procno, phys, value, true);
	return res;
}

/** Probe TLB entry
 *
 */
static r4k_exc_t TLBP(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	if (CP0_USABLE(cpu)) {
		cp0_index(cpu).val = 1 << cp0_index_p_shift;
		uint32_t xvpn2 = cp0_entryhi(cpu).val & cp0_entryhi_vpn2_mask;
		uint32_t xasid = cp0_entryhi(cpu).val & cp0_entryhi_asid_mask;
		unsigned int i;
		
		for (i = 0; i < TLB_ENTRIES; i++) {
			/*
			 * Mask the VPN2 value from EntryHi with the PageMask
			 * value from the TLB before comparing with the VPN2
			 * value from the TLB. This does not respect the official
			 * R4000 documentation, but it is compliant with the
			 * behaviour of other MIPS CPUs and it is actually
			 * necessary for proper support for multiple page
			 * sizes.
			 */
			if ((cpu->tlb[i].vpn2 == (xvpn2 & cpu->tlb[i].mask)) &&
			    ((cpu->tlb[i].global) || (cpu->tlb[i].asid == xasid))) {
				cp0_index(cpu).val = i;
				break;
			}
		}
		
		return r4k_excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return r4k_excCpU;
}

/** Read entry from the TLB
 *
 */
static r4k_exc_t TLBR(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	if (CP0_USABLE(cpu)) {
		unsigned int i = cp0_index_index(cpu);
		
		if (i > 47) {
			alert("R4000: Invalid value in Index (TLBR)");
			cp0_pagemask(cpu).val = 0;
			cp0_entryhi(cpu).val = 0;
			cp0_entrylo0(cpu).val = 0;
			cp0_entrylo1(cpu).val = 0;
		} else {
			cp0_pagemask(cpu).val = (~cpu->tlb[i].mask) & UINT32_C(0x01ffe000);
			cp0_entryhi(cpu).val = cpu->tlb[i].vpn2 | cpu->tlb[i].asid;
			
			cp0_entrylo0(cpu).val = (cpu->tlb[i].pg[0].pfn >> 6)
			    | (cpu->tlb[i].pg[0].cohh << 3)
			    | ((cpu->tlb[i].pg[0].dirty ? 1 : 0) << 2)
			    | ((cpu->tlb[i].pg[0].valid ? 1 : 0) << 1)
			    | (cpu->tlb[i].global ? 1 : 0);
			
			cp0_entrylo1(cpu).val = (cpu->tlb[i].pg[1].pfn >> 6)
			    | (cpu->tlb[i].pg[1].cohh << 3)
			    | ((cpu->tlb[i].pg[1].dirty ? 1 : 0) << 2)
			    | ((cpu->tlb[i].pg[1].valid ? 1 : 0) << 1)
			    | (cpu->tlb[i].global ? 1 : 0);
		}
		
		return r4k_excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return r4k_excCpU;
}

/** Write a new entry into the TLB
 *
 * The entry index is determined by either
 * the random register (TLBWR) or index (TLBWI).
 *
 */
static r4k_exc_t TLBW(r4k_cpu_t *cpu, bool random)
{
	ASSERT(cpu != NULL);
	
	if (CP0_USABLE(cpu)) {
		
		unsigned int index =
		    random ? cp0_random_random(cpu) : cp0_index_index(cpu);
		
		if (index > 47) {
			/*
			 * Undefined behavior, doing nothing complies.
			 * Random is read-only, its index should be always fine.
			 */
			alert("R4000: Invalid value in Index (TLBWI)");
		} else {
			/* Fill TLB */
			tlb_entry_t *entry = &cpu->tlb[index];
			
			entry->mask = cp0_entryhi_vpn2_mask & ~cp0_pagemask(cpu).val;
			entry->vpn2 = cp0_entryhi(cpu).val & entry->mask;
			entry->global = cp0_entrylo0_g(cpu) & cp0_entrylo1_g(cpu);
			entry->asid = cp0_entryhi_asid(cpu);
			
			entry->pg[0].pfn = (ptr36_t) cp0_entrylo0_pfn(cpu) << 12;
			entry->pg[0].cohh = cp0_entrylo0_c(cpu);
			entry->pg[0].dirty = cp0_entrylo0_d(cpu);
			entry->pg[0].valid = cp0_entrylo0_v(cpu);
			
			entry->pg[1].pfn = (ptr36_t) cp0_entrylo1_pfn(cpu) << 12;
			entry->pg[1].cohh = cp0_entrylo1_c(cpu);
			entry->pg[1].dirty = cp0_entrylo1_d(cpu);
			entry->pg[1].valid = cp0_entrylo1_v(cpu);
		}
		
		/*
		 * Invalidate current CPU binary translation frame.
		 * This should be actually only necessary if the page
		 * with the current PC is affected by the TLB change.
		 * But let's take this conservative precaution in order
		 * not to miss any corner cases.
		 */
		cpu->frame = NULL;
		
		return r4k_excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return r4k_excCpU;
}

/** Disassemble aid routines
 *
 */

static void disassemble_offset(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	int64_t offset =
	    (((int64_t) sign_extend_16_64(instr.i.imm)) << TARGET_SHIFT);
	
	ptr64_t target;
	target.ptr = addr.ptr + offset + 4;
	
	string_printf(mnemonics, " %#" PRIx64, target.ptr);
	
	if (offset + 4 > 0)
		string_printf(comments, "forward");
	else if (offset + 4 < 0)
		string_printf(comments, "backward");
	else
		string_printf(comments, "here");
}

static void disassemble_rs_offset(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, " %s,", r4k_regname[instr.i.rs]);
	disassemble_offset(addr, instr, mnemonics, comments);
}

static void disassemble_rs_rt_offset(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, " %s, %s,",
	    r4k_regname[instr.i.rs], r4k_regname[instr.i.rt]);
	disassemble_offset(addr, instr, mnemonics, comments);
}

static void disassemble_target(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	ptr64_t target;
	target.ptr =
	    ((addr.ptr + 4) & TARGET_COMB) | (instr.j.target << TARGET_SHIFT);
	
	string_printf(mnemonics, " %#" PRIx64, target.ptr);
}

static void disassemble_rt_offset_base(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	int64_t offset = (int64_t) sign_extend_16_64(instr.i.imm);
	
	string_printf(mnemonics, " %s, %" PRId64 "(%s)",
	    r4k_regname[instr.i.rt], offset, r4k_regname[instr.i.rs]);
}

static void disassemble_rs_rt(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s,",
	    r4k_regname[instr.i.rs], r4k_regname[instr.i.rt]);
}

static void disassemble_rd_rs_rt(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s, %s",
	    r4k_regname[instr.r.rd], r4k_regname[instr.r.rs], r4k_regname[instr.r.rt]);
}

static void disassemble_rt_rs(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s",
	    r4k_regname[instr.r.rt], r4k_regname[instr.r.rs]);
}

static void disassemble_rt_cp0(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s",
	    r4k_regname[instr.r.rt], r4k_cp0name[instr.r.rd]);
}

static void disassemble_rt_fs(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s",
	    r4k_regname[instr.r.rt], r4k_cp1name[instr.r.rs]);
}

static void disassemble_rt_cp2(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s",
	    r4k_regname[instr.r.rt], r4k_cp2name[instr.r.rs]);
}

static void disassemble_rt_rs_imm(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	int32_t imm = (int32_t) sign_extend_16_32(instr.i.imm);
	
	string_printf(mnemonics, " %s, %s, %" PRId32,
	    r4k_regname[instr.i.rt], r4k_regname[instr.i.rs], imm);
}

static void disassemble_rt_rs_uimm(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s, %#x",
	    r4k_regname[instr.i.rt], r4k_regname[instr.i.rs], instr.i.imm);
}

static void disassemble_rt_imm(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	int32_t imm = (int32_t) sign_extend_16_32(instr.i.imm);
	
	string_printf(mnemonics, " %s, %" PRId32,
	    r4k_regname[instr.i.rt], imm);
}

static void disassemble_rt_uimm(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %#x",
	    r4k_regname[instr.i.rt], instr.i.imm);
}

static void disassemble_rs_imm(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	int32_t imm = (int32_t) sign_extend_16_32(instr.i.imm);
	
	string_printf(mnemonics, " %s, %" PRId32,
	    r4k_regname[instr.i.rs], imm);
}

static void disassemble_rd_rt_sa(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s, %u",
	    r4k_regname[instr.r.rd], r4k_regname[instr.r.rt], instr.r.sa);
}

static void disassemble_rd_rt_rs(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s, %s",
	    r4k_regname[instr.r.rd], r4k_regname[instr.r.rt], r4k_regname[instr.r.rs]);
}

static void disassemble_rd_rs(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s, %s",
	    r4k_regname[instr.r.rd], r4k_regname[instr.r.rs]);
}

static void disassemble_rs(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s", r4k_regname[instr.r.rs]);
}

static void disassemble_rd(r4k_instr_t instr, string_t *mnemonics,
    string_t *comments)
{
	string_printf(mnemonics, " %s", r4k_regname[instr.r.rd]);
}

/** Implementation of instructions of R4000
 *
 * Include those instructions which are supported
 * by R4000.
 *
 */

#include "instr/_reserved.c"
#include "instr/_warning.c"
#include "instr/_xcrd.c"
#include "instr/_xhlt.c"
#include "instr/_xrd.c"
#include "instr/_xtrc.c"
#include "instr/_xtr0.c"
#include "instr/_xval.c"
#include "instr/add.c"
#include "instr/addi.c"
#include "instr/addiu.c"
#include "instr/addu.c"
#include "instr/and.c"
#include "instr/andi.c"
#include "instr/bc0f.c"
#include "instr/bc0fl.c"
#include "instr/bc0t.c"
#include "instr/bc0tl.c"
#include "instr/bc1f.c"
#include "instr/bc1fl.c"
#include "instr/bc1t.c"
#include "instr/bc1tl.c"
#include "instr/bc2f.c"
#include "instr/bc2fl.c"
#include "instr/bc2t.c"
#include "instr/bc2tl.c"
#include "instr/beq.c"
#include "instr/beql.c"
#include "instr/bgez.c"
#include "instr/bgezal.c"
#include "instr/bgezall.c"
#include "instr/bgezl.c"
#include "instr/bgtz.c"
#include "instr/bgtzl.c"
#include "instr/blez.c"
#include "instr/blezl.c"
#include "instr/bltz.c"
#include "instr/bltzal.c"
#include "instr/bltzall.c"
#include "instr/bltzl.c"
#include "instr/bne.c"
#include "instr/bnel.c"
#include "instr/break.c"
#include "instr/cache.c"
#include "instr/cfc1.c"
#include "instr/cfc2.c"
#include "instr/ctc1.c"
#include "instr/ctc2.c"
#include "instr/dadd.c"
#include "instr/daddi.c"
#include "instr/daddiu.c"
#include "instr/daddu.c"
#include "instr/ddiv.c"
#include "instr/ddivu.c"
#include "instr/div.c"
#include "instr/divu.c"
#include "instr/dsll.c"
#include "instr/dsllv.c"
#include "instr/dsll32.c"
#include "instr/dsra.c"
#include "instr/dsrav.c"
#include "instr/dsra32.c"
#include "instr/dsrl.c"
#include "instr/dsrlv.c"
#include "instr/dsrl32.c"
#include "instr/dsub.c"
#include "instr/dsubu.c"
#include "instr/dmfc0.c"
#include "instr/dmfc1.c"
#include "instr/dmtc0.c"
#include "instr/dmtc1.c"
#include "instr/dmult.c"
#include "instr/dmultu.c"
#include "instr/eret.c"
#include "instr/j.c"
#include "instr/jal.c"
#include "instr/jalr.c"
#include "instr/jr.c"
#include "instr/lb.c"
#include "instr/lbu.c"
#include "instr/ld.c"
#include "instr/ldc1.c"
#include "instr/ldc2.c"
#include "instr/ldl.c"
#include "instr/ldr.c"
#include "instr/lh.c"
#include "instr/lhu.c"
#include "instr/ll.c"
#include "instr/lld.c"
#include "instr/lui.c"
#include "instr/lw.c"
#include "instr/lwc1.c"
#include "instr/lwc2.c"
#include "instr/lwl.c"
#include "instr/lwr.c"
#include "instr/lwu.c"
#include "instr/mfc0.c"
#include "instr/mfc1.c"
#include "instr/mfc2.c"
#include "instr/mfhi.c"
#include "instr/mflo.c"
#include "instr/mtc0.c"
#include "instr/mtc1.c"
#include "instr/mthi.c"
#include "instr/mtlo.c"
#include "instr/mult.c"
#include "instr/multu.c"
#include "instr/nor.c"
#include "instr/_xint.c"
#include "instr/or.c"
#include "instr/ori.c"
#include "instr/sb.c"
#include "instr/sc.c"
#include "instr/scd.c"
#include "instr/sd.c"
#include "instr/sdc1.c"
#include "instr/sdc2.c"
#include "instr/sdl.c"
#include "instr/sdr.c"
#include "instr/sh.c"
#include "instr/sll.c"
#include "instr/sllv.c"
#include "instr/slt.c"
#include "instr/slti.c"
#include "instr/sltiu.c"
#include "instr/sltu.c"
#include "instr/sra.c"
#include "instr/srav.c"
#include "instr/srl.c"
#include "instr/srlv.c"
#include "instr/sub.c"
#include "instr/subu.c"
#include "instr/sw.c"
#include "instr/swc1.c"
#include "instr/swc2.c"
#include "instr/swl.c"
#include "instr/swr.c"
#include "instr/sync.c"
#include "instr/syscall.c"
#include "instr/teq.c"
#include "instr/teqi.c"
#include "instr/tge.c"
#include "instr/tgei.c"
#include "instr/tgeiu.c"
#include "instr/tgeu.c"
#include "instr/tlbp.c"
#include "instr/tlbr.c"
#include "instr/tlbwi.c"
#include "instr/tlbwr.c"
#include "instr/tlt.c"
#include "instr/tlti.c"
#include "instr/tltiu.c"
#include "instr/tltu.c"
#include "instr/tne.c"
#include "instr/tnei.c"
#include "instr/xor.c"
#include "instr/xori.c"

/** Instruction decoding tables
 *
 */

static instr_fnc_t opcode_map[64] = {
	/* 0 */
	instr__reserved,  /* r4k_opcSPECIAL */
	instr__reserved,  /* r4k_opcREGIMM */
	instr_j,
	instr_jal,
	instr_beq,
	instr_bne,
	instr_blez,
	instr_bgtz,
	
	/* 8 */
	instr_addi,
	instr_addiu,
	instr_slti,
	instr_sltiu,
	instr_andi,
	instr_ori,
	instr_xori,
	instr_lui,
	
	/* 16 */
	instr__reserved,  /* r4k_opcCOP0 */
	instr__reserved,  /* r4k_opcCOP1 */
	instr__reserved,  /* r4k_opcCOP2 */
	instr__reserved,  /* unused */
	instr_beql,
	instr_bnel,
	instr_blezl,
	instr_bgtzl,
	
	/* 24 */
	instr_daddi,
	instr_daddiu,
	instr_ldl,
	instr_ldr,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	/* 32 */
	instr_lb,
	instr_lh,
	instr_lwl,
	instr_lw,
	instr_lbu,
	instr_lhu,
	instr_lwr,
	instr_lwu,
	
	/* 40 */
	instr_sb,
	instr_sh,
	instr_swl,
	instr_sw,
	instr_sdl,
	instr_sdr,
	instr_swr,
	instr_cache,
	
	/* 48 */
	instr_ll,
	instr_lwc1,
	instr_lwc2,
	instr__reserved,  /* unused */
	instr_lld,
	instr_ldc1,
	instr_ldc2,
	instr_ld,
	
	instr_sc,
	instr_swc1,
	instr_swc2,
	instr__reserved,  /* unused */
	instr_scd,
	instr_sdc1,
	instr_sdc2,
	instr_sd
};

static instr_fnc_t func_map[64] = {
	instr_sll,
	instr__reserved,  /* unused */
	instr_srl,
	instr_sra,
	instr_sllv,
	instr__reserved,  /* unused */
	instr_srlv,
	instr_srav,
	
	instr_jr,
	instr_jalr,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr_syscall,
	instr_break,
	instr__xcrd,  /* unused */
	instr_sync,
	
	instr_mfhi,
	instr_mthi,
	instr_mflo,
	instr_mtlo,
	instr_dsllv,
	instr__reserved,  /* unused */
	instr_dsrlv,
	instr_dsrav,
	
	instr_mult,
	instr_multu,
	instr_div,
	instr_divu,
	instr_dmult,
	instr_dmultu,
	instr_ddiv,
	instr_ddivu,
	
	instr_add,
	instr_addu,
	instr_sub,
	instr_subu,
	instr_and,
	instr_or,
	instr_xor,
	instr_nor,
	
	instr__xhlt,
	instr__xint,
	instr_slt,
	instr_sltu,
	instr_dadd,
	instr_daddu,
	instr_dsub,
	instr_dsubu,
	
	instr_tge,
	instr_tgeu,
	instr_tlt,
	instr_tltu,
	instr_teq,
	instr__xval,
	instr_tne,
	instr__xrd,
	
	instr_dsll,
	instr__xtrc,
	instr_dsrl,
	instr_dsra,
	instr_dsll32,
	instr__xtr0,
	instr_dsrl32,
	instr_dsra32
};

static instr_fnc_t rt_map[32] = {
	instr_bltz,
	instr_bgez,
	instr_bltzl,
	instr_bgezl,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr_tgei,
	instr_tgeiu,
	instr_tlti,
	instr_tltiu,
	instr_teqi,
	instr__reserved,  /* unused */
	instr_tnei,
	instr__reserved,  /* unused */
	
	instr_bltzal,
	instr_bgezal,
	instr_bltzall,
	instr_bgezall,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop0_rs_map[32] = {
	instr_mfc0,
	instr_dmfc0,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr_mtc0,
	instr_dmtc0,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* cop0rsBC */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* cop0rsCO */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop1_rs_map[32] = {
	instr_mfc1,
	instr_dmfc1,
	instr_cfc1,
	instr__reserved,  /* unused */
	instr_mtc1,
	instr_dmtc1,
	instr_ctc1,
	instr__reserved,  /* unused */
	
	instr__reserved,  /* cop1rsBC */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop2_rs_map[32] = {
	instr_mfc2,
	instr__reserved,  /* unused */
	instr_cfc2,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr_ctc2,
	instr__reserved,  /* unused */
	
	instr__reserved,  /* cop2rsBC */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop0_rt_map[32] = {
	instr_bc0f,
	instr_bc0t,
	instr_bc0fl,
	instr_bc0tl,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop1_rt_map[32] = {
	instr_bc1f,
	instr_bc1t,
	instr_bc1fl,
	instr_bc1tl,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop2_rt_map[32] = {
	instr_bc2f,
	instr_bc2t,
	instr_bc2fl,
	instr_bc2tl,
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
	instr__reserved   /* unused */
};

static instr_fnc_t cop0_func_map[64] = {
	instr__warning,   /* unused */
	instr_tlbr,
	instr_tlbwi,
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr_tlbwr,
	instr__warning,   /* unused */
	
	instr_tlbp,
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr__reserved,  /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr_eret,
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning,   /* unused */
	instr__warning    /* unused */
};

static mnemonics_fnc_t mnemonics_opcode_map[64] = {
	/* 0 */
	mnemonics__reserved,  /* r4k_opcSPECIAL */
	mnemonics__reserved,  /* r4k_opcREGIMM */
	mnemonics_j,
	mnemonics_jal,
	mnemonics_beq,
	mnemonics_bne,
	mnemonics_blez,
	mnemonics_bgtz,
	
	/* 8 */
	mnemonics_addi,
	mnemonics_addiu,
	mnemonics_slti,
	mnemonics_sltiu,
	mnemonics_andi,
	mnemonics_ori,
	mnemonics_xori,
	mnemonics_lui,
	
	/* 16 */
	mnemonics__reserved,  /* r4k_opcCOP0 */
	mnemonics__reserved,  /* r4k_opcCOP1 */
	mnemonics__reserved,  /* r4k_opcCOP2 */
	mnemonics__reserved,  /* unused */
	mnemonics_beql,
	mnemonics_bnel,
	mnemonics_blezl,
	mnemonics_bgtzl,
	
	/* 24 */
	mnemonics_daddi,
	mnemonics_daddiu,
	mnemonics_ldl,
	mnemonics_ldr,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	/* 32 */
	mnemonics_lb,
	mnemonics_lh,
	mnemonics_lwl,
	mnemonics_lw,
	mnemonics_lbu,
	mnemonics_lhu,
	mnemonics_lwr,
	mnemonics_lwu,
	
	/* 40 */
	mnemonics_sb,
	mnemonics_sh,
	mnemonics_swl,
	mnemonics_sw,
	mnemonics_sdl,
	mnemonics_sdr,
	mnemonics_swr,
	mnemonics_cache,
	
	/* 48 */
	mnemonics_ll,
	mnemonics_lwc1,
	mnemonics_lwc2,
	mnemonics__reserved,  /* unused */
	mnemonics_lld,
	mnemonics_ldc1,
	mnemonics_ldc2,
	mnemonics_ld,
	
	mnemonics_sc,
	mnemonics_swc1,
	mnemonics_swc2,
	mnemonics__reserved,  /* unused */
	mnemonics_scd,
	mnemonics_sdc1,
	mnemonics_sdc2,
	mnemonics_sd
};

static mnemonics_fnc_t mnemonics_func_map[64] = {
	mnemonics_sll,
	mnemonics__reserved,  /* unused */
	mnemonics_srl,
	mnemonics_sra,
	mnemonics_sllv,
	mnemonics__reserved,  /* unused */
	mnemonics_srlv,
	mnemonics_srav,
	
	mnemonics_jr,
	mnemonics_jalr,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_syscall,
	mnemonics_break,
	mnemonics__xcrd,
	mnemonics_sync,
	
	mnemonics_mfhi,
	mnemonics_mthi,
	mnemonics_mflo,
	mnemonics_mtlo,
	mnemonics_dsllv,
	mnemonics__reserved,  /* unused */
	mnemonics_dsrlv,
	mnemonics_dsrav,
	
	mnemonics_mult,
	mnemonics_multu,
	mnemonics_div,
	mnemonics_divu,
	mnemonics_dmult,
	mnemonics_dmultu,
	mnemonics_ddiv,
	mnemonics_ddivu,
	
	mnemonics_add,
	mnemonics_addu,
	mnemonics_sub,
	mnemonics_subu,
	mnemonics_and,
	mnemonics_or,
	mnemonics_xor,
	mnemonics_nor,
	
	mnemonics__xhlt,
	mnemonics__xint,
	mnemonics_slt,
	mnemonics_sltu,
	mnemonics_dadd,
	mnemonics_daddu,
	mnemonics_dsub,
	mnemonics_dsubu,
	
	mnemonics_tge,
	mnemonics_tgeu,
	mnemonics_tlt,
	mnemonics_tltu,
	mnemonics_teq,
	mnemonics__xval,
	mnemonics_tne,
	mnemonics__xrd,
	
	mnemonics_dsll,
	mnemonics__xtrc,
	mnemonics_dsrl,
	mnemonics_dsra,
	mnemonics_dsll32,
	mnemonics__xtr0,
	mnemonics_dsrl32,
	mnemonics_dsra32
};

static mnemonics_fnc_t mnemonics_rt_map[32] = {
	mnemonics_bltz,
	mnemonics_bgez,
	mnemonics_bltzl,
	mnemonics_bgezl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics_tgei,
	mnemonics_tgeiu,
	mnemonics_tlti,
	mnemonics_tltiu,
	mnemonics_teqi,
	mnemonics__reserved,  /* unused */
	mnemonics_tnei,
	mnemonics__reserved,  /* unused */
	
	mnemonics_bltzal,
	mnemonics_bgezal,
	mnemonics_bltzall,
	mnemonics_bgezall,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop0_rs_map[32] = {
	mnemonics_mfc0,
	mnemonics_dmfc0,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_mtc0,
	mnemonics_dmtc0,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop0rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop0rsCO */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop1_rs_map[32] = {
	mnemonics_mfc1,
	mnemonics_dmfc1,
	mnemonics_cfc1,
	mnemonics__reserved,  /* unused */
	mnemonics_mtc1,
	mnemonics_dmtc1,
	mnemonics_ctc1,
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop1rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop2_rs_map[32] = {
	mnemonics_mfc2,
	mnemonics__reserved,  /* unused */
	mnemonics_cfc2,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_ctc2,
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* cop2rsBC */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop0_rt_map[32] = {
	mnemonics_bc0f,
	mnemonics_bc0t,
	mnemonics_bc0fl,
	mnemonics_bc0tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop1_rt_map[32] = {
	mnemonics_bc1f,
	mnemonics_bc1t,
	mnemonics_bc1fl,
	mnemonics_bc1tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop2_rt_map[32] = {
	mnemonics_bc2f,
	mnemonics_bc2t,
	mnemonics_bc2fl,
	mnemonics_bc2tl,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

static mnemonics_fnc_t mnemonics_cop0_func_map[64] = {
	mnemonics__reserved,  /* unused */
	mnemonics_tlbr,
	mnemonics_tlbwi,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics_tlbwr,
	mnemonics__reserved,  /* unused */
	
	mnemonics_tlbp,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics_eret,
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved,  /* unused */
	mnemonics__reserved   /* unused */
};

/** Decode MIPS R4000 instruction mnemonics
 *
 * @return Instruction mnemonics function.
 *
 */
mnemonics_fnc_t decode_mnemonics(r4k_instr_t instr)
{
	mnemonics_fnc_t fnc;
	
	/*
	 * Basic opcode decoding based
	 * on the opcode field.
	 */
	switch (instr.r.opcode) {
	case r4k_opcSPECIAL:
		/*
		 * SPECIAL opcode decoding based
		 * on the func field.
		 */
		fnc = mnemonics_func_map[instr.r.func];
		break;
	case r4k_opcREGIMM:
		/*
		 * REGIMM opcode decoding based
		 * on the rt field.
		 */
		fnc = mnemonics_rt_map[instr.r.rt];
		break;
	case r4k_opcCOP0:
		/*
		 * COP0 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop0rsBC:
			/*
			 * COP0/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = mnemonics_cop0_rt_map[instr.r.rt];
			break;
		case cop0rsCO:
			/*
			 * COP0/CO opcode decoding
			 * based on the 8-bit func field.
			 */
			fnc = mnemonics_cop0_func_map[instr.cop.func];
			break;
		default:
			fnc = mnemonics_cop0_rs_map[instr.r.rs];
		}
		break;
	case r4k_opcCOP1:
		/*
		 * COP1 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop1rsBC:
			/*
			 * COP1/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = mnemonics_cop1_rt_map[instr.r.rt];
			break;
		default:
			fnc = mnemonics_cop1_rs_map[instr.r.rs];
		}
		break;
	case r4k_opcCOP2:
		/*
		 * COP2 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.r.rs) {
		case cop2rsBC:
			/*
			 * COP2/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = mnemonics_cop2_rt_map[instr.r.rt];
			break;
		default:
			fnc = mnemonics_cop2_rs_map[instr.r.rs];
		}
		break;
	default:
		fnc = mnemonics_opcode_map[instr.r.opcode];
	}
	
	return fnc;
}

/** Initial state */
#define HARD_RESET_STATUS         (cp0_status_erl_mask | cp0_status_bev_mask)
#define HARD_RESET_START_ADDRESS  UINT64_C(0xffffffffbfc00000)
#define HARD_RESET_PROC_ID        UINT64_C(0x0000000000000400)
#define HARD_RESET_CAUSE          0
#define HARD_RESET_WATCHLO        0
#define HARD_RESET_WATCHHI        0
#define HARD_RESET_CONFIG         0
#define HARD_RESET_RANDOM         47
#define HARD_RESET_WIRED          0

/** Exception handling */
#define EXCEPTION_BOOT_BASE_ADDRESS     UINT64_C(0xffffffffbfc00200)
#define EXCEPTION_BOOT_RESET_ADDRESS    HARD_RESET_START_ADDRESS
#define EXCEPTION_NORMAL_BASE_ADDRESS   UINT64_C(0xffffffff80000000)
#define EXCEPTION_NORMAL_RESET_ADDRESS  HARD_RESET_START_ADDRESS
#define EXCEPTION_OFFSET                UINT64_C(0x0180)

/** Initialize simulation environment
 *
 */
void r4k_init(r4k_cpu_t *cpu, unsigned int procno)
{
	ASSERT(cpu != NULL);
	
	ptr64_t start_address;
	start_address.ptr = HARD_RESET_START_ADDRESS;
	
	/* Initially set all members to zero */
	memset(cpu, 0, sizeof(r4k_cpu_t));
	
	cpu->procno = procno;
	r4k_set_pc(cpu, start_address);
	
	/* Inicialize cp0 registers */
	cp0_config(cpu).val = HARD_RESET_CONFIG;
	cp0_random(cpu).val = HARD_RESET_RANDOM;
	cp0_wired(cpu).val = HARD_RESET_WIRED;
	cp0_prid(cpu).val = HARD_RESET_PROC_ID;
	
	/* Initial status value */
	cp0_status(cpu).val = HARD_RESET_STATUS;
	
	cp0_cause(cpu).val = HARD_RESET_CAUSE;
	cp0_watchlo(cpu).val = HARD_RESET_WATCHLO;
	cp0_watchhi(cpu).val = HARD_RESET_WATCHHI;
	
	/* Breakpoints */
	list_init(&cpu->bps);
}

/** Set the PC register
 *
 */
void r4k_set_pc(r4k_cpu_t *cpu, ptr64_t value)
{
	ASSERT(cpu != NULL);
	
	cpu->pc.ptr = value.ptr;
	cpu->pc_next.ptr = value.ptr + 4;
}

/** Read an instruction
 *
 * Does not change the value if an exception occurs.
 * Fetching instructions (on R4000) does not trigger
 * the WATCH exception.
 *
 */
//exc_t cpu_read_ins(r4k_cpu_t *cpu, ptr64_t addr, uint32_t *icode, bool noisy)
//{
//	ASSERT(cpu != NULL);
//	ASSERT(icode != NULL);
//	
//	exc_t res = align_test32(cpu, addr, noisy);
//	switch (res) {
//	case r4k_excNone:
//		break;
//	case r4k_excAddrError:
//		return r4k_excAdES;
//	default:
//		ASSERT(false);
//	}
//	
//	ptr36_t phys;
//	res = access_mem(cpu, AM_FETCH, addr, &phys, noisy);
//	switch (res) {
//	case r4k_excNone:
//		*icode = physmem_read32(cpu, phys, true);
//		break;
//	case r4k_excAddrError:
//		res = r4k_excAdEL;
//		break;
//	case r4k_excTLB:
//		res = r4k_excTLBL;
//		break;
//	case r4k_excTLBR:
//		res = r4k_excTLBLR;
//		break;
//	default:
//		ASSERT(false);
//	}
//	
//	if ((noisy) && (res != r4k_excNone) && (cpu->branch == BRANCH_NONE))
//		cpu->excaddr = cpu->pc;
//	
//	return res;
//}

/** Assert the specified interrupt
 *
 */
void r4k_interrupt_up(r4k_cpu_t *cpu, unsigned int no)
{
	ASSERT(cpu != NULL);
	ASSERT(no < INTR_COUNT);
	
	cp0_cause(cpu).val |= 1 << (cp0_cause_ip0_shift + no);
	cpu->intr[no]++;
}

/* Deassert the specified interrupt
 *
 */
void r4k_interrupt_down(r4k_cpu_t *cpu, unsigned int no)
{
	ASSERT(cpu != NULL);
	ASSERT(no < INTR_COUNT);
	
	cp0_cause(cpu).val &= ~(1 << (cp0_cause_ip0_shift + no));
}

/** Decode MIPS R4000 instruction
 *
 * @return Instruction implementation.
 *
 */
static instr_fnc_t decode(r4k_instr_t instr)
{
	instr_fnc_t fnc;

	/*
	 * Basic opcode decoding based
	 * on the opcode field.
	 */
	switch (instr.r.opcode) {
	case r4k_opcSPECIAL:
		/*
		 * SPECIAL opcode decoding based
		 * on the func field.
		 */
		fnc = func_map[instr.r.func];
		if(instr.r.func == 0x21){
			ASSERT(fnc == instr_addu);
		}
		break;
	case r4k_opcREGIMM:
		/*
		 * REGIMM opcode decoding based
		 * on the rt field.
		 */
		fnc = rt_map[instr.r.rt];
		break;
	case r4k_opcCOP0:
		/*
		 * COP0 opcode decoding based
		 * on the rs field.
		 */
		// should be based on COP format
		switch (instr.cop.rs) {
		case cop0rsBC:
			/*
			 * COP0/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop0_rt_map[instr.cop.rt];
			break;
		case cop0rsCO:
			/*
			 * COP0/CO opcode decoding
			 * based on the 8-bit func field.
			 */
			fnc = cop0_func_map[instr.cop.func];
			break;
		default:
			fnc = cop0_rs_map[instr.cop.rs];
		}
		break;
	case r4k_opcCOP1:
		/*
		 * COP1 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.cop.rs) {
		case cop1rsBC:
			/*
			 * COP1/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop1_rt_map[instr.cop.rt];
			break;
		default:
			fnc = cop1_rs_map[instr.cop.rs];
		}
		break;
	case r4k_opcCOP2:
		/*
		 * COP2 opcode decoding based
		 * on the rs field.
		 */
		switch (instr.cop.rs) {
		case cop2rsBC:
			/*
			 * COP2/BC opcode decoding
			 * based on the rt field.
			 */
			fnc = cop2_rt_map[instr.cop.rt];
			break;
		default:
			fnc = cop2_rs_map[instr.cop.rs];
		}
		break;
	default:
		fnc = opcode_map[instr.r.opcode];
	}
	
	return fnc;
}

/** Translate instruction virtual address to physical memory frame
 *
 */
static r4k_exc_t cpu_frame(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	ptr64_t virt;
	virt.ptr = cpu->pc.ptr;
	virt.lo &= ~((uint32_t) FRAME_MASK);
	
	ptr36_t phys;
	r4k_exc_t res = r4k_convert_addr(cpu, virt, &phys, false, true);
	switch (res) {
	case r4k_excNone:
		break;
	case r4k_excAddrError:
		if (cpu->branch == BRANCH_NONE)
			cpu->excaddr = cpu->pc;
		return r4k_excAdEL;
	case r4k_excTLB:
		if (cpu->branch == BRANCH_NONE)
			cpu->excaddr = cpu->pc;
		return r4k_excTLBL;
	case r4k_excTLBR:
		if (cpu->branch == BRANCH_NONE)
			cpu->excaddr = cpu->pc;
		return r4k_excTLBLR;
	default:
		ASSERT(false);
	}
	
	cpu->frame = physmem_find_frame(phys);
	if (cpu->frame == NULL) {
		alert("Trying to fetch instructions from outside of physical memory");
		return r4k_excAdEL;
	}
	
	return r4k_excNone;
}

/** Decode instructions in a physical memory frame
 *
 */
static void frame_decode(frame_t *frame, instr_fnc_t* out)
{
	ASSERT(frame != NULL);
	// Remove when caching is back
	//ASSERT(!frame->valid);
	
	unsigned int i;
	for (i = 0; i < ADDR2INSTR(FRAME_SIZE); i++) {
		r4k_instr_t instr = *((r4k_instr_t *) (frame->data + INSTR2ADDR(i)));
		*(out + i) = decode(instr);
	}
	
	frame->valid = true;
}

/** Change the processor state according to the exception type
 *
 */
static void handle_exception(r4k_cpu_t *cpu, r4k_exc_t res)
{
	ASSERT(cpu != NULL);

	bool tlb_refill = false;
	
	/* Convert TLB Refill exceptions */
	if ((res == r4k_excTLBLR) || (res == r4k_excTLBSR)) {
		tlb_refill = true;
		if (res == r4k_excTLBLR)
			res = r4k_excTLBL;
		else
			res = r4k_excTLBS;
	}
	
	ASSERT(res <= r4k_excVCED);
	
	/* The standby mode is cancelled by the exception */
	if (cpu->stdby)
		r4k_set_pc(cpu, cpu->pc_next);
	
	cpu->stdby = false;
	
	/* User info and register fill */
	if (machine_trace) {
		if (tlb_refill)
			alert("cpu%u raised TLB refill exception %u: %s", cpu->procno,
			    res, txt_exc[res]);
		else
			alert("cpu%u raised exception %u: %s", cpu->procno,
			    res, txt_exc[res]);
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_exccode_mask;
	cp0_cause(cpu).val |= res << cp0_cause_exccode_shift;
	
	/* Exception branch control */
	cp0_cause(cpu).val &= ~cp0_cause_bd_mask;
	if (cpu->branch == BRANCH_PASSED)
		cp0_cause(cpu).val |= cp0_cause_bd_mask;
	
	if (!cp0_status_exl(cpu)) {
		cp0_epc(cpu).val = cpu->excaddr.ptr;
		if ((res == r4k_excInt) && (cpu->branch != BRANCH_COND))
			cp0_epc(cpu).val = cpu->pc.ptr;
	}
	
	ptr64_t exc_pc;
	/* Exception vector base address */
	if (cp0_status_bev(cpu)) {
		/* Boot time */
		if (res != r4k_excReset)
			exc_pc.ptr = EXCEPTION_BOOT_BASE_ADDRESS;
		else
			exc_pc.ptr = EXCEPTION_BOOT_RESET_ADDRESS;
	} else {
		/* Normal time */
		if (res != r4k_excReset)
			exc_pc.ptr = EXCEPTION_NORMAL_BASE_ADDRESS;
		else
			exc_pc.ptr = EXCEPTION_NORMAL_RESET_ADDRESS;
	}
	
	/* Exception vector offsets */
	if ((cp0_status_exl(cpu)) || (!tlb_refill))
		exc_pc.ptr += EXCEPTION_OFFSET;
	
	r4k_set_pc(cpu, exc_pc);
	
	/* Switch to kernel mode */
	cp0_status(cpu).val |= cp0_status_exl_mask;
}

/** Execute one CPU instruction
 *
 */
static r4k_exc_t execute(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	/* Binary translation */
	// TODO: do this in a similar way to RISC
	if (cpu->frame == NULL) {
		r4k_exc_t res;
		
		do {
			res = cpu_frame(cpu);
			if (res != r4k_excNone)
				handle_exception(cpu, res);
		} while (res != r4k_excNone);

		// decode everytime, because we are not caching
		frame_decode(cpu->frame, cpu->trans);
	}

	// decode if frame is invalid
	if(!cpu->frame->valid){
		frame_decode(cpu->frame, cpu->trans);
	}
	
	/* Fetch decoded instruction */
	unsigned int i = cpu->pc.ptr & FRAME_MASK;
	r4k_instr_t instr = *((r4k_instr_t *) (cpu->frame->data + i));
	instr_fnc_t fnc = *(cpu->trans + ADDR2INSTR(i));

	/* Execute instruction */
	r4k_exc_t exc = fnc(cpu, instr);
	
	if (machine_trace)
		r4k_idump(cpu, cpu->pc, instr, true);
	
	/* Branch test */
	if ((cpu->branch == BRANCH_COND) || (cpu->branch == BRANCH_NONE))
		cpu->excaddr.ptr = cpu->pc.ptr;
	
	/* Register 0 contains a hardwired zero value */
	cpu->regs[0].val = 0;
	
	/* PC update */
	if (exc == r4k_excJump) {
		/*
		 * Execute the instruction in the branch
		 * delay slot. The jump target is stored
		 * in pc_next.
		 */
		exc = r4k_excNone;
		cpu->pc.ptr += 4;
	} else {
		/*
		 * Advance to the next instruction
		 * as usual.
		 */
		cpu->pc.ptr = cpu->pc_next.ptr;
		cpu->pc_next.ptr += 4;
	}
	
	return exc;
}

/** CPU management
 *
 */
static void manage(r4k_cpu_t *cpu, r4k_exc_t exc, ptr64_t old_pc)
{
	ASSERT(cpu != NULL);
	
	/* Test for interrupt request */
	if ((exc == r4k_excNone) &&
	    (!cp0_status_exl(cpu)) &&
	    (!cp0_status_erl(cpu)) &&
	    (cp0_status_ie(cpu)) &&
	    ((cp0_cause(cpu).val & cp0_status(cpu).val) & cp0_cause_ip_mask) != 0)
		exc = r4k_excInt;
	
	/* Exception control */
	if (exc != r4k_excNone)
		handle_exception(cpu, exc);
	
	/* Increase counter */
	cp0_count(cpu).val++;
	
	/* Decrease random register */
	if (cp0_random(cpu).val-- == 0)
		cp0_random(cpu).val = 47;
	
	if (cp0_random(cpu).val < cp0_wired(cpu).val)
		cp0_random(cpu).val = 47;
	
	/*
	 * Timer control.
	 *
	 * N.B.: Count and Compare are truly 32 bit CP0
	 *       registers even in 64-bit mode.
	 */
	if (cp0_count(cpu).lo == cp0_compare(cpu).lo)
		/* Generate interrupt request */
		cp0_cause(cpu).val |= 1 << cp0_cause_ip7_shift;
	
	/* Branch delay slot control */
	if (cpu->branch > BRANCH_NONE)
		cpu->branch--;
	
	/*
	 * Reset the binary translation if we are outside
	 * the original frame.
	 */
	if ((old_pc.ptr | FRAME_MASK) != (cpu->pc.ptr | FRAME_MASK)){
		//TODO: remove when cached decode is back
		cpu->frame->valid = false;
		cpu->frame = NULL;
	}
}

/** CPU cycle accounting after one instruction execution
 *
 */
static void account(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	if (cpu->stdby) {
		cpu->w_cycles++;
	} else {
		if (CPU_KERNEL_MODE(cpu))
			cpu->k_cycles++;
		else
			cpu->u_cycles++;
	}
}

/* Simulate one cycle of the processor
 *
 */
void r4k_step(r4k_cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	/* Instruction execute */
	r4k_exc_t exc = r4k_excNone;
	ptr64_t old_pc = cpu->pc;
	
	if (!cpu->stdby)
		exc = execute(cpu);
	
	/* Processor management */
	manage(cpu, exc, old_pc);
	
	/* Cycle accounting */
	account(cpu);
}

bool r4k_sc_access(r4k_cpu_t *cpu, ptr36_t addr) {
	bool hit = cpu->lladdr == ALIGN_DOWN(addr, 4);
	if(hit){
		cpu->llbit = false;
	}
	return hit;
}
