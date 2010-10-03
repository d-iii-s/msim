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

#include <stdlib.h>
#include <stdio.h>

#include "../check.h"

#include "../main.h"
#include "../mtypes.h"
#include "../endi.h"
#include "cpu.h"
#include "../device/machine.h"
#include "../debug/debug.h"
#include "../debug/breakpoint.h"
#include "../text.h"
#include "../io/output.h"
#include "../debug/gdb.h"
#include "../env.h"
#include "../fault.h"


/** Initial state */
#define HARD_RESET_STATUS         (cp0_status_erl_mask | cp0_status_bev_mask)
#define HARD_RESET_START_ADDRESS  0xbfc00000
#define HARD_RESET_PROC_ID        0x00000400
#define HARD_RESET_CAUSE          0
#define HARD_RESET_WATCHLO        0
#define HARD_RESET_WATCHHI        0
#define HARD_RESET_CONFIG         0
#define HARD_RESET_RANDOM         47
#define HARD_RESET_WIRED          0

#define EXCEPTION_OFFSET  0x180

#define TRAP(x) \
	if (x) \
		res = excTr;

/** TLB lookup result */
typedef enum {
	TLBL_OK,
	TLBL_REFILL,
	TLBL_INVALID,
	TLBL_MODIFIED
} tlb_look_t;

/** Memory access mode */
typedef enum {
	AM_FETCH,
	AM_READ,
	AM_WRITE
} acc_mode_t;

/** Initialize simulation environment
 *
 */
void cpu_init(cpu_t *cpu, size_t procno)
{
	/* Initially set all members to zero */
	memset(cpu, 0, sizeof(cpu_t));
	
	cpu->procno = procno;
	cpu_set_pc(HARD_RESET_START_ADDRESS);
	
	/* Inicialize cp0 registers */
	cpu->cp0[cp0_Config] = HARD_RESET_CONFIG;
	cpu->cp0[cp0_Random] = HARD_RESET_RANDOM;
	cpu->cp0[cp0_Wired] = HARD_RESET_WIRED;
	cpu->cp0[cp0_PRId] = HARD_RESET_PROC_ID;
	
	/* Initial status value */
	cpu->cp0[cp0_Status] = HARD_RESET_STATUS;
	
	cpu->cp0[cp0_Cause] = HARD_RESET_CAUSE;
	cpu->cp0[cp0_WatchLo] = HARD_RESET_WATCHLO;
	cpu->cp0[cp0_WatchHi] = HARD_RESET_WATCHHI;
	
	/* Breakpoints */
	list_init(&cpu->bps);
}

/** Set the PC register
 *
 */
void cpu_set_pc(cpu_t *cpu, ptr_t value)
{
	cpu->pc = value;
	cpu->pc_next = value + 4;
}

/** Address traslation through the TLB table
 *
 * See tlb_look_t definition
 *
 */
static tlb_look_t tlb_look(cpu_t *cpu, ptr_t *addr, bool wr)
{
	/* Ignore TLB test */
	if (cp0_status_ts == 1)
		return TLBL_OK;
	
	unsigned int hint = cpu->tlb_hint;
	unsigned int i;
	
	/* Look for the TBL hit */
	for (i = 0; i < TLB_ENTRIES; i++) {
		tlb_entry_t *entry = &cpu->tlb[(i + hint) % TLB_ENTRIES];
		
		/* TLB hit? */
		if ((*addr & entry->mask) == entry->vpn2) {
			/* Test ASID */
			if ((!entry->global) && (entry->asid != cp0_entryhi_asid))
				continue;
			
			/* Calculate subpage */
			uint32_t smask = (entry->mask >> 1) | SBIT;
			unsigned int subpage =
			    ((*addr & entry->mask) < (*addr & smask)) ? 1 : 0;
			
			/* Test valid & dirty */
			if (!entry->pg[subpage].valid)
				return TLBL_INVALID;
			
			if ((wr) && (!entry->pg[subpage].dirty))
				return TLBL_MODIFIED;
			
			/* Make address */
			ptr_t amask = *addr & (~smask);
			*addr = amask | (cpu->tlb[i].pg[subpage].pfn & smask);
			
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
static void fill_tlb_error(cpu_t *cpu, ptr_t addr)
{
	cp0_badvaddr(cpu) = addr;
	
	cp0_context(cpu) &= cp0_context_ptebase_mask;
	cp0_context(cpu) |= (addr >> cp0_context_addr_shift) & ~cp0_context_res1_mask;
	
	cp0_entryhi(cpu) &= cp0_entryhi_asid_mask;
	cp0_entryhi(cpu) |= addr & cp0_entryhi_vpn2_mask;
}

/** Fill registers as the Address error exception occures
 *
 */
static void fill_addr_error(cpu_t *cpu, ptr_t addr, bool noisy)
{
	if (noisy) {
		cp0_badvaddr(cpu) = addr;
		cp0_context(cpu) &= ~cp0_context_badvpn2_mask;  /* Undefined */
		cp0_entryhi(cpu) &= ~cp0_entryhi_vpn2_mask;     /* Undefined */
	}
}

/** Search through TLB and generates apropriate exception
 *
 */
static exc_t tlb_hit(cpu_t *cpu, ptr_t *addr, bool wr, bool noisy)
{
	switch (tlb_look(cpu, addr, wr)) {
	case TLBL_OK:
		break;
	case TLBL_REFILL:
		if (noisy) {
			cpu->tlb_refill++;
			fill_tlb_error(cpu, *addr);
		}
		
		return excTLBR;
	case TLBL_INVALID:
		if (noisy) {
			cpu->tlb_invalid++;
			fill_tlb_error(cpu, *addr);
		}
		
		return excTLB;
	case TLBL_MODIFIED:
		if (noisy) {
			pr->tlb_modified++;
			fill_tlb_error(cpu, *addr);
		}
		
		return excMod;
	}
	
	return excNone;
}

/** The user mode address conversion
 *
 */
static exc_t convert_addr_user(cpu_t *cpu, ptr_t *addr, bool wr, bool noisy)
{
	/* Testi 31 bit or look into TLB */
	if ((*addr & SBIT) != 0) {
		fill_addr_error(cpu, *addr, noisy);
		return excAddrError;
	}
	
	/* useg */
	return tlb_hit(cpu, addr, wr, noisy);
}

/** The supervisor mode address conversion
 *
 */
static exc_t convert_addr_supervisor(cpu_t *cpu, ptr_t *addr, bool wr,
    bool noisy)
{
	if (*addr < 0x80000000)  /* suseg */
		return tlb_hit(cpu, addr, wr, noisy);
	
	if (*addr < 0xc0000000) {
		fill_addr_error(cpu, *addr, noisy);
		return excAddrError;
	}
	
	if (*addr < 0xe0000000)  /* ssseg */
		return tlb_hit(cpu, addr, wr, noisy);
	
	/* *addr > 0xe0000000 */
	fill_addr_error(cpu, *addr, noisy);
	return excAddrError;
}

/** The kernel mode address conversion
 *
 */
static exc_t convert_addr_kernel(cpu_t *cpu, ptr_t *addr, bool wr, bool noisy)
{
	if (*addr < 0x80000000) {  /* kuseg */
		if (!cp0_status_erl(cpu))
			return tlb_hit(cpu, addr, wr, noisy);
		
		return excNone;
	}
	
	if (*addr < 0xa0000000) {  /* kseg0 */
		*addr -= 0x80000000;
		return excNone;
	}
	
	if (*addr < 0xc0000000) {  /* kseg1 */
		*addr -= 0xa0000000;
		return excNone;
	}
	
	if (*addr < 0xe0000000)  /* kseg2 */
		return tlb_hit(cpu, addr, wr, noisy);
	
	/* *addr > 0xe0000000 (kseg3) */
	return tlb_hit(cpu, addr, wr, noisy);
}

/** The conversion of virtual addresses
 *
 * @param write Write access.
 * @param noisy Fill apropriate processor registers
 *              if the address is incorrect.
 *
 */
exc_t convert_addr(cpu_t *cpu, ptr_t *addr, bool write, bool noisy)
{
	/* Test the type of the translation */
	if ((cp0_status_ksu(cpu) == 2) && (!cp0_status_exl(cpu)) &&
	    (!cp0_status_erl(cpu)))
		return convert_addr_user(cpu, addr, write, noisy);
	
	if ((cp0_status_ksu(cpu) == 1) && (!cp0_status_exl(cpu)) &&
	    (!cp0_status_erl(cpu)))
		return convert_addr_supervisor(cpu, addr, write, noisy);
	
	return convert_addr_kernel(cpu, addr, write, noisy);
}

/** Test for correct alignment
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static exc_t mem_align_test(cpu_t *cpu, ptr_t addr, len_t size, bool noisy)
{
	if (((size == 2) && (addr & 1)) || ((size == 4) && (addr & 3))) {
		fill_addr_error(cpu, addr, noisy);
		return excAddrError;
	}
	
	return excNone;
}

/** Access the virtual memory
 *
 * The operation (read/write) is specified via the wr parameter.
 * This routine does not specify the exception type.
 *
 * If the exception occures, the value does not change.
 *
 * @param mode  Memory access mode
 * @param addr  Memory address
 * @param size  Argument size in bytes (1, 2, 4)
 * @param value The value which has to be written/read to
 * @param noisy Generate exception in case of invalid operation
 *
 */
static exc_t acc_mem(cpu_t *cpu, acc_mode_t mode, ptr_t addr, len_t size,
    uint32_t *value, bool noisy)
{
	exc_t res = mem_align_test(cpu, addr, size, noisy);
	
	if (res == excNone) {
		res = convert_addr(cpu, &addr, mode == AM_WRITE, noisy);
		
		/* Check for watched address */
		if (((cp0_watchlo_r(cpu)) && (mode == AM_READ))
		    || ((cp0_watchlo_w(cpu)) && (mode == AM_WRITE))) {
			
			/* The matching is done on
			   8-byte aligned addresses */
			
			if (cpu->waddr == (addr >> 3)) {
				/*
				 * If EXL is set, the exception has to be postponed,
				 * the memory access should (probably) proceed.
				 */
				if (cp0_status_exl == 1) {
					cpu->wpending = true;
					cpu->wexcaddr = cpu->pc;
				} else
					return excWATCH;
			}
		}
		
		if (res == excNone) {
			if (mode == AM_WRITE)
				mem_write(cpu, addr, *value, size, true);
			else
				*value = mem_read(pr, addr, size, true);
		}
	}
	
	return res;
}

/** Preform the read access from the virtual memory
 *
 * Does not change the value if an exception occurs.
 *
 */
exc_t cpu_read_mem(cpu_t *cpu, ptr_t addr, len_t size, uint32_t *value,
    bool noisy)
{
	switch (acc_mem(cpu, AM_READ, addr, size, value, noisy)) {
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	case excWATCH:
		return excWATCH;
	default:
		break;
	}
	
	return excNone;
}

/** Preform the fetch access from the virtual memory
 *
 * Does not change the value if an exception occurs.
 * Fetching instructions (on R4000) does not trigger
 * the WATCH exception.
 *
 */
static exc_t cpu_fetch_mem(cpu_t *cpu, ptr_t addr, len_t size, uint32_t *value,
    bool noisy)
{
	switch (acc_mem(pr, AM_FETCH, addr, size, value, noisy)) {
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	default:
		break;
	}
	
	return excNone;
}

/** Perform the write operation to the virtual memory
 *
 */
static exc_t cpu_write_mem(cpu_t *cpu, ptr_t addr, len_t size, uint32_t value,
    bool noisy)
{
	switch (acc_mem(cpu, AM_WRITE, addr, size, &value, noisy)) {
	case excAddrError:
		return excAdES;
	case excTLB:
		return excTLBS;
	case excTLBR:
		return excTLBSR;
	case excMod:
		return excMod;
	case excWATCH:
		return excWATCH;
	case excNone:
		return excNone;
	default:
		die(ERR_INTERN, "Internal error at %s(%u)", __FILE__, __LINE__);
	}
	
	/* Unreachable */
	return excNone;
}

/** Read an instruction
 *
 */
exc_t cpu_read_ins(cpu_t *cpu, ptr_t addr, uint32_t *value, bool noisy)
{
	exc_t res = cpu_fetch_mem(cpu, addr, BITS_32, value, noisy);
	if ((res != excNone) && (pr->branch == BRANCH_NONE))
		cpu->excaddr = cpu->pc;
	
	return res;
}

/** Assert the specified interrupt
 *
 */
void cpu_interrupt_up(cpu_t *cpu, unsigned int no)
{
	PRE(no < INTR_COUNT);
	
	cp0_cause(cpu) |= 1 << (cp0_cause_ip0_shift + no);
	cpu->intr[no]++;
}

/* Deassert the specified interrupt
 *
 */
void proc_interrupt_down(cpu_t *cpu, unsigned int no)
{
	PRE(no < INTR_COUNT);
	
	cp0_cause(cpu) &= ~(1 << (cp0_cause_ip0_shift + no));
}

/** Update the copy of registers
 *
 */
void cpu_update_debug(cpu_t *cpu)
{
	memcpy(cpu->old_regs, cpu->regs, sizeof(cpu->regs));
	memcpy(cpu->old_cp0, cpu->cp0, sizeof(cpu->cp0));
	
	pr->old_loreg = pr->loreg;
	pr->old_hireg = pr->hireg;
}

/** Perform the multiplication of two integers
 *
 */
static void multiply(cpu_t *cpu, uint32_t a, uint32_t b, bool sign)
{
	/* Quick test */
	if ((a == 0) || (b == 0)) {
		pr->hireg = 0;
		pr->loreg = 0;
		return;
	}
	
	if (sign) {
		/* Signed multiplication */
		int64_t r = ((int64_t) ((int32_t) a)) * ((int64_t) ((int32_t) b));
		
		/* Result */
		pr->loreg = ((uint64_t) r) & 0xffffffff;
		pr->hireg = ((uint64_t) r) >> 32;
	} else {
		/* Unsigned multiplication */
		uint64_t r = ((uint64_t) a) * ((uint64_t) b);
		
		/* Result */
		pr->loreg = r & 0xffffffff;
		pr->hireg = r >> 32;
	}
}

/** Write a new entry into the TLB
 *
 * The entry index is determined by either
 * the random register (TLBWR) or index (TLBWI).
 *
 */
static void TLBW(cpu_t *cpu, bool random, exc_t *res)
{
	if ((cp0_status_cu0 == 1)
	    || ((cp0_status_ksu == 0)
	    || (cp0_status_exl == 1)
	    || (cp0_status_erl == 1))) {
		
		unsigned int index =
		    random ? cp0_random_random(cpu) : cp0_index_index(cpu);
		
		if (index > 47) {
			/*
			 * Undefined behavior, doing nothing complies.
			 * Random is read-only, its index should be always fine.
			 */
			mprintf("\nTLBWI: Invalid value in Index\n");
		} else {
			/* Fill TLB */
			tlb_ent_t *entry = &cpu->tlb[index];
			
			entry->mask = cp0_entryhi_vpn2_mask & ~cp0_pagemask;
			entry->vpn2 = cp0_entryhi & entry->mask;
			entry->global = cp0_entrylo0_g & cp0_entrylo1_g;
			entry->asid = cp0_entryhi_asid;
			
			entry->pg[0].pfn = cp0_entrylo0_pfn << 12;
			entry->pg[0].cohh = cp0_entrylo0_c;
			entry->pg[0].dirty = cp0_entrylo0_d;
			entry->pg[0].valid = cp0_entrylo0_v;
			
			entry->pg[1].pfn = cp0_entrylo1_pfn << 12;
			entry->pg[1].cohh = cp0_entrylo1_c;
			entry->pg[1].dirty = cp0_entrylo1_d;
			entry->pg[1].valid = cp0_entrylo1_v;
		}
	} else {
		/* Coprocessor unusable */
		*res = excCpU;
		entry->cp0[cp0_Cause] &= ~cp0_cause_ce_mask;
	}
}

/** Execute the instruction specified by the opcode
 *
 * A really huge one, isn't it.
 *
 */
static exc_t execute(cpu_t *cpu, instr_info_t *ii2)
{
	instr_info_t ii = *ii2;
	exc_t res = excNone;
	uint32_t pca = pr->pc_next + 4;
	int32_t rrs = pr->regs[ii.rs];
	int32_t rrt = pr->regs[ii.rt];
	
	switch (ii.opcode) {
	
	/*
	 * aritmetic, logic, shifts
	 */
	
	case opcADD:
		{
			int32_t i1;
			i1 = rrs + rrt;
			if (!((rrs ^ rrt) & SBIT) && ((rrs ^ i1) & SBIT)) {
				res = excOv;
				break;
			}
			pr->regs[ii.rd] = i1;
		}
		break;
	case opcADDI:
		{
			int32_t i1;
			i1 = rrs + ii.imm;
			if (!( (rrs ^ ii.imm) & SBIT) && ((ii.imm ^ i1) & SBIT)) {
				res = excOv;
				break;
			}
			pr->regs[ii.rt] = i1;
		}
		break;
	case opcADDIU:
		pr->regs[ii.rt] = rrs + ii.imm;
		break;
	case opcADDU:
		pr->regs[ii.rd] = rrs + rrt;
		break;
	case opcAND:
		pr->regs[ii.rd] = rrs & rrt;
		break;
	case opcANDI:
		pr->regs[ii.rt] = rrs & (ii.imm & 0xffff);
		break;
	case opcCLO:
		{
			int32_t i = 0;
			uint32_t tmp = rrs;
			
			while ((tmp & 0x80000000u) && i < 32) {
				i++;
				tmp <<= 1;
			}
			
			pr->regs[ii.rd] = i;
		}
		break;
	case opcCLZ:
		{
			int32_t i = 0;
			uint32_t tmp = rrs;
			
			while (!(tmp & 0x80000000u) && i < 32) {
				i++;
				tmp <<= 1;
			}
			
			pr->regs[ii.rd] = i;
		}
		break;
	case opcDADD:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDADDI:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDADDIU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDADDU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDDIV:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDDIVU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDIV:
		if (rrt == 0) {
			pr->loreg = 0;
			pr->hireg = 0;
		} else {
			pr->loreg = rrs / rrt;
			pr->hireg = rrs % rrt;
		}
		break;
	case opcDIVU:
		{
			uint32_t dw1 = (uint32_t) rrs;
			uint32_t dw2 = (uint32_t) rrt;
			if (dw2 == 0) {
				pr->loreg = 0;
				pr->hireg = 0; 
			} else {
				pr->loreg = (int32_t) (dw1 / dw2); 
				pr->hireg = (int32_t) (dw1 % dw2); 
			}
		}
		break;
	case opcDMULT:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDMULTU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSLL:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSLLV:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSLL32:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRA:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRAV:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRA32:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRL:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRLV:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSRL32:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSUB:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDSUBU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcMADD:
		{
			uint64_t old = ((uint64_t) pr->hireg << 32) | pr->loreg;
			
			multiply(pr, rrs, rrt, true); 
			old += ((uint64_t) pr->hireg << 32) | pr->loreg;
			pr->hireg = old >> 32;
			pr->loreg = old & 0xffffffff;
		}
		break;
	case opcMADDU:
		{
			uint64_t old = ((uint64_t) pr->hireg << 32) | pr->loreg;
			
			multiply(pr, rrs, rrt, false); 
			old += ((uint64_t) pr->hireg << 32) | pr->loreg;
			pr->hireg = old >> 32;
			pr->loreg = old & 0xffffffff;
		}
		break;
	case opcMSUB:
		{
			uint64_t old = ((uint64_t) pr->hireg << 32) | pr->loreg;
			
			multiply(pr, rrs, rrt, true); 
			old -= ((uint64_t) pr->hireg << 32) | pr->loreg;
			pr->hireg = old >> 32;
			pr->loreg = old & 0xffffffff;
		}
		break;
	case opcMSUBU:
		{
			uint64_t old = ((uint64_t) pr->hireg << 32) | pr->loreg;
			
			multiply(pr, rrs, rrt, false); 
			old -= ((uint64_t) pr->hireg << 32) | pr->loreg;
			pr->hireg = old >> 32;
			pr->loreg = old & 0xffffffff;
		}
		break;
	case opcMUL:
		{
			uint64_t res = rrs * rrt;
			pr->regs[ii.rd] = res & 0xffffffff;
		}
		break;
	case opcMOVN:
		if (rrt != 0)
			pr->regs[ii.rd] = rrs;
		break;
	case opcMOVZ:
		if (rrt == 0)
			pr->regs[ii.rd] = rrs;
		break;
	case opcMULT:
		multiply(pr, rrs, rrt, true);
		break;
	case opcMULTU:
		multiply(pr, rrs, rrt, false);
		break;
	case opcNOR:
		pr->regs[ii.rd] = ~(rrs | rrt);
		break;
	case opcOR:
		pr->regs[ii.rd] = rrs | rrt;
		break;
	case opcORI:
		pr->regs[ii.rt] = rrs | (ii.imm & 0xffff);
		break;
	case opcSLL:
		pr->regs[ii.rd] = (uint32_t) rrt << ii.shift;
		break;
	case opcSLLV:
		pr->regs[ii.rd] = (uint32_t) rrt << (rrs & 0x1f);
		break;
	case opcSLT:
		pr->regs[ii.rd] = rrs < rrt;
		break;
	case opcSLTI:
		pr->regs[ii.rt] = rrs < ii.imm;
		break;
	case opcSLTIU:
		pr->regs[ii.rt] = (uint32_t) rrs < (uint32_t) ii.imm;
		break;
	case opcSLTU:
		pr->regs[ii.rd] = (uint32_t) rrs < (uint32_t) rrt;
		break;
	case opcSRA:
		pr->regs[ii.rd] = rrt >> ii.shift;
		break;
	case opcSRAV:
		pr->regs[ii.rd] = rrt >> (rrs & 0x1f);
		break;
	case opcSRL:
		pr->regs[ii.rd] = (uint32_t) rrt >> ii.shift;
		break;
	case opcSRLV:
		pr->regs[ii.rd] = (uint32_t) rrt >> (rrs & 0x1f);
		break;
	case opcSUB:
		{
			uint32_t i = rrs - rrt;
			if (((rrs ^ rrt) & SBIT) && ((rrs ^ i) & SBIT)) {
				res = excOv;
				break;
			}
			pr->regs[ii.rd] = i;
		}
		break;
	case opcSUBU:
		pr->regs[ii.rd] = rrs - rrt;
		break;
	case opcXOR:
		pr->regs[ii.rd] = rrs ^ rrt;
		break;
	case opcXORI:
		pr->regs[ii.rt] = rrs ^ (ii.imm & 0xffff);
		break;
	
	/*
	 * branches and jumps
	 */
	case opcBC0FL:
	case opcBC1FL:
	case opcBC2FL:
	case opcBC3FL:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl == 1)
		    || (cp0_status_erl == 1))) {
			/* Ignore - always false */
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0F:
	case opcBC1F:
	case opcBC2F:
	case opcBC3F:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl == 1)
		    || (cp0_status_erl == 1))) {
			/* Ignore - always false */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0TL:
	case opcBC1TL:
	case opcBC2TL:
	case opcBC3TL:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl == 1)
		    || (cp0_status_erl == 1))) {
			/* Ignore - always true */
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0T:
	case opcBC1T:
	case opcBC2T:
	case opcBC3T:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl == 1)
		    || (cp0_status_erl == 1))) {
			/* Ignore - always true */
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBEQ:
		if (rrs == rrt) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBEQL:
		if (rrs == rrt) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcBGEZAL:
		pr->regs[31] = pr->pc + 8;
		/* no break */
	case opcBGEZ:
		if (!(rrs & SBIT)) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBGEZALL:
		pr->regs[31] = pr->pc + 8;
		/* no break */
	case opcBGEZL:
		if (!(rrs & SBIT)) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcBGTZ:
		if (rrs > 0) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBGTZL:
		if (rrs > 0) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcBLEZ:
		if (rrs <= 0) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBLEZL:
		if (rrs <= 0) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcBLTZAL:
		pr->regs[31] = pr->pc_next + 4;
		/* no break */
	case opcBLTZ:
		if (rrs & SBIT) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBLTZALL:
		pr->regs[31] = pr->pc_next + 4;
		/* no break */
	case opcBLTZL:
		if (rrs & SBIT) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcBNE:
		if (rrs != rrt) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		}
		break;
	case opcBNEL:
		if (rrs != rrt) {
			pca = pr->pc_next + (ii.imm << 2);
			pr->branch = BRANCH_COND;
		} else {
			pr->pc_next += 4;
			pca = pr->pc_next + 4;
		}
		break;
	case opcJAL:
		pr->regs[31] = pr->pc_next + 4;
		/* no break */
	case opcJ:
		pca = (pr->pc_next & TARGET_COMB) | ii.imm;
		pr->branch = BRANCH_COND;
		break;
	case opcJALR:
		pr->regs[ii.rd] = pr->pc_next + 4;
		/* no break */
	case opcJR:
		pca = rrs;
		pr->branch = BRANCH_COND;
		break;
	
	/*
	 * load, store
	 */
	case opcLB:
		{
			uint32_t tmp;
			res = read_proc_mem(pr, rrs + ii.imm, BITS_8, &tmp, true);
			if (res == excNone)
				pr->regs[ii.rt] = (tmp & 0x80) ?
					(tmp | 0xffffff00) :
					(tmp & 0xff);
		}
		break;
	case opcLBU:
		{
			uint32_t tmp;
			res = read_proc_mem(pr, rrs + ii.imm, BITS_8, &tmp, true);
			if (res == excNone)
				pr->regs[ii.rt] = tmp & 0xff;
		}
		break;
	case opcLD:
		res = excRI;
		break;
	case opcLDL:
		res = excRI;
		break;
	case opcLDR:
		res = excRI;
		break;
	case opcLH:
		{
			uint32_t tmp;
			res = read_proc_mem(pr, rrs + ii.imm, BITS_16, &tmp, true);
			if (res == excNone)
				pr->regs[ii.rt] = (tmp &0x8000) ?
					(tmp | 0xffff0000) :
					(tmp & 0xffff);
		}
		break;
	case opcLHU:
		{
			uint32_t tmp;
			res = read_proc_mem(pr, rrs + ii.imm, BITS_16, &tmp, true);
			if (res == excNone)
				pr->regs[ii.rt] = tmp & 0xffff;
		}
		break;
	case opcLL:
		{
			uint32_t ll_addr;  /* Target LL address */
			uint32_t ll_val;   /* Value of the addressed memory */
			
			/* Compute virtual target address
			   and issue read operation */
			ll_addr = rrs + ii.imm;
			res = read_proc_mem(pr, ll_addr, BITS_32, &ll_val, true);
			
			if (res == excNone) {  /* If the read operation has been successful */
				/* Store the value */
				pr->regs[ii.rt] = ll_val;
				
				/* Since we need physical address to track, issue the
				   address conversion. It can't fail now. */
				convert_addr(pr, &ll_addr, false, false);
				
				/* Register address for tracking. */
				register_sc(pr);
				pr->llbit = true;
				pr->lladdr = ll_addr;
			} else {
				/* Invalid address; Cancel the address tracking */
				unregister_sc(pr);
				pr->llbit = false;
				pr->lladdr = (uint32_t) -1;
			}
		}
		break;
	case opcLLD:
		res = excRI;
		break;
	case opcLUI:
		pr->regs[ii.rt] = ii.imm << 16;
		break;
	case opcLW:
		res = read_proc_mem(pr, rrs + ii.imm, BITS_32, (uint32_t *) &pr->regs[ii.rt], true);
		break;
	case opcLWL: 
		{
			static struct {
				uint32_t a;
				uint32_t s;
			} tab[] = {
				{ 0x00ffffff, 24 },
				{ 0x0000ffff, 16 },
				{ 0x000000ff, 8 },
				{ 0x00000000, 0 }
			};
			
			uint32_t val;
			uint32_t oaddr = rrs + ii.imm;
			uint32_t addr = oaddr & ~0x3;
			res = read_proc_mem(pr, addr, BITS_32, &val, true);
			
			if (res == excNone) {
				int index = oaddr & 0x3;
				
				pr->regs[ii.rt] &= tab[index].a;
				pr->regs[ii.rt] |= val << tab[index].s;
			}
		}
		break;
	case opcLWR:
		{
			static struct {
				uint32_t a;
				uint32_t s;
			} tab[] = {
				{ 0x00000000, 0 },
				{ 0xff000000, 8 },
				{ 0xffff0000, 16 },
				{ 0xffffff00, 24 }
			};
			
			uint32_t val;
			uint32_t oaddr = rrs + ii.imm;
			uint32_t addr = oaddr & ~0x3;
			res = read_proc_mem(pr, addr, BITS_32, &val, true);
			
			if (res == excNone) {
				int index = oaddr & 0x3;
				
				pr->regs[ii.rt] &= tab[index].a;
				pr->regs[ii.rt] |= (val >> tab[index].s) & ~tab[index].a;
			}
		}
		break;
	case opcLWU:
		res = excRI;
		break;
	case opcSB:
		res = write_proc_mem(pr, rrs + ii.imm, BITS_8, pr->regs[ii.rt], true);
		break;
	case opcSC:
		if (!pr->llbit) {
			/* If we are not tracking LL-SC,
			   then SC has to fail */
			pr->regs[ii.rt] = 0;
		} else {
			/* We do track LL-SC address */
			
			/* Compute trarget address */
			uint32_t sc_addr = rrs + ii.imm;
			
			/* Perform the write operation */
			res = write_proc_mem(pr, sc_addr, BITS_32, pr->regs[ii.rt], true);
			if (res == excNone) {
				/* The operation has been successful,
				   write the result, but... */
				pr->regs[ii.rt] = 1;
				
				/* ...we are too polite if LL and SC addresses differ.
				   In such a case, the behaviour of SC is undefined.
				   Let's check that. */
				convert_addr(pr, &sc_addr, false, false);
				
				/* sc_addr now contains physical target address */
				if (sc_addr != pr->lladdr) {
					/* LL and SC addresses do not match ;( */
					if (errors)
						mprintf("\nError: LL-SC addresses do not match\n\n");
				}
			} else {
				/* Error writing the target */
			}
			
			/* SC always stops LL-SC address tracking */
			unregister_sc(pr);
			pr->llbit = false;
			pr->lladdr = (uint32_t) -1;
		}
		break;
	case opcSCD:
		res = excRI;
		break;
	case opcSD:
		res = excRI;
		break;
	case opcSDL:
		res = excRI;
		break;
	case opcSDR:
		res = excRI;
		break;
	case opcSH:
		res = write_proc_mem(pr, rrs + ii.imm, BITS_16, pr->regs[ii.rt], true);
		break;
	case opcSW:
		res = write_proc_mem(pr, rrs + ii.imm, BITS_32, pr->regs[ii.rt], true);
		break;
	case opcSWL:
		{
			static struct {
				uint32_t a;
				uint32_t s;
			} tab[] = {
				{ 0xffffff00, 24 },
				{ 0xffff0000, 16 },
				{ 0xff000000, 8 },
				{ 0x00000000, 0 }
			};
			
			uint32_t val;
			uint32_t oaddr = rrs + ii.imm;
			uint32_t addr = oaddr & ~0x3;
			res = read_proc_mem(pr, addr, BITS_32, &val, true);
			
			if (res == excNone) {
				int index = oaddr & 0x3;
				
				val &= tab[index].a;
				val |= (pr->regs[ii.rt] >> tab[index].s) & ~tab[index].a;
				
				res = write_proc_mem(pr, addr, BITS_32, val, true);
			}
		}
		break;
	case opcSWR:
		{
			static struct {
				uint32_t a;
				uint32_t s;
			} tab[] = {
				{ 0x00000000, 0 },
				{ 0x000000ff, 8 },
				{ 0x0000ffff, 16 },
				{ 0x00ffffff, 24 }
			};
			
			uint32_t val;
			uint32_t oaddr = rrs + ii.imm;
			uint32_t addr = oaddr & ~0x3;
			res = read_proc_mem(pr, addr, BITS_32, &val, true);
			
			if (res == excNone) {
				int index = oaddr & 0x3;
				
				val &= tab[index].a;
				val |= pr->regs[ii.rt] << tab[index].s;
				
				res = write_proc_mem(pr, addr, BITS_32, val, true);
			}
		}
		break;
	
	/*
	 * traps
	 */
	case opcTEQ:
		TRAP(rrs == rrt);
		break;
	case opcTEQI:
		TRAP(rrs == ii.imm);
		break;
	case opcTGE:
		TRAP(rrs >= rrt);
		break;
	case opcTGEI:
		TRAP(rrs >= ii.imm);
		break;
	case opcTGEIU:
		TRAP((uint32_t) rrs >= (uint32_t) ii.imm);
		break;
	case opcTGEU:
		TRAP((uint32_t) rrs >= (uint32_t) rrt);
		break;
	case opcTLT:
		TRAP(rrs < rrt);
		break;
	case opcTLTI:
		TRAP(rrs < ii.imm);
		break;
	case opcTLTIU:
		TRAP((uint32_t) rrs < (uint32_t) ii.imm);
		break;
	case opcTLTU:
		TRAP((uint32_t) rrs < (uint32_t) rrt);
		break;
	case opcTNE:
		TRAP(rrs != rrt);
		break;
	case opcTNEI:
		TRAP(rrs != ii.imm);
		break;
	
	/*
	 * special instructions
	 */
	case opcCFC0:
		/* This instruction is not valid */
		break;
	case opcCFC1:
		if (cp0_status_cu1 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu1;
		}
		break;
	case opcCFC2:
		if (cp0_status_cu2 == 1) {
				/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu2;
		}
		break;
	case opcCFC3:
		if (cp0_status_cu3 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu3;
		}
		break;
	case opcCTC0:
		/* This instruction is not valid */
		break;
	case opcCTC1:
		if (cp0_status_cu1 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu1;
		}
		break;
	case opcCTC2:
		if (cp0_status_cu2 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu2;
		}
		break;
	case opcCTC3:
		if (cp0_status_cu3) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu3;
		}
		break;
	case opcERET:
		if ((cp0_status_cu0)
		    || (!cp0_status_ksu)
		    || (cp0_status_exl)
		    || (cp0_status_erl)) {
			/* ERET breaks LL-SC address tracking */
			pr->llbit = false;
			pr->lladdr = (uint32_t) -1;
			unregister_sc(pr);
			
			/* Delay slot test */
			if ((pr->branch != BRANCH_NONE) && (errors))
				mprintf("\nError: ERET in a delay slot\n\n");
			
			if (cp0_status_erl) {
				/* Error level */
				pr->pc_next = cp0_errorepc;
				pca = pr->pc_next + 4;
				cp0_status &= ~cp0_status_erl_mask;
			} else {
				/* Exception level */
				pr->pc_next = cp0_epc;
				pca = pr->pc_next + 4;
				cp0_status &= ~cp0_status_exl_mask;
			}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}	
		break;
	case opcDMFC0:
		res = excRI;
		break;
	case opcDMTC0:
		res = excRI;
		break;
	case opcMFC0:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl == 1)
		    || (cp0_status_erl == 1)))
			pr->regs[ii.rt] = pr->cp0[ii.rd];
		else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcMFHI:
		pr->regs[ii.rd] = pr->hireg;
		break;
	case opcMFLO:
		pr->regs[ii.rd] = pr->loreg;
		break;
	case opcMTC0:
		if ((cp0_status_cu0 == 1)
		    || ((cp0_status_ksu == 0)
		    || (cp0_status_exl)
		    || (cp0_status_erl)))
			switch (ii.rd) {
			/* 0 */
			case cp0_Index:
				cp0_index = rrt & 0x3f;
				break;
			case cp0_Random:
				/* Ignored, read-only */
				break;
			case cp0_EntryLo0:
				cp0_entrylo0 = rrt & 0x3fffffff;
				break;
			case cp0_EntryLo1:
				cp0_entrylo1 = rrt & 0x3fffffff;
				break;
			case cp0_Context:
				cp0_context = rrt & 0xfffffff0;
				break;
			case cp0_PageMask:
				cp0_pagemask = 0;
				if ((rrt == 0x0)
					| (rrt == 0x6000)
					| (rrt == 0x1e000)
					| (rrt == 0x7e000)
					| (rrt == 0x1fe000)
					| (rrt == 0x7fe000)
					| (rrt == 0x1ffe000))
					cp0_pagemask = rrt & cp0_pagemask_mask_mask;
				else if (errors) 
					mprintf("\nMTC0: Invalid value for PageMask\n");
				break;
			case cp0_Wired:
				cp0_random = 47;
				cp0_wired = rrt & 0x3f;
				if (cp0_wired > 47) 
					mprintf("\nMTC0: Invalid value for Wired\n");
				break;
			case cp0_Res1:
				/* Ignored, reserved */
				break;
			/* 8 */
			case cp0_BadVAddr:
				/* Ignored, read-only */
				break;
			case cp0_Count:
				cp0_count = rrt;
				break;
			case cp0_EntryHi:
				cp0_entryhi = rrt & 0xfffff0ff;
				break;
			case cp0_Compare:
				cp0_compare = rrt;
				cp0_cause &= ~(1 << cp0_cause_ip7_shift);
				break;
			case cp0_Status:
				cp0_status = rrt & 0xff77ff1f;
				break;
			case cp0_Cause:
				cp0_cause &= ~(cp0_cause_ip0_mask | cp0_cause_ip1_mask);
				cp0_cause |= rrt & (cp0_cause_ip0_mask | cp0_cause_ip1_mask);
				break;
			case cp0_EPC:
				cp0_epc = rrt;
				break;
			case cp0_PRId:
				/* Ignored, read-only */
				break;
			/* 16 */
			case cp0_Config:
				/* Ignored for simulation */
				cp0_config = rrt & 0xffffefff;
				break;
			case cp0_LLAddr:
				cp0_lladdr = rrt;
				break;
			case cp0_WatchLo:
				cp0_watchlo = rrt & (~cp0_watchlo_res_mask);
				pr->waddr = cp0_watchhi_paddr1;
				pr->waddr <<= (32 - cp0_watchlo_paddr0_shift);
				pr->waddr |= cp0_watchlo_paddr0;
				break;
			case cp0_WatchHi:
				cp0_watchhi = rrt & (~cp0_watchhi_res_mask);
				pr->waddr = cp0_watchhi_paddr1;
				pr->waddr <<= (32 - cp0_watchlo_paddr0_shift);
				pr->waddr |= cp0_watchlo_paddr0;
				break;
			case cp0_XContext:
				/* Ignored ? in 32bit MIPS */
				break;
			case cp0_Res2:
				/* Ignored, reserved */
				break;
			case cp0_Res3:
				/* Ignored, reserved */
				break;
			case cp0_Res4:
				/* Ignored, reserved */
				break;
			/* 24 */
			case cp0_Res5:
				/* Ignored, reserved */
				break;
			case cp0_Res6:
				/* Ignored, reserved */
				break;
			case cp0_ECC:
				/* Ignored for simulation */
				cp0_ecc = (rrt & cp0_ecc_ecc_mask) << cp0_ecc_ecc_shift;
				break;
			case cp0_CacheErr:
				/* Ignored, read-only */
				break;
			case cp0_TagLo:
				cp0_taglo = rrt;
				break;
			case cp0_TagHi:
				cp0_taghi = rrt;
				break;
			case cp0_ErrorEPC:
				cp0_errorepc = rrt;
				break;
			case cp0_Res7:
				/* Ignored */
				break;
			} else {
				/* Coprocessor unusable */
				res = excCpU;
				cp0_cause &= ~cp0_cause_ce_mask;
			}
		break;
	case opcMTC1:
		if (cp0_status_cu1 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu1;
		}
		break;
	case opcMTC2:
		if (cp0_status_cu2 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu2;
		}
		break;
	case opcMTC3:
		if (cp0_status_cu3 == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
			cp0_cause |= cp0_cause_ce_cu3;
		}
		break;
	case opcDMTC1:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDMTC2:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDMTC3:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcSDC1:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcSDC2:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcMTHI:
		pr->hireg = rrs;
		break;
	case opcMTLO:
		pr->loreg = rrs;
		break;
	case opcSYNC:
		/* No synchronisation is needed */
		break;
	case opcSYSCALL:
		res = excSys;
		break;
	case opcRES:
		res = excRI;
		break;
	case opcQRES:
		/* Quiet reserved */
		break;
	case opcTLBP:
		if ((cp0_status_cu0 == 1) || (cp0_status_ksu == 0) ||
		    (cp0_status_exl == 1) || (cp0_status_erl == 1)) {
			uint32_t xvpn2, xasid;
			unsigned int i;
			
			cp0_index = 1 << cp0_index_p_shift;
			
			xvpn2 = cp0_entryhi & cp0_entryhi_vpn2_mask;
			xasid = cp0_entryhi & cp0_entryhi_asid_mask;
			for (i = 0; i < 48; i++)
				if ((pr->tlb[i].vpn2 == xvpn2) &&
				    (pr->tlb[i].global || (pr->tlb[i].asid == xasid))) {
					cp0_index = i;
					break;
				}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcTLBR:
		if ((cp0_status_cu0 == 1) ||
		    (cp0_status_ksu == 0) || (cp0_status_exl == 1) ||
		    (cp0_status_erl == 1)) {
			int i = cp0_index_index;
			
			if (i > 47) {
				mprintf("\nTLBR: Invalid value in Index\n");
				cp0_pagemask = 0;
				cp0_entryhi = 0;
				cp0_entrylo0 = 0;
				cp0_entrylo1 = 0;
			} else {
				cp0_pagemask = (~pr->tlb[i].mask) & 0x01ffe000;
				cp0_entryhi = pr->tlb[i].vpn2 | pr->tlb[i].asid;
				
				cp0_entrylo0 = (pr->tlb[i].pg[0].pfn >> 6)
					| (pr->tlb[i].pg[0].cohh << 3)
					| ((pr->tlb[i].pg[0].dirty ? 1 : 0) << 2)
					| ((pr->tlb[i].pg[0].valid ? 1 : 0) << 1)
					| (pr->tlb[i].global ? 1 : 0);
				
				cp0_entrylo1 = (pr->tlb[i].pg[1].pfn >> 6)
					| (pr->tlb[i].pg[1].cohh << 3)
					| ((pr->tlb[i].pg[1].dirty ? 1 : 0) << 2)
					| ((pr->tlb[i].pg[1].valid ? 1 : 0) << 1)
					| (pr->tlb[i].global ? 1 : 0);
			}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause &= ~cp0_cause_ce_mask;
		}
		break;
	case opcTLBWI:
		TLBW(pr, false, &res);
		break;
	case opcTLBWR:
		TLBW(pr, true, &res);
		break;
	case opcBREAK:
		res = excBp;
		break;
	case opcWAIT:
		pr->pc_next = pr->pc;
		pr->stdby = true;
		break;
	
	/*
	 * machine debugging instructions
	 */
	case opcDVAL:
		mprintf("\nDebug: value %Xh (%dd)\n\n", pr->regs[4], pr->regs[4]);
		break;
	case opcDTRC:
		if (!totrace) {
			reg_view(pr);
			mprintf("\n");
		}
		update_deb(pr);
		totrace = true;
		break;
	case opcDTRO:
		totrace = false;
		break;
	case opcDRV:
		mprintf("\nDebug: register view\n");
		reg_view(pr);
		mprintf("\n");
		break;
	case opcDHLT:
		if (totrace)
			mprintf("\nMachine halt\n\n");
		tohalt = true;
		break;
	case opcDINT:
		interactive = true;
		break;
	}
	
	/* Branch test */
	if ((pr->branch == BRANCH_COND) || (pr->branch == BRANCH_NONE))
		pr->excaddr = pr->pc;
	
	/* PC update */
	if (res == excNone) {
		pr->pc = pr->pc_next;
		pr->pc_next = pca;
	}
	
	/* reg0 control */
	pr->regs[0] = 0;
	
	return res;
}

/** Change the processor state according to the exception type
 *
 */
static void handle_exception(cpu_t *cpu, exc_t res)
{
	bool tlb_refill = false;
	
	/* Convert TLB Refill exceptions */
	if ((res == excTLBLR) || (res == excTLBSR)) {
		tlb_refill = true;
		if (res == excTLBLR)
			res = excTLBL;
		else
			res = excTLBS;
	}
	
	pr->stdby = false;
	
	/* User info and register filling */
	if (totrace)
		mprintf("\nRaised exception: %s\n\n", exc_text[res]);
	
	cp0_cause &= ~cp0_cause_exccode_mask;
	cp0_cause |= res << cp0_cause_exccode_shift;
	
	/* Exception branch control */
	cp0_cause &= ~cp0_cause_bd_mask;
	if (pr->branch == BRANCH_PASSED)
		cp0_cause |= cp0_cause_bd_mask;
	
	if (!cp0_status_exl) {
		cp0_epc = pr->excaddr;
		if ((res == excInt) && (pr->branch != BRANCH_COND))
			cp0_epc = pr->pc;
	}
	
	/* Exception vector base address */
	if (cp0_status_bev) {
		/* Booting time */
		if (res != excReset)
			pr->pc = 0xbfc00200;
		else
			pr->pc = 0xbfc00000;
	} else {
		/* Normal time */
		if (res != excReset)
			pr->pc = 0x80000000;
		else
			pr->pc = 0xbfc00000;
	}
	
	/* Exception vector offsets */
	if ((cp0_status_exl) || (!tlb_refill))
		pr->pc += EXCEPTION_OFFSET;
	
	pr->pc_next = pr->pc + 4;
	
	/* Turn to kernel mode */
	cp0_status |= cp0_status_exl_mask;
}

/** React on interrupt requests, updates internal timer, random register
 *
 */
static void manage(cpu_t *cpu, exc_t res)
{
	/* Test for interrupt request */
	if ((res == excNone)
	    && (!cp0_status_exl)
	    && (!cp0_status_erl)
	    && (cp0_status_ie)
	    && ((cp0_cause & cp0_status) & cp0_cause_ip_mask) != 0)
		res = excInt;
	
	/* Exception control */
	if (res != excNone)
		handle_exception(cpu, res);
	
	/* Increase counter */
	cp0_count(cpu)++;
	
	/* Decrease random register */
	if (cp0_random(cpu)-- == 0)
		cp0_random(cpu) = 47;
	
	if (cp0_random(cpu) < cp0_wired(cpu))
		cp0_random(cpu) = 47;
	
	/* Timer control */
	if (cp0_count(cpu) == cp0_compare(cpu))
		/* Generate interrupt request */
		cp0_cause(cpu) |= 1 << cp0_cause_ip7_shift;
}

/* Simulate one instruction
 *
 * Four main stages are performed:
 *  - instruction fetch
 *  - instruction decode
 *  - instruction execution
 *  - debugging output
 *
 */
static void instruction(cpu_t *cpu, exc_t *res)
{
	instr_info_t ii;
	
	/* Fetch instruction */
	*res = read_proc_ins(cpu, cpu->pc, &ii.icode, true);
	if (*res == excNone) {
		/* Decode instruction */
		decode_instr(&ii);
		
		/* Execute instruction */
		uint32_t old_pc = cpu->pc;
		*res = execute(cpu, &ii);
		
		/* Debugging output */
		if (totrace) {
			char *modified_regs;
			
			if (iregch)
				modified_regs = modified_regs_dump(cpu);
			else
				modified_regs = NULL;
			
			iview(cpu, old_pc, &ii, modified_regs);
			
			if (modified_regs != NULL)
				safe_free(modified_regs);
		}
	}
}

/* Simulate one step of the processor
 *
 * This is just one instruction.
 *
 */
void cpu_step(cpu_t *cpu)
{
	exc_t res = excNone;
	
	/* Instruction execute */
	if (!cpu->stdby)
		instruction(cpu, &res);
	
	/* Processor control */
	manage(cpu, res);
	
	/* Cycle accounting */
	if (cpu->stdby)
		cpu->w_cycles++;
	else {
		if ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))
			cpu->k_cycles++;
		else
			cpu->u_cycles++;
	}
	
	/* Branch delay slot control */
	if (cpu->branch > BRANCH_NONE)
		cpu->branch--;
}
