/*
 * A MIPS R4000 processor simulation
 * 
 * Copyright (c) 2000-2004 Viliam Holub 
 */

#include <stdlib.h>
#include <stdio.h>

#include "mtypes.h"
#include "endi.h"
#include "processor.h"
#include "machine.h"
#include "debug.h"
#include "text.h"
#include "output.h"
#include "gdb.h"
#include "env.h"



/*
 * Initial state
 */
#define	HARD_RESET_STATUS		(cp0_status_erl_mask|cp0_status_bev_mask)
#define	HARD_RESET_START_ADDRESS	0xbfc00000
#define HARD_RESET_PROC_ID		0x00000200
#define HARD_RESET_CAUSE		0
#define HARD_RESET_WATCHLO		0
#define HARD_RESET_WATCHHI		0
#define HARD_RESET_CONFIG		0
#define HARD_RESET_RANDOM		47
#define HARD_RESET_WIRED		0

#define EXCEPTION_OFFSET	0x180


/*
 * Actual processor context. Not very elegant...
 */
processor_s *pr;



/** processor_init - Initialize simulation environment.
 */
void
processor_init( int procno)

{
	int i;
 
	pr->procno = procno;
	pr->stdby = false;
	
	pr->k_cycles = 0;
	pr->u_cycles = 0;
	pr->w_cycles = 0;
	pr->tlb_refill = 0;
	pr->tlb_invalid = 0;
	pr->tlb_modified = 0;
	
	/* inicializing general registers */
	for (i=0; i<32; i++) pr->regs[ i] = pr->fpregs[ i] = 0;

	/* inicializing registers */
	pr->loreg = pr->hireg = 0;
	pr->pcreg = HARD_RESET_START_ADDRESS;
	pr->pcnextreg = pr->pcreg + 4;
	pr->branch = 0;

	/* inicializing internal variables */
	pr->lladdr = (uint32_t) -1;
	pr->llval = false;

	/* inicializing cp0 registers */
	pr->cp0[ CP0_Config] = HARD_RESET_CONFIG;
	pr->cp0[ CP0_Random] = HARD_RESET_RANDOM;
	pr->cp0[ CP0_Wired] = HARD_RESET_WIRED;
	pr->cp0[ CP0_PRId] = HARD_RESET_PROC_ID;
	
	/* initial status value */
	pr->cp0[ CP0_Status] = HARD_RESET_STATUS; 
	
	pr->cp0[ CP0_Cause] = HARD_RESET_CAUSE;
	pr->cp0[ CP0_WatchLo] = HARD_RESET_WATCHLO;
	pr->cp0[ CP0_WatchHi] = HARD_RESET_WATCHHI;

	for (i=0;i<8;i++)
		pr->intr[ i] = 0;
	
	/* inicializing registers for debug */
	for (i=0; i<32; i++)
		pr->old_regs[ i] = 0;
	for (i=0; i<32; i++)
		pr->old_cp0[ i] = pr->cp0[ i];
	pr->old_loreg = pr->loreg; pr->old_hireg = pr->hireg;

	/* setting up list of tlb entries */
	pr->tlblist = &pr->tlb[ 47];
	for (i=47; i; i--)
		pr->tlb[ i].next = &pr->tlb[ i-1];
	pr->tlb[ 0].next = 0;
}


/** set_general_reg - Writes a value into the register in the actual context.
 *
 * Used mainly from external modules.
 */
void
set_general_reg( int regno, int value)
		
{ 
	if (regno != 0)
		pr->regs[ regno] = value;
}


/** set_pc_reg - Sets the PC register in the actual context.
 */
void
set_pc_reg( int value)

{
	pr->pcreg = value;
	pr->pcnextreg = value +4;
}


enum tlb_look_e
{
	tlbl_ok,
	tlbl_refill,
	tlbl_invalid,
	tlbl_modified
};


/** tlb_look - Address TLB traslation.
 *
 * R	see tlbl_look_e definition
 */
static enum tlb_look_e
tlb_look( uint32_t *addr, bool wr)

{
	TLBEnt *e, *prev;
	uint32_t a = *addr;

	/* ignore TLB test */
	if (cp0_status_ts == 1)
		return tlbl_ok;

	/* looking for the TBL hit */
	for (prev=0, e=pr->tlblist; e; prev=e, e=e->next)
	{
		/* TLB hit?  */
		if ((a & e->mask) == (uint32_t) e->vpn2)

		{
			int smask, subpageno;

			/* testing asid */
			if (!e->global && (e->asid != cp0_entryhi_asid))
				continue;

			/* count sub-page */
			smask = (e->mask >> 1) | SBIT; 
			subpageno = ((a & e->mask) < (a & smask)) ? 1 : 0;

			/* testing valid & dirty */
			if (!e->pg[ subpageno].valid)
				return tlbl_invalid; 
			if (wr && !e->pg[ subpageno].dirty)
				return tlbl_modified; 

			/* making address */
			a &= ~smask;
			*addr = a | (e->pg[ subpageno].pfn & smask);

			/* ok, now optimizing the list */
			if (prev)
			{
				prev->next = e->next;
				e->next = pr->tlblist;
				pr->tlblist = e;
			}

			return tlbl_ok;
		}
	}

	return tlbl_refill;
}


/** fill_tlb_error - Fills cp0 registers with specified address.
 */
static void
fill_tlb_error( uint32_t addr)

{
	cp0_badvaddr = addr;
	cp0_context &= cp0_context_ptebase_mask;
	cp0_context |= (addr >> cp0_context_addr_shift) & ~cp0_context_res1_mask;
	cp0_entryhi &= cp0_entryhi_asid_mask;
	cp0_entryhi |= addr & cp0_entryhi_vpn2_mask;
}


/** tlb_hit - Looks for TLB and generates apropriate exception.
 */
static enum exc
tlb_hit( uint32_t *addr, bool wr, bool h)

{
	switch (tlb_look( addr, wr))
	{
		case 0: /* ok */
			break;
		case 1: /* refill */
			if (h)
			{
				pr->tlb_refill++;
				fill_tlb_error( *addr);
			}
			return excTLBR;
		case 2: /* invalid */
			if (h)
			{
				pr->tlb_invalid++;
				fill_tlb_error( *addr);
			}
			return excTLB;
		case 3: /* modified */
			if (h)
			{
				pr->tlb_modified++;
				fill_tlb_error( *addr);
			}
			return excMod;
		
	}
	return excNone;
}


/** convert_addr_user - The user mode address conversion.
 */
static enum exc
convert_addr_user( uint32_t *addr, bool wr, bool h)

{
	/* testing 31 bit or looking to TLB */
	if ((*addr & SBIT) != 0)
		return excAddrError;
	
	return tlb_hit( addr, wr, h); /* useg */
}


/** convert_addr_supervisor - The supervisor mode address conversion.
 */
static enum exc
convert_addr_supervisor( uint32_t *addr, bool wr, bool h)

{

	if (*addr < 0x80000000) /* suseg */
		return tlb_hit( addr, wr, h);
	else if (*addr < 0xc0000000)
		return excAddrError;
	else if (*addr < 0xe0000000)
		return tlb_hit( addr, wr, h);  /* ssseg */
	else /* *addr > 0xe0000000 */
		return excAddrError;
}


/** convert_addr_kernel - The kernel mode address conversion.
 */
static enum exc
convert_addr_kernel( uint32_t *addr, bool wr, bool h)

{
	if (*addr < 0x80000000) /* kuseg */
	{
		if (!cp0_status_erl)
			return tlb_hit( addr, wr, h);
	}
	else if (*addr < 0xa0000000) *addr -= 0x80000000; /* kseg0 */
	else if (*addr < 0xc0000000) *addr -= 0xa0000000; /* kseg1 */
	else if (*addr < 0xe0000000) /* kseg2 */
		return tlb_hit( addr, wr, h);
	else /*	*addr > 0xe0000000*/ /* kseg3 */
		return tlb_hit( addr, wr, h);

	return excNone;
}


/** convert_addr - The conversion of virtual addresses.
 */
enum exc
convert_addr( uint32_t *addr, bool wr, bool h)

{
	/* testing type of translation */
	if ((cp0_status_ksu == 2) && !cp0_status_exl && !cp0_status_erl)
		return convert_addr_user( addr, wr, h);
	else
	if ((cp0_status_ksu == 1) && !cp0_status_exl && !cp0_status_erl)
		return convert_addr_supervisor( addr, wr, h);
	else
		return convert_addr_kernel( addr, wr, h);
}


/** mem_align_test - Tests for correct align and fills BadVAddr if bad.
 */
static enum exc
mem_align_test( uint32_t addr, int size)

{
	if (((size == 2) && (addr & 1)) || ((size == 4) && (addr & 3)))
		{ 
			cp0_badvaddr = addr;
			return excAddrError; 
		}
	return excNone;
}
	

/** acc_mem - Access the virtual memory.
 *
 * The operation (read/write) is specified via the wr parameter. This routine
 * does not specify the exception type.
 *
 * IN wr	operation type specification
 * IN addr	memory address
 * IN size	argument size in bytes - 1, 2, 4
 * IO value	the value which has to be written/read to
 * IN h		generate exception in case of invalid operation
 */
static enum exc
acc_mem( bool wr, uint32_t addr, int size, uint32_t *value, bool h)

{
	enum exc res;
	
	if ((res=mem_align_test( addr, size)) == excNone)
	{
		res = convert_addr( &addr, wr, h);

		if (res == excNone)
		{
			if (wr)
				mem_write( addr, *value, size);
			else
				*value = mem_read( addr);
		}
	}

	return res;
}


/** read_proc_mem - Preforms the read access from the virtual memory.
 */
enum exc
read_proc_mem( uint32_t addr, int size, uint32_t *value, bool h)

{
	switch (acc_mem( false, addr, size, value, h))
	{
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


/** write_proc_mem - Performs the write operation to the virtual memory.
 */
enum exc
write_proc_mem( uint32_t addr, int size, uint32_t value, bool h)

{ 
	switch (acc_mem( true, addr, size, &value, h))
	{
		case excAddrError:
			return excAdES;
		case excTLB:
			return excTLBS;
		case excTLBR:
			return excTLBSR;
		default:
			return excNone;
	}
}


/** read_proc_ins - Reads an instruction.
 */
enum exc
read_proc_ins( uint32_t addr, uint32_t *value, bool h)

{
	enum exc res;

	res = read_proc_mem( addr, INT32, value, h);
	if (res != excNone)
		pr->excaddr = pr->pcreg;

	return res;
}


/** proc_interrupt_up - Asserts the specified interrupt.
 */
void
proc_interrupt_up( int no)

{
	if (no >= 0 && no <= 6)
	{
		cp0_cause |= 1 << (cp0_cause_ip0_shift +no);
		pr->intr[ no]++;
	}
}


/* proc_interrupt_doen - Deasserts the specified interrupt.
 */
void
proc_interrupt_down( int no)

{
	if (no >= 0 && no <= 6)
		cp0_cause &= ~(1 << (cp0_cause_ip0_shift +no));
}


/** update_deb - Updates the copy of registers.
 */
void
update_deb( void)

{
	int i;
	for (i=0; i<32; i++)
		pr->old_regs[ i] = pr->regs[ i];
	for (i=0; i<32; i++)
		pr->old_cp0[ i] = pr->cp0[ i];
	pr->old_loreg = pr->loreg;
	pr->old_hireg = pr->hireg;
}


/** multiply - A multiplication of two integers.
 */
static void
Multiply( uint32_t a, uint32_t b, bool sig)

{
	/* quick test */
	if ((a == 0) || (b == 0))
	{
		pr->hireg = pr->loreg = 0;
		return;
	}

	/* signum test */
	if (sig)
	{
		/* signed multiplication */
		int64_t r = (int64_t)a * (int64_t)b;
		
		/* result */
		pr->loreg = r & 0xffffffff;
		pr->hireg = r >> 32;
	}
	else
	{
		/* unsigned multiplication */
		uint64_t r = (uint64_t)a * (uint64_t)b;
		
		/* result */
		pr->loreg = r & 0xffffffff;
		pr->hireg = r >> 32;
	}
}


/** TLBW - Writes a new entry into the TLB.
 */
static void
TLBW( int reg, enum exc *res)

{
	if ((cp0_status_cu0 == 1)||
		 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))
	{
		int i = pr->cp0[ reg];
		TLBEnt *t = &pr->tlb[ i];

		if (i > 47)
			dprintf( "\nTLBWI: Invalid value in Index\n");
		else
		{
			/* tlb filling */
			t->mask = 0xffffe000 & ~cp0_pagemask;
			t->vpn2 = cp0_entryhi & t->mask;
			t->global = cp0_entrylo0_g & cp0_entrylo1_g;
			t->asid = cp0_entryhi_asid;

			t->pg[ 0].pfn = cp0_entrylo0_pfn << 12;
			t->pg[ 0].cohh	= cp0_entrylo0_c;
			t->pg[ 0].dirty = cp0_entrylo0_d;
			t->pg[ 0].valid = cp0_entrylo0_v;
			
			t->pg[ 1].pfn = cp0_entrylo1_pfn << 12;
			t->pg[ 1].cohh	= cp0_entrylo1_c;
			t->pg[ 1].dirty = cp0_entrylo1_d;
			t->pg[ 1].valid = cp0_entrylo1_v;
		}
	}
	else
	{
		/* coprocessor unusable */
		*res = excCpU;
		pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
	}
}



/** execute - Execute an instruction specified by opcode.
 */
static enum exc
execute( TInstrInfo *ii2)

{
	TInstrInfo ii = *ii2;
	enum exc res = excNone;
	uint32_t pca = pr->pcnextreg +4;
	int rrs = pr->regs[ ii.rs];
	int rrt = pr->regs[ ii.rt];
	
	switch (ii.opcode)
	{
		/*
		 * aritmetic, logic, shifts
		 */

		case opcADD:
			{
				int32_t i1;
				i1 = rrs +rrt;
				if (!((rrs^rrt) & SBIT) && ((rrs^i1) & SBIT)) 
				{
					res = excOv;
					break;
				}
				pr->regs[ ii.rd] = i1;
			}
			break;
			
		case opcADDI:
			{
				int32_t i1;
				i1 = rrs +ii.imm;
				if (!( (rrs ^ ii.imm) & SBIT) && ((ii.imm ^ i1) & SBIT)) 
				{
					res = excOv;
					break;
				}
				pr->regs[ ii.rt] = i1;
			}
			break;
			
		case opcADDIU:
			pr->regs[ ii.rt] = rrs + ii.imm;
			break;
			
		case opcADDU:
			pr->regs[ ii.rd] = rrs + rrt;
			break;
			
		case opcAND:
			pr->regs[ ii.rd] = rrs & rrt;
			break;
			
		case opcANDI:
			pr->regs[ ii.rt] = rrs & (ii.imm & 0xffff);
			break;

		case opcDADD:
		case opcDADDI:
		case opcDADDIU:
		case opcDADDU:
		case opcDDIV:
		case opcDDIVU:
			/* 64-bit instructions */
			/* raise exception */
			res = excRI;
			break;
			
		case opcDIV:
			if ( rrt == 0) pr->loreg = pr->hireg = 0;
			else 
			{
				pr->loreg = rrs / rrt;
				pr->hireg = rrs % rrt;
			}
			break;
			
		case opcDIVU:		
			{
				uint32_t dw1 = (uint32_t) rrs;
				uint32_t dw2 = (uint32_t) rrt;
				if (dw2 == 0) pr->loreg = pr->hireg = 0; 
				else 
				{
					pr->loreg = (int) (dw1 / dw2); 
					pr->hireg = (int) (dw1 % dw2); 
				}
				}
			break;

		case opcDMULT:
		case opcDMULTU:
		case opcDSLL:
		case opcDSLLV:
		case opcDSLL32:
		case opcDSRA:
		case opcDSRAV:
		case opcDSRA32:
		case opcDSRL:
		case opcDSRLV:
		case opcDSRL32:
		case opcDSUB:
		case opcDSUBU:
			/* 64-bit instructions */
			/* raise exception */
			res = excRI;
			break;

		case opcMULT:
			Multiply( rrs, rrt, true);
			break;
			
		case opcMULTU:
			Multiply( rrs, rrt, false);
			break;

		case opcNOR:
			pr->regs[ ii.rd] = ~(rrs | rrt);
			break;
			
		case opcOR:
			pr->regs[ ii.rd] = rrs | rrt;
			break;
			
		case opcORI:
			pr->regs[ ii.rt] = rrs | ( ii.imm & 0xffff);
			break;
			
		case opcSLL:
			pr->regs[ ii.rd] = (uint32_t)rrt << ii.shift;
			break;
			
		case opcSLLV:
			pr->regs[ ii.rd] = (uint32_t)rrt << (rrs & 0x1f);
			break;
		 
		case opcSLT:
			pr->regs[ ii.rd] = rrs < rrt;
			break;
			
		case opcSLTI:
			pr->regs[ ii.rt] = rrs < ii.imm;
			break;
			
		case opcSLTIU:
			pr->regs[ ii.rt] = (uint32_t)rrs < (uint32_t)ii.imm;
			break;
						
		case opcSLTU:
			pr->regs[ ii.rd] = (uint32_t)rrs < (uint32_t) rrt;
			break;
						
		case opcSRA:
			pr->regs[ ii.rd] = rrt >> ii.shift;
			break;
			
		case opcSRAV:
			pr->regs[ ii.rd] = rrt >> (rrs & 0x1f);
			break;
			
		case opcSRL:
			pr->regs[ ii.rd] = (uint32_t)rrt >> ii.shift;		
			break;
			
		case opcSRLV:
			pr->regs[ ii.rd] = (uint32_t)rrt >> (rrs & 0x1f);
			break;
			
		case opcSUB:
			{
				uint32_t i = rrs-rrt;
				if (((rrs^rrt) & SBIT) && ((rrs^i) & SBIT))
				{
					res = excOv;
					break;
				}
				pr->regs[ ii.rd] = i;
			}
			break;
						
		case opcSUBU:
			pr->regs[ ii.rd] = rrs-rrt;
			break;
			
		case opcXOR:
			pr->regs[ ii.rd] = rrs^rrt;
			break;
			
		case opcXORI:
			pr->regs[ ii.rt] = rrs ^ (ii.imm & 0xffff);
			break;

		/*
		 * branches and jumps
		 */


		case opcBC0FL:
		case opcBC1FL:
		case opcBC2FL:
		case opcBC3FL:
			if ((cp0_status_cu0 == 1)||
				 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))

			{
				/* ignore - always false */
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;

		case opcBC0F:
		case opcBC1F:
		case opcBC2F:
		case opcBC3F:
			if ((cp0_status_cu0 == 1)||
				 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))

			{
				/* ignore - always false */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;

		case opcBC0TL:
		case opcBC1TL:
		case opcBC2TL:
		case opcBC3TL:
			if ((cp0_status_cu0 == 1)||
				 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))

			{
				/* ignore - always true */
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;

		case opcBC0T:
		case opcBC1T:
		case opcBC2T:
		case opcBC3T:
			if ((cp0_status_cu0 == 1)||
				 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))

			{
				/* ignore - always true */
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;

		case opcBEQ:
			if (rrs == rrt)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;
			
		case opcBEQL:
			if (rrs == rrt)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;

		case opcBGEZAL:
			pr->regs[ 31] = pr->pcreg +8;
			/* no break */
		case opcBGEZ:
			if (!(rrs & SBIT))
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;
			
		case opcBGEZALL:
			pr->regs[ 31] = pr->pcreg +8;
			/* no break */
		case opcBGEZL:
			if (!(rrs & SBIT))
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;
			
		case opcBGTZ:
			if (rrs > 0)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;
			
		case opcBGTZL:
			if (rrs > 0)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;
			
		case opcBLEZ:
			if (rrs <= 0)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;
			
		case opcBLEZL:
			if (rrs <= 0)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;

		case opcBLTZAL:
			pr->regs[ 31] = pr->pcnextreg +4;
			/* no break */
		case opcBLTZ:
			if (rrs & SBIT)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;
			
		case opcBLTZALL:
			pr->regs[ 31] = pr->pcnextreg +4;
			/* no break */
		case opcBLTZL:
			if (rrs & SBIT)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;

		case opcBNE:
			if (rrs != rrt)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			break;

		case opcBNEL:
			if (rrs != rrt)
			{
				pca = pr->pcnextreg +(ii.imm<<2);
				pr->branch = 2;
			}
			else
			{
				pr->pcnextreg += 4;
				pca = pr->pcnextreg +4;
			}
			break;

		case opcJAL:
			pr->regs[ 31] = pr->pcnextreg +4;
			/* no break */
		case opcJ:
			pca = (pr->pcnextreg & TARGET_COMB) | ii.imm;
			pr->branch = 2;
			break;
			
		case opcJALR:
			pr->regs[ ii.rd] = pr->pcnextreg +4;
			/* no break */
		case opcJR:
			pca = rrs;
			pr->branch = 2;
			break;

		/*
		 * load, store
		 */

		case opcLB:
			{
				uint32_t dw1;
				res = read_proc_mem( rrs +ii.imm, INT8, &dw1, true);
				if (res != excNone)
					break;

				pr->regs[ ii.rt] = (dw1 & 0x80) ? (dw1 | 0xffffff00) : (dw1 & 0xff);
			}
			break;

		case opcLBU:
			{
				uint32_t i;
				res = read_proc_mem( rrs +ii.imm, INT8, &i, true);
				pr->regs[ ii.rt] = i&0xff;
			}
			break;
			
		case opcLD:
		case opcLDL:
		case opcLDR:
			res = excRI;
			break;

		case opcLH:
			{
				uint32_t dw1;
				res = read_proc_mem( rrs +ii.imm, INT16, &dw1, true);
				if (res != excNone)
					break;

				pr->regs[ ii.rt] = (dw1 &0x8000) ? (dw1 | 0xffff0000) :
						(dw1 & 0xffff);
			}
			break;
			
		case opcLHU:
			{
				uint32_t i;
				res = read_proc_mem( rrs +ii.imm, INT16, &i, true);
				pr->regs[ ii.rt] = i&0xffff;
			}
			break;
			
		case opcLL:
			{
				uint32_t llvalx;

				pr->lladdr = rrs +ii.imm;
				res = read_proc_mem( pr->lladdr, INT32, &llvalx, true);
				if (res == excNone)
				{
					pr->regs[ ii.rt] = llvalx;
					res = convert_addr( &pr->lladdr, false, true);
					RegisterLL();
					pr->llval = true;
				}
				else
					pr->lladdr = (uint32_t) -1;
			}
			break;
			
		case opcLLD:
			res = excRI;
			break;

		case opcLUI:
			pr->regs[ ii.rt] = ii.imm << 16;
			break;
			
		case opcLW:
			res = read_proc_mem( rrs +ii.imm, INT32, &pr->regs[ ii.rt], true);
			break;
					
		case opcLWL: 
			{
				uint32_t i1, dw1;
				res = read_proc_mem( i1=(rrs + ii.imm) & ~0x3, INT32, &dw1, true);
			
				if (res != excNone)
				switch (i1 & 3)
				{
					case 0: 
						pr->regs[ ii.rt] = dw1;
						break;
					case 1:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xff) | (dw1 << 8);
						break;
					case 2:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xffff) | (dw1 << 16);
						break;
					case 3:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xffffff) | (dw1 << 24);
						break;
				}
			}
			break;
		 
		case opcLWR:
			{
				uint32_t i1, dw1;
				res = read_proc_mem( (i1=(rrs +ii.imm)) & ~0x3, INT32, &dw1, true);
			
				if (res != excNone)
				switch (i1 & 0x3) 
				{
					case 0:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xffffff00) | ((dw1 >> 24) & 0xff);
						break;
					case 1:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xffff0000) | ((dw1 >> 16) & 0xffff);
						break;
					case 2:
						pr->regs[ ii.rt] =
							(pr->regs[ ii.rt] & 0xff000000) | ((dw1 >> 8) & 0xffffff);
						break;
					case 3:
						pr->regs[ ii.rt] = dw1;
						break;
				}
			}
			break;
			
		case opcLWU:
			res = excRI;
			break;
			
		case opcSB:
			res = write_proc_mem( rrs +ii.imm, INT8, pr->regs[ ii.rt], true);
			break;
			
		case opcSC:
			UnregisterLL();
			if (!pr->llval)
				pr->regs[ ii.rt] = 0;
			else
			{
				uint32_t dw1, dw2;
				pr->llval = false;
				dw1 = dw2 = rrs +ii.imm;

				res = convert_addr( &dw1, false, true);
				if (res == excNone)
				{
					if (dw1 != pr->lladdr)
						pr->regs[ ii.rt] = 0; /* this is undefined */
					else
					{
						res = write_proc_mem( dw2, INT32, pr->regs[ ii.rt], true);
						if (res == excNone)
							pr->regs[ ii.rt] = 1;
					}
				}
			}
			break;
			
		case opcSCD:
		case opcSD:
		case opcSDL:
		case opcSDR:
			res = excRI;
			break;

		case opcSH:
			res = write_proc_mem( rrs +ii.imm, INT16, pr->regs[ ii.rt], true);
			break;

		case opcSW:
			res = write_proc_mem( rrs +ii.imm, INT32, pr->regs[ ii.rt], true);
			break;
			
		case opcSWL:
			{
				uint32_t i1, dw1;
				res = read_proc_mem( i1=((rrs + ii.imm)) & ~0x3, INT32, &dw1, true);
			
				if (res != excNone)
				{
					switch (i1 & 0x3)
					{
						case 0: 
							dw1 = pr->regs[ ii.rt];
							break;
						case 1:
							dw1 = (dw1 & 0xff000000) | ((pr->regs[ ii.rt] >> 8) & 0xffffff);
							break;
						case 2:
							dw1 = (dw1 & 0xffff0000) | ((pr->regs[ ii.rt] >> 16) & 0xffff);
							break;
						case 3:
							dw1 = (dw1 & 0xffffff00) | ((pr->regs[ ii.rt] >> 24) & 0xff);
							break;
					}
				
					res = write_proc_mem( i1 & ~0x3, INT32, dw1, true);
				}
			}
			break;
			
		case opcSWR:
			{
				uint32_t i1, dw1;
				res = read_proc_mem( i1=((rrs + ii.imm)) & ~0x3, INT32, &dw1, true);
			
				if (res != excNone)
				{
					switch (i1 & 0x3)
					{
						case 0: 
							dw1 = (dw1 & 0xffffff) | (pr->regs[ ii.rt] << 24);
							break;
						case 1:
							dw1 = (dw1 & 0xffff) | (pr->regs[ ii.rt] << 16);
							break;
						case 2:
							dw1 = (dw1 & 0xff) | (pr->regs[ ii.rt] << 8);
							break;
						case 3:
							dw1 = pr->regs[ ii.rt];
							break;
					}
				
					res = write_proc_mem( i1 & ~0x3, INT32, dw1, true);
				}
			}
			break;
			
		/*
		 * traps
		 */


		case opcTEQ:
			if (rrs == rrt)
				res = excTr;
			break;
			
		case opcTEQI:
			if (rrs == ii.imm)
				res = excTr;
			break;

		case opcTGE:
			if (rrs >= rrt)
				res = excTr;
			break;

		case opcTGEI:
			if (rrs >= ii.imm)
				res = excTr;
			break;
			
		case opcTGEIU:
			if ((uint32_t)rrs >= (uint32_t)ii.imm)
				res = excTr;
			break;

		case opcTGEU:
			if ((uint32_t)rrs >= (uint32_t)rrt)
				res = excTr;
			break;
			
		case opcTLT:
			if (rrs < rrt)
				res = excTr;
			break;

		case opcTLTI:
			if (rrs < ii.imm)
				res = excTr;
			break;

		case opcTLTIU:
			if ((uint32_t)rrs < (uint32_t)ii.imm)
				res = excTr;
			break;

		case opcTLTU:
			if ((uint32_t)rrs < (uint32_t)rrt)
				res = excTr;
			break;

		case opcTNE:
			if (rrs != rrt)
				res = excTr;
			break;

		case opcTNEI:
			if (rrs != ii.imm)
				res = excTr;
			break;

		/*
		 * special instructions
		 */

		case opcCFC0:
			/* this instruction is not valid */
			break;

		case opcCFC1:
			if (cp0_status_cu1 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu1;
			}
			break;

		case opcCFC2:
			if (cp0_status_cu2 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu2;
			}
			break;

		case opcCFC3:
			if (cp0_status_cu3 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu3;
			}
			break;

		case opcCTC0:
			/* this instruction is not valid */
			break;

		case opcCTC1:
			if (cp0_status_cu1 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu1;
			}
			break;

		case opcCTC2:
			if (cp0_status_cu2 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu2;
			}
			break;

		case opcCTC3:
			if (cp0_status_cu3)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu3;
			}
			break;

		case opcERET:
			if (cp0_status_cu0 || !cp0_status_ksu || cp0_status_exl ||
				cp0_status_erl)

			{
				/* ll-break */
				pr->lladdr = (uint32_t) -1;
				pr->llval = false;
				UnregisterLL();
				
				/* delay slot test */
				if (pr->branch && errors)
					dprintf( "\nError: ERET in a delay slot\n\n");
			
				if (cp0_status_erl)
				{
					/* error level */
					pr->pcnextreg = cp0_errorepc;
					pca = pr->pcnextreg +4;
					cp0_status &= ~cp0_status_erl_mask;
				}
				else
				{
					/* exception level */
					pr->pcnextreg = cp0_epc;
					pca = pr->pcnextreg +4;
					cp0_status &= ~cp0_status_exl_mask;
				}
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				cp0_cause &= ~cp0_cause_ce_mask;
			}	
			break;

		case opcDMFC0:
		case opcDMTC0:
			res = excRI;
			break;

		case opcMFC0:
			if ((cp0_status_cu0 == 1)||
				 ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||(cp0_status_erl == 1)))
				pr->regs[ ii.rt] = pr->cp0[ ii.rd];
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;

		case opcMFHI:
			pr->regs[ ii.rd] = pr->hireg;
			break;

		case opcMFLO:
			pr->regs[ ii.rd] = pr->loreg;
			break;

		case opcMTC0:
			if ((cp0_status_cu0 == 1) ||
				 ((cp0_status_ksu == 0) || cp0_status_exl || 
				  cp0_status_erl))
			switch (ii.rd)
			{
				/* 0 */
				case CP0_Index:
					cp0_index = rrt & 0x3f;
					break;
				case CP0_Random:
					/* ignored, read-only */
					break;
				case CP0_EntryLo0:
					cp0_entrylo0 = rrt & 0x3fffffff;
					break;
				case CP0_EntryLo1:
					cp0_entrylo1 = rrt & 0x3fffffff;
					break;
				case CP0_Context:
					cp0_context = rrt & 0xfffffff0;
					break;
				case CP0_PageMask:
					cp0_pagemask = 0;
					if ((rrt == 0x0)|(rrt == 0x6000)|(rrt == 0x1e000)|(rrt == 0x7e000)|
						(rrt == 0x1fe000)|(rrt == 0x7fe000)|(rrt == 0x1ffe000))
						cp0_pagemask = rrt & cp0_pagemask_mask_mask;
					else if (errors) 
						dprintf( "\nMTC0: Invalid value for PageMask\n");
					break;
				case CP0_Wired:
					cp0_random = 47;
					cp0_wired = rrt & 0x3f;
					if (cp0_wired > 47) 
						dprintf( "\nMTC0: Invalid value for Wired\n");
					break;
				case CP0_Res1:
					/* ignored, reserved */
					break;
				/* 8 */
				case CP0_BadVAddr:
					/* ignored, read-only */
					break;
				case CP0_Count:
					cp0_count = rrt;
					break;
				case CP0_EntryHi:
					cp0_entryhi = rrt & 0xfffff0ff;
					break;
				case CP0_Compare:
					cp0_compare = rrt;
					cp0_cause &= ~(1 << cp0_cause_ip7_shift);
					break;
				case CP0_Status:
					cp0_status = rrt & 0xff77ff1f;
					break;
				case CP0_Cause:
					cp0_cause &= ~(cp0_cause_ip0_mask | cp0_cause_ip1_mask);
					cp0_cause |= rrt & (cp0_cause_ip0_mask | cp0_cause_ip1_mask);
					break;
				case CP0_EPC:
					cp0_epc = rrt;
					break;
				case CP0_PRId:
					/* ignored, read-only */
					break;
				/* 16 */
				case CP0_Config:
					/* ignore for simulation */
					cp0_config = rrt & 0xffffefff;
					break;
				case CP0_LLAddr:
					cp0_lladdr = rrt;
					break;
				case CP0_WatchLo:
				case CP0_WatchHi:
				case CP0_XContext:
					/* ignored ? in 32bit MIPS */
					break;
				case CP0_Res2:
				case CP0_Res3:
				case CP0_Res4:
				/* 24 */
				case CP0_Res5:
				case CP0_Res6:
					/* ignored, reserved */
					break;
				case CP0_ECC:
					/* ignored for simulation */
					cp0_ecc = (rrt & cp0_ecc_ecc_mask) << cp0_ecc_ecc_shift;
					break;
				case CP0_CacheErr:
					/* ignored, read-only */
					break;
				case CP0_TagLo:
					cp0_taglo = rrt;
				case CP0_TagHi:
					cp0_taghi = rrt;
					break;
				case CP0_ErrorEPC:
					cp0_errorepc = rrt;
					break;
				case CP0_Res7:
					/* ignored */
					break;
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				cp0_cause &= ~cp0_cause_ce_mask;
			}
			break;

		case opcMTC1:
			if (cp0_status_cu1 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu1;
			}
			break;

		case opcMTC2:
			if (cp0_status_cu2 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu2;
			}
			break;

		case opcMTC3:
			if (cp0_status_cu3 == 1)

			{
				/* ignored */
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
				pr->cp0[ CP0_Cause] |= cp0_cause_ce_cu3;
			}
			break;

		case opcDMTC1:
		case opcDMTC2:
			case opcDMTC3:
			/* 64-bit instructions */
			/* raise exception */
			res = excRI;
		 break;

		case opcSDC1:
		case opcSDC2:
			/* 64-bit instructions */
			/* raise exception */
			res = excRI;
			break;

		case opcMTHI:
			pr->hireg = rrs;
			break;
			
		case opcMTLO:
			pr->loreg = rrs;
			break;
			
		case opcSYNC:
			/* no synchronisation is needed */
			break;
			
		case opcSYSCALL:
			res = excSys;
			break;

		case opcRES:
			res = excRI;
			break;

		case opcQRES:
			/* quiet reserved */
			break;

		case opcTLBP:
			if ((cp0_status_cu0 == 1) || (cp0_status_ksu == 0) ||
				(cp0_status_exl == 1) || (cp0_status_erl == 1))
			{
				uint32_t xvpn2, xasid;
				int i;

				cp0_index = 1 << cp0_index_p_shift;

				xvpn2 = cp0_entryhi & cp0_entryhi_vpn2_mask;
				xasid = cp0_entryhi & cp0_entryhi_asid_mask;
				for (i=0; i<48; i++)
				if ( (pr->tlb[ i].vpn2 == xvpn2) &&
					(pr->tlb[ i].global || (pr->tlb[ i].asid == xasid)))
				{
					cp0_index = i;
					break;
				}
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;
			
		case opcTLBR:
			if ((cp0_status_cu0 == 1) ||
				 (cp0_status_ksu == 0) || (cp0_status_exl == 1) ||
				 (cp0_status_erl == 1))
			{
				int i;
				i = cp0_index_index;

				if (i > 47)
				{
					dprintf( "\nTLBR: Invalid value in Index\n");
					pr->cp0[ CP0_PageMask] = 0; pr->cp0[ CP0_EntryHi]	= 0;
					pr->cp0[ CP0_EntryLo0] = 0; pr->cp0[ CP0_EntryLo1] = 0;
				}
				else
				{
					cp0_pagemask = (~pr->tlb[ i].mask) & 0x01ffe000;
					cp0_entryhi = pr->tlb[ i].vpn2 | pr->tlb[ i].asid;
					cp0_entrylo0 = (pr->tlb[ i].pg[ 0].pfn >> 6)| 
						 (pr->tlb[ i].pg[ 0].cohh << 3) |
						 ((pr->tlb[ i].pg[ 0].dirty ? 1 : 0) << 2) |
						 ((pr->tlb[ i].pg[ 0].valid ? 1 : 0) << 1) |
						 (pr->tlb[ i].global ? 1 : 0);
					cp0_entrylo1 = (pr->tlb[ i].pg[ 1].pfn >> 6) | 
						 (pr->tlb[ i].pg[ 1].cohh << 3) |
						 ((pr->tlb[ i].pg[ 1].dirty ? 1 : 0) << 2) |
						 ((pr->tlb[ i].pg[ 1].valid ? 1 : 0) << 1) |
						 (pr->tlb[ i].global ? 1 : 0);
				}
			}
			else
			{
				/* coprocessor unusable */
				res = excCpU;
				pr->cp0[ CP0_Cause] &= ~cp0_cause_ce_mask;
			}
			break;
			
		case opcTLBWI:
			TLBW( CP0_Index, &res);
			break;
			
		case opcTLBWR:
			TLBW( CP0_Random, &res);
			break;

		case opcBREAK:
			if (remote_gdb_conn)
				gdb_handle_event( 8);
			else
				res = excBp;
			break;

		case opcWAIT:
			pr->pcnextreg = pr->pcreg;
			pr->stdby = true;
			break;

		/*
		 * machine debugging instructions
		 */

		case opcDVAL:
			dprintf( "\nDebug: value %Xh (%dd)\n\n", pr->regs[ 4], pr->regs[ 4]);
			break;
			
		case opcDTRC:
			if (!totrace) 
			{
				reg_view();
				dprintf( "\n");
			}
			update_deb();
			totrace = true;
			break;
			
		case opcDTRO:
			totrace = false;
			break;
			
		case opcDRV:
			dprintf( "\nDebug: register view\n");
			reg_view();
			dprintf( "\n");
			break;
			
		case opcDHLT:
			if (totrace) dprintf( "\nMachine halt.\n\n");
			tohalt = true;
			break;

		case opcDINT:
			interactive = true;
			break;
	}

	/* branch test */
	if ((pr->branch == 2)||(pr->branch == 0)) pr->excaddr = pr->pcreg;
	
	/* pc update  */
	if (res == excNone)
	{
		pr->pcreg = pr->pcnextreg;
		pr->pcnextreg = pca;
	}
	
	/* reg0 control */
	pr->regs[ 0] = 0;

	return res;
}


/* exception control */
static void
exception_handle( enum exc res)

{
	bool tlb_refill = false;

	/* convert TLB Refill exceptions */
	if ((res == excTLBLR) || (res == excTLBSR))
	{
		tlb_refill = true;
		if (res == excTLBLR)
			res = excTLBL;
		else
			res = excTLBS;
	}
	
	pr->stdby = false;
	
	/* user info and register filling */
	if (totrace)
		dprintf( "\nRaised exception: %s\n\n", excText[ res]);
	cp0_cause &= ~cp0_cause_exccode_mask;
	cp0_cause |= res << cp0_cause_exccode_shift;
		
	/* exception branch control */
	cp0_cause &= ~cp0_cause_bd_mask;
	if (pr->branch == 1) cp0_cause |= cp0_cause_bd_mask;

	cp0_epc = pr->excaddr;
	if ((res == excInt) && (pr->branch != 2)) cp0_epc = pr->pcreg;
		
	/* exception vector base address */
	if (cp0_status_bev)
	{
		/* booting time */
		if (res != excReset) pr->pcreg = 0xbfc00200;
		else pr->pcreg = 0xbfc00000;
	}
	else
	{
		/* normal time */
		if (res != excReset) pr->pcreg = 0x80000000;
		else pr->pcreg = 0xbfc00000;
	}
	
	/* exception vector offsets */
	if (cp0_status_exl || !tlb_refill)
		pr->pcreg += EXCEPTION_OFFSET;
	pr->pcnextreg = pr->pcreg +4;
		
	/* turn to kernel mode */
	cp0_status |= cp0_status_exl_mask;
}


/** manage - Processor management.
 *
 * Reacts on interrupt requests, updates internal timer, random register etc.
 */
static void
manage( enum exc res)

{
	/* test for interrupt request */
	if (	(res == excNone) &&
		!cp0_status_exl && !cp0_status_erl && cp0_status_ie &&
		((cp0_cause & cp0_status) & cp0_cause_ip_mask) != 0)
		res = excInt;
	
	/* exception control */
	if (res != excNone)
		exception_handle( res);

	/* increase counter */
	cp0_count_count++;

	/* decrease random register */
	if (cp0_random-- == 0)
		cp0_random = 47;
	if (cp0_random < cp0_wired)
		cp0_random = 47;

	/* timer control */
	if (cp0_count_count == cp0_compare_compare)
		/* generating interrupt request */
		cp0_cause |= 1 << cp0_cause_ip7_shift;
}


/* instruction - Tries to simulate one instruction.
 *
 * The are three main parts to simulate the instruction: read the instruction
 * opcore, decode, execute and perform debug output.
 */
static void
instruction( enum exc *res)

{
	TInstrInfo ii;

	/* reading instruction code */
	*res = read_proc_ins( pr->pcreg, &ii.icode, true);
	if (*res == excNone)
	{
		char modif_regs[ 1024];
		uint32_t old_pcreg = pr->pcreg;
		
		/* decoding instruction code */
		decode_instr( &ii);
		
		/* executing instruction */
		*res = execute( &ii);
		
		/* view changes */
		if (totrace && iregch)
			modified_regs_dump( 1024, modif_regs);
		else
			modif_regs[ 0] = 0;
		
		/* instruction disassembling */
		if (totrace)
			iview( old_pcreg, &ii, true, modif_regs);
	}
}


/* step - Simulates one step of the processor.
 *
 * This is often one instruction.
 */
void step() 

{
	enum exc res = excNone;
	
	/* instruction execute */
	if (!pr->stdby)
		instruction( &res);
	
	/* processor control */
	manage( res);
	
	if (pr->stdby)
		pr->w_cycles++;
	else
	{
		if ((cp0_status_ksu == 0)||(cp0_status_exl == 1)||
				(cp0_status_erl == 1))
			pr->k_cycles++;
		else
			pr->u_cycles++;
	}
	
	if (pr->branch != 0)
		pr->branch--;
}
