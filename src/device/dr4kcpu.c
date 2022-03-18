/*
 * Copyright (c) 2003-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  MIPS R4000 microprocessor (32 bit part) device
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "dr4kcpu.h"
#include "device.h"
#include "cpu/general_cpu.h"
#include "cpu/mips_r4000/cpu.h"
#include "../debug/debug.h"
#include "../debug/breakpoint.h"
#include "../fault.h"
#include "../main.h"
#include "../utils.h"


/** Get first available CPU id
 *
 * @return First available CPU id or MAX_CPUS if no
 *         more CPU slots available.
 *
 */
static unsigned int dcpu_get_free_id(void)
{
	unsigned int c;
	unsigned int id_mask = 0;
	device_t *dev = NULL;
	
	while (dev_next(&dev, DEVICE_FILTER_PROCESSOR))
		id_mask |= 1 << ((r4k_cpu_t *) dev->data)->procno;
	
	for (c = 0; c < MAX_CPUS; c++, id_mask >>= 1)
		if (!(id_mask & 1))
			return c;
	
	return MAX_CPUS;
}

/** Initialization
 *
 */
static bool dr4kcpu_init(token_t *parm, device_t *dev)
{
	unsigned int id = dcpu_get_free_id();
	
	if (id == MAX_CPUS) {
		error("Maximum CPU count exceeded (%u)", MAX_CPUS);
		return false;
	}
	
	r4k_cpu_t *cpu = safe_malloc_t(r4k_cpu_t);
	r4k_cpu_init(cpu, id);
	dev->data = cpu;
	
	return true;
}

/** Info command implementation
 *
 */
static bool dr4kcpu_info(token_t *parm, device_t *dev)
{
	printf("R4000\n");
	return true;
}

/** Stat command implementation
 *
 */
static bool dr4kcpu_stat(token_t *parm, device_t *dev)
{
	r4k_cpu_t *cpu = (r4k_cpu_t *) dev->data;
	
	printf("[Total cycles      ] [In kernel space   ] [In user space     ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
	    (uint64_t) cpu->k_cycles + cpu->u_cycles + cpu->w_cycles,
	    cpu->k_cycles, cpu->u_cycles);
	
	printf("[Wait cycles       ] [TLB Refill exc    ] [TLB Invalid exc   ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
	    cpu->w_cycles, cpu->tlb_refill, cpu->tlb_invalid);
	
	printf("[TLB Modified exc  ] [Interrupt 0       ] [Interrupt 1       ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
	    cpu->tlb_modified, cpu->intr[0], cpu->intr[1]);
	
	printf("[Interrupt 2       ] [Interrupt 3       ] [Interrupt 4       ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n\n",
	    cpu->intr[2], cpu->intr[3], cpu->intr[4]);
	
	printf("[Interrupt 5       ] [Interrupt 6       ] [Interrupt 7       ]\n");
	printf("%20" PRIu64 " %20" PRIu64 " %20" PRIu64 "\n",
	    cpu->intr[5], cpu->intr[6], cpu->intr[7]);
	
	return true;
}

/** Cp0d command implementation
 *
 */
static bool dr4kcpu_cp0d(token_t *parm, device_t *dev)
{
	if (parm->ttype == tt_end) {
		cp0_dump_all((r4k_cpu_t *) dev->data);
		return true;
	}
	if (parm->ttype != tt_uint) {
		error("Register number required");
		return false;
	}
	uint64_t no = parm_uint(parm);
	if (no >= MAX_CP0_REGISTERS) {
		error("Out of range (0..%u)", MAX_CP0_REGISTERS - 1);
		return false;
	}
	
	cp0_dump((r4k_cpu_t *) dev->data, no);
	return true;
}

/** Tlbd command implementation
 *
 */
static bool dr4kcpu_tlbd(token_t *parm, device_t *dev)
{
	tlb_dump((r4k_cpu_t *) dev->data);
	return true;
}

/** Md command implementation
 *
 */
static bool dr4kcpu_md(token_t *parm, device_t *dev)
{
	uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
	uint64_t _cnt = parm_uint(parm);
	
	if (!virt_range(_addr)) {
		error("Virtual address out of range");
		return false;
	}
	
	if (!virt_range(_cnt)) {
		error("Count out of virtual memory range");
		return false;
	}
	
	if (!virt_range(_addr + _cnt * 4)) {
		error("Count exceeds virtual memory range");
		return false;
	}
	
	ptr64_t addr;
	len64_t cnt;
	len64_t i;
	
	for (addr.ptr = _addr, cnt = _cnt, i = 0;
	    i < cnt; addr.ptr += 4, i++) {
		if ((i & 0x03U) == 0)
			printf("  %#018" PRIx64 "    ", addr.ptr);
		
		uint32_t val;
		exc_t res = cpu_read_mem32((r4k_cpu_t *) dev->data, addr, &val, false);
		
		if (res == excNone)
			printf("%08" PRIx32 " ", val);
		else
			printf("xxxxxxxx ");
		
		if ((i & 0x03U) == 3)
			printf("\n");
	}
	
	if (i != 0)
		printf("\n");
	
	return true;
}

/** Id command implementation
 *
 */
static bool dr4kcpu_id(token_t *parm, device_t *dev)
{
	uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
	uint64_t _cnt = parm_uint(parm);
	
	if (!virt_range(_addr)) {
		error("Virtual address out of range");
		return false;
	}
	
	if (!virt_range(_cnt)) {
		error("Count out of virtual memory range");
		return false;
	}
	
	if (!virt_range(_addr + _cnt * 4)) {
		error("Count exceeds virtual memory range");
		return false;
	}
	
	ptr64_t addr;
	len64_t cnt;
	
	for (addr.ptr = _addr, cnt = _cnt; cnt > 0;
	    addr.ptr += 4, cnt--) {
		r4k_instr_t instr;
		// FIXME
		exc_t res = excNone;
		// exc_t res = cpu_read_ins((r4k_cpu_t *) dev->data, addr, &instr.val, false);
		
		if (res != excNone)
			instr.val = 0;
		
		idump((r4k_cpu_t *) dev->data, addr, instr, false);
	}
	
	return true;
}

/** Rd command implementation
 *
 */
static bool dr4kcpu_rd(token_t *parm, device_t *dev)
{
	reg_dump((r4k_cpu_t *) dev->data);
	return true;
}

/** Goto command implementation
 *
 */
static bool dr4kcpu_goto(token_t *parm, device_t *dev)
{
	r4k_cpu_t *cpu = (r4k_cpu_t *) dev->data;
	uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
	
	if (!virt_range(_addr)) {
		error("Virtual address out of range");
		return false;
	}
	
	ptr64_t addr;
	addr.ptr = _addr;
	
	r4k_cpu_set_pc(cpu, addr);
	return true;
}

/** Break command implementation
 *
 */
static bool dr4kcpu_break(token_t *parm, device_t *dev)
{
	r4k_cpu_t *cpu = (r4k_cpu_t *) dev->data;
	uint64_t _addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
	
	if (!virt_range(_addr)) {
		error("Virtual address out of range");
		return false;
	}
	
	ptr64_t addr;
	// Extend the address as the user will not enter it in 64bit mode
	// when the emulated CPU is 32bit.
	addr.ptr = UINT64_C(0xffffffff00000000) | _addr;
	
	breakpoint_t *bp = breakpoint_init(addr,
	    BREAKPOINT_KIND_SIMULATOR);
	list_append(&cpu->bps, &bp->item);
	
	return true;
}

/** Bd command implementation
 *
 */
static bool dr4kcpu_bd(token_t *parm, device_t *dev)
{
	r4k_cpu_t *cpu = (r4k_cpu_t *) dev->data;
	breakpoint_t *bp;
	
	printf("[address ] [hits              ] [kind    ]\n");
	
	for_each(cpu->bps, bp, breakpoint_t) {
		const char *kind = (bp->kind == BREAKPOINT_KIND_SIMULATOR)
		    ? "Simulator" : "Debugger";
		
		printf("%#018" PRIx64 " %20" PRIu64 " %s\n",
		    bp->pc.ptr, bp->hits, kind);
	}
	
	return true;
}

/** Br command implementation
 *
 */
static bool dr4kcpu_br(token_t *parm, device_t *dev)
{
	r4k_cpu_t *cpu = (r4k_cpu_t *) dev->data;
	uint64_t addr = ALIGN_DOWN(parm_uint_next(&parm), 4);
	
	if (!virt_range(addr)) {
		error("Virtual address out of range");
		return false;
	}
	
	bool fnd = false;
	breakpoint_t *bp;
	for_each(cpu->bps, bp, breakpoint_t) {
		if (bp->pc.ptr == addr) {
			list_remove(&cpu->bps, &bp->item);
			safe_free(bp);
			fnd = true;
			break;
		}
	}
	
	if (!fnd) {
		error("Unknown breakpoint");
		return false;
	}
	
	return true;
}

/** Done
 *
 */
static void dr4kcpu_done(device_t *dev)
{
	safe_free(dev->name);
	safe_free(dev->data);
}

/** Execute one processor step
 *
 */
static void dr4kcpu_step(device_t *dev)
{
	r4k_cpu_step((r4k_cpu_t *) dev->data);
}

r4k_cpu_t *dcpu_find_no(unsigned int no)
{
	device_t *dev = NULL;
	
	while (dev_next(&dev, DEVICE_FILTER_PROCESSOR)) {
		r4k_cpu_t* cpu = (r4k_cpu_t *) dev->data;
		if (cpu->procno == no)
			return cpu;
	}
	
	return NULL;
}

void dcpu_interrupt_up(unsigned int cpuno, unsigned int no)
{
	r4k_cpu_t *cpu = dcpu_find_no(cpuno);
	
	if (cpu != NULL)
		r4k_cpu_interrupt_up(cpu, no);
}

void dcpu_interrupt_down(unsigned int cpuno, unsigned int no)
{
	r4k_cpu_t *cpu = dcpu_find_no(cpuno);
	
	if (cpu != NULL)
		r4k_cpu_interrupt_down(cpu, no);
}

cmd_t dr4kcpu_cmds[] = {
	{
		"init",
		(fcmd_t) dr4kcpu_init,
		DEFAULT,
		DEFAULT,
		"Initialization",
		"Initialization",
		REQ STR "pname/processor name" END
	},
	{
		"help",
		(fcmd_t) dev_generic_help,
		DEFAULT,
		DEFAULT,
		"Display this help text",
		"Display this help text",
		OPT STR "cmd/command name" END
	},
	{
		"info",
		(fcmd_t) dr4kcpu_info,
		DEFAULT,
		DEFAULT,
		"Display configuration information",
		"Display configuration information",
		NOCMD
	},
	{
		"stat",
		(fcmd_t) dr4kcpu_stat,
		DEFAULT,
		DEFAULT,
		"Display processor statistics",
		"Display processor statistics",
		NOCMD
	},
	{
		"cp0d",
		(fcmd_t) dr4kcpu_cp0d,
		DEFAULT,
		DEFAULT,
		"Dump contents of the coprocessor 0 register(s)",
		"Dump contents of the coprocessor 0 register(s)",
		OPT INT "rn/register number" END
	},
	{
		"tlbd",
		(fcmd_t) dr4kcpu_tlbd,
		DEFAULT,
		DEFAULT,
		"Dump content of TLB",
		"Dump content of TLB",
		NOCMD
	},
	{
		"md",
		(fcmd_t) dr4kcpu_md,
		DEFAULT,
		DEFAULT,
		"Dump specified TLB mapped memory block",
		"Dump specified TLB mapped memory block",
		REQ INT "saddr/starting address" NEXT
		REQ INT "size/size" END
	},
	{
		"id",
		(fcmd_t) dr4kcpu_id,
		DEFAULT,
		DEFAULT,
		"Dump instructions from specified TLB mapped memory",
		"Dump instructions from specified TLB mapped memory",
		REQ INT "saddr/starting address" NEXT
		REQ INT "cnt/count" END
	},
	{
		"rd",
		(fcmd_t) dr4kcpu_rd,
		DEFAULT,
		DEFAULT,
		"Dump contents of CPU general registers",
		"Dump contents of CPU general registers",
		NOCMD
	},
	{
		"goto",
		(fcmd_t) dr4kcpu_goto,
		DEFAULT,
		DEFAULT,
		"Go to address",
		"Go to address",
		REQ INT "addr/address" END
	},
	{
		"break",
		(fcmd_t) dr4kcpu_break,
		DEFAULT,
		DEFAULT,
		"Add code breakpoint",
		"Add code breakpoint",
		REQ INT "addr/address" END
	},
	{
		"bd",
		(fcmd_t) dr4kcpu_bd,
		DEFAULT,
		DEFAULT,
		"Dump code breakpoints",
		"Dump code breakpoints",
		NOCMD
	},
	{
		"br",
		(fcmd_t) dr4kcpu_br,
		DEFAULT,
		DEFAULT,
		"Remove code breakpoint",
		"Remove code breakpoint",
		REQ INT "addr/address" END
	},
	LAST_CMD
};

device_type_t dr4kcpu = {
	/* CPU is simulated deterministically */
	.nondet = false,
	
	/* Type name */
	.name = "dcpu",
	
	/* Brief description*/
	.brief = "MIPS R4000 processor",
	
	/* Full description */
	.full = "MIPS R4000 processor restricted to 32 bits without FPU",
	
	/* Functions */
	.done = dr4kcpu_done,
	.step = dr4kcpu_step,
	
	/* Commands */
	.cmds = dr4kcpu_cmds
};

//? rename?
static bool r4k_cpu_convert_addr(r4k_cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write){
	return r4k_convert_addr(cpu, virt, phys, write, false);
}

// surpress wrong pointer type warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

cpu_type_t r4k_cpu = {
	.interrupt_up = r4k_cpu_interrupt_up,
	.interrupt_down = r4k_cpu_interrupt_down,
	
	.convert_addr = r4k_cpu_convert_addr,
	.set_pc = r4k_cpu_set_pc	
};

#pragma GCC diagnostic pop
