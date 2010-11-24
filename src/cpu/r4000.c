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
#include "../debug/breakpoint.h"
#include "../debug/debug.h"
#include "../debug/gdb.h"
#include "../device/device.h"
#include "../device/machine.h"
#include "../assert.h"
#include "../endian.h"
#include "../env.h"
#include "../fault.h"
#include "../main.h"
#include "../text.h"
#include "../utils.h"
#include "r4000.h"

/** Sign bit */
#define SBIT32   UINT32_C(0x80000000)

/** Initial state */
#define HARD_RESET_STATUS         (cp0_status_erl_mask | cp0_status_bev_mask)
#define HARD_RESET_START_ADDRESS  UINT64_C(0xffffffffbfc00000)
#define HARD_RESET_PROC_ID        UINT32_C(0x00000400)
#define HARD_RESET_CAUSE          0
#define HARD_RESET_WATCHLO        0
#define HARD_RESET_WATCHHI        0
#define HARD_RESET_CONFIG         0
#define HARD_RESET_RANDOM         47
#define HARD_RESET_WIRED          0

#define EXCEPTION_OFFSET  UINT64_C(0x0180)

#define TRAP(expr) \
	if (expr) \
		res = excTr;

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

/** Initialize simulation environment
 *
 */
void cpu_init(cpu_t *cpu, unsigned int procno)
{
	ASSERT(cpu != NULL);
	
	ptr64_t start_address;
	start_address.ptr = HARD_RESET_START_ADDRESS;
	
	/* Initially set all members to zero */
	memset(cpu, 0, sizeof(cpu_t));
	
	cpu->procno = procno;
	cpu_set_pc(cpu, start_address);
	
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
void cpu_set_pc(cpu_t *cpu, ptr64_t value)
{
	ASSERT(cpu != NULL);
	
	cpu->pc.ptr = value.ptr;
	cpu->pc_next.ptr = value.ptr + 4;
}

/** Address traslation through the TLB table
 *
 * See tlb_look_t definition
 *
 */
static tlb_look_t tlb_look(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr)
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
static void fill_tlb_error(cpu_t *cpu, ptr64_t addr)
{
	ASSERT(cpu != NULL);
	
	cp0_badvaddr(cpu).val = addr.ptr;
	
	cp0_context(cpu).val &= cp0_context_ptebase_mask;
	cp0_context(cpu).val |= (addr.ptr >> cp0_context_addr_shift) & ~cp0_context_res1_mask;
	
	cp0_entryhi(cpu).val &= cp0_entryhi_asid_mask;
	cp0_entryhi(cpu).val |= addr.ptr & cp0_entryhi_vpn2_mask;
}

/** Fill registers as the Address error exception occures
 *
 */
static void fill_addr_error(cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if (noisy) {
		cp0_badvaddr(cpu).val = addr.ptr;
		cp0_context(cpu).val &= ~cp0_context_badvpn2_mask;  /* Undefined */
		cp0_entryhi(cpu).val &= ~cp0_entryhi_vpn2_mask;     /* Undefined */
	}
}

/** Search through TLB and generates apropriate exception
 *
 */
static exc_t tlb_hit(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr,
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
		
		return excTLBR;
	case TLBL_INVALID:
		if (noisy) {
			cpu->tlb_invalid++;
			fill_tlb_error(cpu, virt);
		}
		
		return excTLB;
	case TLBL_MODIFIED:
		if (noisy) {
			cpu->tlb_modified++;
			fill_tlb_error(cpu, virt);
		}
		
		return excMod;
	}
	
	return excNone;
}

/** The user mode address conversion
 *
 */
static exc_t convert_addr_user(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_USER_MODE(cpu));
	
	/* Test bit 31 or lookup in TLB */
	if ((virt.lo & SBIT32) != 0) {
		fill_addr_error(cpu, virt, noisy);
		return excAddrError;
	}
	
	/* useg */
	return tlb_hit(cpu, virt, phys, wr, noisy);
}

/** The supervisor mode address conversion
 *
 */
static exc_t convert_addr_supervisor(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_SUPERVISOR_MODE(cpu));
	
	if (virt.lo < UINT32_C(0x80000000))  /* suseg */
		return tlb_hit(cpu, virt, phys, wr, noisy);
	
	if (virt.lo < UINT32_C(0xc0000000)) {
		fill_addr_error(cpu, virt, noisy);
		return excAddrError;
	}
	
	if (virt.lo < UINT32_C(0xe0000000))  /* ssseg */
		return tlb_hit(cpu, virt, phys, wr, noisy);
	
	/* virt > UINT32_C(0xe0000000) */
	fill_addr_error(cpu, virt, noisy);
	return excAddrError;
}

/** The kernel mode address conversion
 *
 */
static exc_t convert_addr_kernel(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_KERNEL_MODE(cpu));
	
	if (virt.lo < UINT32_C(0x80000000)) {  /* kuseg */
		if (!cp0_status_erl(cpu))
			return tlb_hit(cpu, virt, phys, wr, noisy);
		
		return excNone;
	}
	
	if (virt.lo < UINT32_C(0xa0000000)) {  /* kseg0 */
		*phys = virt.lo - UINT32_C(0x80000000);
		return excNone;
	}
	
	if (virt.lo < UINT32_C(0xc0000000)) {  /* kseg1 */
		*phys = virt.lo - UINT32_C(0xa0000000);
		return excNone;
	}
	
	if (virt.lo < UINT32_C(0xe0000000))  /* kseg2 */
		return tlb_hit(cpu, virt, phys, wr, noisy);
	
	/* virt > UINT32_C(0xe0000000) (kseg3) */
	return tlb_hit(cpu, virt, phys, wr, noisy);
}

/** The conversion of virtual addresses
 *
 * @param write Write access.
 * @param noisy Fill apropriate processor registers
 *              if the address is incorrect.
 *
 */
exc_t convert_addr(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool write,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	if (CPU_USER_MODE(cpu))
		return convert_addr_user(cpu, virt, phys, write, noisy);
	
	if (CPU_SUPERVISOR_MODE(cpu))
		return convert_addr_supervisor(cpu, virt, phys, write, noisy);
	
	return convert_addr_kernel(cpu, virt, phys, write, noisy);
}

/** Test for correct alignment (16 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static inline exc_t align_test16(cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x01U) != 0) {
		fill_addr_error(cpu, addr, noisy);
		return excAddrError;
	}
	
	return excNone;
}

/** Test for correct alignment (32 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static inline exc_t align_test32(cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x03U) != 0) {
		fill_addr_error(cpu, addr, noisy);
		return excAddrError;
	}
	
	return excNone;
}

/** Test for correct alignment (64 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static inline exc_t align_test64(cpu_t *cpu, ptr64_t addr, bool noisy)
{
	ASSERT(cpu != NULL);
	
	if ((addr.ptr & 0x07U) != 0) {
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
 * @param virt  Virtual memory address
 * @param phys  Physical memory address
 * @param noisy Generate exception in case of invalid operation
 *
 */
static inline exc_t access_mem(cpu_t *cpu, acc_mode_t mode, ptr64_t virt,
    ptr36_t *phys, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	exc_t res = convert_addr(cpu, virt, phys, mode == AM_WRITE, noisy);
	
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
				return excWATCH;
		}
	}
	
	return res;
}

/** Preform read operation from the virtual memory (8 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static inline exc_t cpu_read_mem8(cpu_t *cpu, ptr64_t addr, uint8_t *val,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	ptr36_t phys;
	exc_t res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	if (res != excNone)
		return res;
	
	*val = physmem_read8(cpu, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (16 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static inline exc_t cpu_read_mem16(cpu_t *cpu, ptr64_t addr, uint16_t *val,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	exc_t res = align_test16(cpu, addr, noisy);
	if (res != excNone)
		return res;
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	if (res != excNone)
		return res;
	
	*val = physmem_read16(cpu, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (32 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
exc_t cpu_read_mem32(cpu_t *cpu, ptr64_t addr, uint32_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	exc_t res = align_test32(cpu, addr, noisy);
	if (res != excNone)
		return res;
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	if (res != excNone)
		return res;
	
	*val = physmem_read32(cpu, phys, true);
	return res;
}

/** Perform write operation to the virtual memory (8 bits)
 *
 */
static inline exc_t cpu_write_mem8(cpu_t *cpu, ptr64_t addr, uint8_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	ptr36_t phys;
	exc_t res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		physmem_write8(cpu, phys, value, true);
		break;
	case excAddrError:
		res = excAdES;
		break;
	case excTLB:
		res = excTLBS;
		break;
	case excTLBR:
		res = excTLBSR;
		break;
	default:
		break;
	}
	
	return res;
}

/** Perform write operation to the virtual memory (16 bits)
 *
 */
static inline exc_t cpu_write_mem16(cpu_t *cpu, ptr64_t addr, uint16_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	exc_t res = align_test16(cpu, addr, noisy);
	if (res != excNone)
		return res;
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		physmem_write16(cpu, phys, value, true);
		break;
	case excAddrError:
		res = excAdES;
		break;
	case excTLB:
		res = excTLBS;
		break;
	case excTLBR:
		res = excTLBSR;
		break;
	default:
		break;
	}
	
	return res;
}

/** Perform write operation to the virtual memory (32 bits)
 *
 */
static inline exc_t cpu_write_mem32(cpu_t *cpu, ptr64_t addr, uint32_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	exc_t res = align_test32(cpu, addr, noisy);
	if (res != excNone)
		return res;
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		physmem_write32(cpu, phys, value, true);
		break;
	case excAddrError:
		res = excAdES;
		break;
	case excTLB:
		res = excTLBS;
		break;
	case excTLBR:
		res = excTLBSR;
		break;
	default:
		break;
	}
	
	return res;
}

/** Read an instruction
 *
 * Does not change the value if an exception occurs.
 * Fetching instructions (on R4000) does not trigger
 * the WATCH exception.
 *
 */
exc_t cpu_read_ins(cpu_t *cpu, ptr64_t addr, uint32_t *icode, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(icode != NULL);
	
	exc_t res = align_test32(cpu, addr, noisy);
	if (res != excNone)
		return res;
	
	ptr36_t phys;
	res = access_mem(cpu, AM_FETCH, addr, &phys, noisy);
	switch (res) {
	case excNone:
		*icode = physmem_read32(cpu, phys, true);
		break;
	case excAddrError:
		res = excAdEL;
		break;
	case excTLB:
		res = excTLBL;
		break;
	case excTLBR:
		res = excTLBLR;
		break;
	default:
		break;
	}
	
	if ((noisy) && (res != excNone) && (cpu->branch == BRANCH_NONE))
		cpu->excaddr = cpu->pc;
	
	return res;
}

/** Find an activated memory breakpoint
 *
 * Find an activated memory breakpoint which would be hit for specified
 * memory address and access conditions.
 *
 * @param addr         Address, where the breakpoint can be hit.
 * @param size         Size of the access operation.
 * @param access_flags Specifies the access condition, under the breakpoint
 *                     will be hit.
 *
 * @return Found breakpoint structure or NULL if there is not any.
 *
 */
static inline void physmem_breakpoint_find(ptr36_t addr, len36_t size,
    access_t access_type)
{
	physmem_breakpoint_t *breakpoint;
	
	for_each(physmem_breakpoints, breakpoint, physmem_breakpoint_t) {
		if (breakpoint->addr + breakpoint->size < addr)
			continue;
		
		if (breakpoint->addr > addr + size)
			continue;
		
		if ((access_type & breakpoint->access_flags) != 0) {
			physmem_breakpoint_hit(breakpoint, access_type);
			break;
		}
	}
}

static inline physmem_area_t* find_physmem_area(ptr36_t addr)
{
	physmem_area_t *area;
	
	for_each(physmem_areas, area, physmem_area_t) {
		ptr36_t area_start = area->start;
		ptr36_t area_end = area_start + area->size;
		
		if ((addr >= area_start) && (addr < area_end))
			return area;
	}
	
	return NULL;
}

static inline uint8_t devmem_read8(cpu_t *cpu, ptr36_t addr)
{
	uint8_t val = (uint8_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read8)
			dev->type->read8(cpu, dev, addr, &val);
	
	return val;
}

static inline uint16_t devmem_read16(cpu_t *cpu, ptr36_t addr)
{
	uint16_t val = (uint16_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read16)
			dev->type->read16(cpu, dev, addr, &val);
	
	return val;
}

static inline uint32_t devmem_read32(cpu_t *cpu, ptr36_t addr)
{
	uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read32)
			dev->type->read32(cpu, dev, addr, &val);
	
	return val;
}

/** Physical memory read (8 bits)
 *
 * Read 8 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param cpu       Processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint8_t physmem_read8(cpu_t *cpu, ptr36_t addr, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/*
	 * No memory area found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (area == NULL)
		return devmem_read8(cpu, addr);
	
	/* Check for memory read breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 1, ACCESS_READ);
	
	return convert_uint8_t_endian(*((uint8_t *)
	    (area->data + (addr - area->start))));
}

/** Physical memory read (16 bits)
 *
 * Read 16 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param cpu       Processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint16_t physmem_read16(cpu_t *cpu, ptr36_t addr, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/*
	 * No memory area found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (area == NULL)
		return devmem_read16(cpu, addr);
	
	/* Check for memory read breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 2, ACCESS_READ);
	
	return convert_uint16_t_endian(*((uint16_t *)
	    (area->data + (addr - area->start))));
}

/** Physical memory read (32 bits)
 *
 * Read 32 bits from memory. At first try to read from configured memory
 * regions, then from a device which supports reading at specified address.
 * If the address is not contained in any memory region and no device
 * manages it, the default value is returned.
 *
 * @param cpu       Processor which wants to read.
 * @param addr      Address of memory to be read.
 * @param protected If true the memory breakpoints check is performed.
 *
 * @return Value in specified piece of memory or the default memory value
 *         if the address is not valid.
 *
 */
uint32_t physmem_read32(cpu_t *cpu, ptr36_t addr, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/*
	 * No memory area found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (area == NULL)
		return devmem_read32(cpu, addr);
	
	/* Check for memory read breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 4, ACCESS_READ);
	
	return convert_uint32_t_endian(*((uint32_t *)
	    (area->data + (addr - area->start))));
}

/** Load Linked and Store Conditional control
 *
 */
static inline void sc_control(ptr36_t addr)
{
	sc_item_t *sc_item = (sc_item_t *) sc_list.head;
	
	while (sc_item != NULL) {
		cpu_t *sc_cpu = sc_item->cpu;
		
		if (sc_cpu->lladdr == addr) {
			sc_cpu->llbit = false;
			
			sc_item_t *tmp = sc_item;
			sc_item = (sc_item_t *) sc_item->item.next;
			
			list_remove(&sc_list, &tmp->item);
			safe_free(tmp);
		} else
			sc_item = (sc_item_t *) sc_item->item.next;
	}
}

static inline bool devmem_write8(cpu_t *cpu, ptr36_t addr, uint8_t val)
{
	bool written = false;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL)) {
		if (dev->type->write8) {
			dev->type->write8(cpu, dev, addr, val);
			written = true;
		}
	}
	
	return written;
}

static inline bool devmem_write16(cpu_t *cpu, ptr36_t addr, uint16_t val)
{
	bool written = false;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL)) {
		if (dev->type->write16) {
			dev->type->write16(cpu, dev, addr, val);
			written = true;
		}
	}
	
	return written;
}

static inline bool devmem_write32(cpu_t *cpu, ptr36_t addr, uint32_t val)
{
	bool written = false;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL)) {
		if (dev->type->write32) {
			dev->type->write32(cpu, dev, addr, val);
			written = true;
		}
	}
	
	return written;
}

/** Physical memory write (8 bits)
 *
 * Write 8 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param cpu       Processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write8(cpu_t *cpu, ptr36_t addr, uint8_t val, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (area == NULL)
		return devmem_write8(cpu, addr, val);
	
	/* Writting to ROM? */
	if ((!area->writable) && (protected))
		return false;
	
	sc_control(addr);
	
	/* Check for memory write breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 1, ACCESS_WRITE);
	
	*((uint8_t *) (area->data + (addr - area->start))) =
	    convert_uint8_t_endian(val);
	
	return true;
}

/** Physical memory write (16 bits)
 *
 * Write 16 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param cpu       Processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write16(cpu_t *cpu, ptr36_t addr, uint16_t val, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (area == NULL)
		return devmem_write16(cpu, addr, val);
	
	/* Writting to ROM? */
	if ((!area->writable) && (protected))
		return false;
	
	sc_control(addr);
	
	/* Check for memory write breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 2, ACCESS_WRITE);
	
	*((uint16_t *) (area->data + (addr - area->start))) =
	    convert_uint16_t_endian(val);
	
	return true;
}

/** Physical memory write (32 bits)
 *
 * Write 32 bits of data to memory at given address. At first try to find
 * a configured memory region which contains the given address. If there
 * is no such region, try to write to appropriate device.
 *
 * @param cpu       Processor which wants to write.
 * @param addr      Address of the memory.
 * @param val       Data to be written.
 * @param protected False to allow writing to ROM memory and ignore
 *                  the memory breakpoints check.
 *
 * @return False if there is no configured memory region and device for
 *         given address or the memory is ROM with protected parameter
 *         set to true.
 *
 */
bool physmem_write32(cpu_t *cpu, ptr36_t addr, uint32_t val, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (area == NULL)
		return devmem_write32(cpu, addr, val);
	
	/* Writting to ROM? */
	if ((!area->writable) && (protected))
		return false;
	
	sc_control(addr);
	
	/* Check for memory write breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 4, ACCESS_WRITE);
	
	*((uint32_t *) (area->data + (addr - area->start))) =
	    convert_uint32_t_endian(val);
	
	return true;
}

/** Assert the specified interrupt
 *
 */
void cpu_interrupt_up(cpu_t *cpu, unsigned int no)
{
	ASSERT(cpu != NULL);
	ASSERT(no < INTR_COUNT);
	
	cp0_cause(cpu).val |= 1 << (cp0_cause_ip0_shift + no);
	cpu->intr[no]++;
}

/* Deassert the specified interrupt
 *
 */
void cpu_interrupt_down(cpu_t *cpu, unsigned int no)
{
	ASSERT(cpu != NULL);
	ASSERT(no < INTR_COUNT);
	
	cp0_cause(cpu).val &= ~(1 << (cp0_cause_ip0_shift + no));
}

/** Update the copy of registers
 *
 */
static void cpu_update_debug(cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	memcpy(cpu->old_regs, cpu->regs, sizeof(cpu->regs));
	memcpy(cpu->old_cp0, cpu->cp0, sizeof(cpu->cp0));
	
	cpu->old_loreg = cpu->loreg;
	cpu->old_hireg = cpu->hireg;
}

/** Perform the multiplication of two integers
 *
 */
static void multiply(cpu_t *cpu, uint32_t a, uint32_t b, bool sign)
{
	ASSERT(cpu != NULL);
	
	/* Quick test */
	if ((a == 0) || (b == 0)) {
		cpu->hireg.lo = 0;
		cpu->loreg.lo = 0;
		return;
	}
	
	if (sign) {
		/* Signed multiplication */
		int64_t r = ((int64_t) ((int32_t) a)) * ((int64_t) ((int32_t) b));
		
		/* Result */
		cpu->loreg.lo = ((uint64_t) r) & UINT32_C(0xffffffff);
		cpu->hireg.lo = ((uint64_t) r) >> 32;
	} else {
		/* Unsigned multiplication */
		uint64_t r = ((uint64_t) a) * ((uint64_t) b);
		
		/* Result */
		cpu->loreg.lo = r & UINT32_C(0xffffffff);
		cpu->hireg.lo = r >> 32;
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
	ASSERT(cpu != NULL);
	ASSERT(res != NULL);
	
	if ((cp0_status_cu0(cpu) == 1)
	    || ((cp0_status_ksu(cpu) == 0)
	    || (cp0_status_exl(cpu) == 1)
	    || (cp0_status_erl(cpu) == 1))) {
		
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
	} else {
		/* Coprocessor unusable */
		*res = excCpU;
		cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	}
}

/** Execute the instruction specified by the opcode
 *
 * A really huge one, isn't it.
 *
 */
static exc_t execute(cpu_t *cpu, instr_info_t ii)
{
	ASSERT(cpu != NULL);
	
	exc_t res = excNone;
	
	ptr64_t pca;
	pca.ptr = cpu->pc_next.ptr + 4;
	
	reg64_t urrs = cpu->regs[ii.rs];
	reg64_t urrt = cpu->regs[ii.rt];
	
	uint8_t utmp8;
	uint16_t utmp16;
	uint32_t utmp32;
	uint32_t utmp32b;
	uint64_t utmp64;
	ptr64_t addr;
	
	switch (ii.opcode) {
	
	/*
	 * Aritmetic, logic, shifts
	 */
	
	case opcADD:
		utmp32 = urrs.lo + urrt.lo;
		
		if (!((urrs.lo ^ urrt.lo) & SBIT32) && ((urrs.lo ^ utmp32) & SBIT32)) {
			res = excOv;
			break;
		}
		
		cpu->regs[ii.rd].lo = utmp32;
		break;
	case opcADDI:
		utmp32 = urrs.lo + ii.imm;
		
		if (!((urrs.lo ^ ii.imm) & SBIT32) && ((ii.imm ^ utmp32) & SBIT32)) {
			res = excOv;
			break;
		}
		
		cpu->regs[ii.rt].lo = utmp32;
		break;
	case opcADDIU:
		cpu->regs[ii.rt].lo = urrs.lo + ii.imm;
		break;
	case opcADDU:
		cpu->regs[ii.rd].lo = urrs.lo + urrt.lo;
		break;
	case opcAND:
		cpu->regs[ii.rd].lo = urrs.lo & urrt.lo;
		break;
	case opcANDI:
		cpu->regs[ii.rt].lo = urrs.lo & (ii.imm & UINT32_C(0x0000ffff));
		break;
	case opcCLO:
		utmp32 = 0;
		utmp32b = urrs.lo;
		
		while ((utmp32b & SBIT32) && (utmp32 < 32)) {
			utmp32++;
			utmp32b <<= 1;
		}
		
		cpu->regs[ii.rd].lo = utmp32;
		break;
	case opcCLZ:
		utmp32 = 0;
		utmp32b = urrs.lo;
		
		while ((!(utmp32b & SBIT32)) && (utmp32 < 32)) {
			utmp32++;
			utmp32b <<= 1;
		}
		
		cpu->regs[ii.rd].lo = utmp32;
		break;
	case opcDADD:
	case opcDADDI:
	case opcDADDIU:
	case opcDADDU:
	case opcDDIV:
	case opcDDIVU:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcDIV:
		if (urrt.lo == 0) {
			cpu->loreg.lo = 0;
			cpu->hireg.lo = 0;
		} else {
			cpu->loreg.lo = (uint32_t) (((int32_t) urrs.lo) / ((int32_t) urrt.lo));
			cpu->hireg.lo = (uint32_t) (((int32_t) urrs.lo) % ((int32_t) urrt.lo));
		}
		break;
	case opcDIVU:
		if (urrt.lo == 0) {
			cpu->loreg.lo = 0;
			cpu->hireg.lo = 0;
		} else {
			cpu->loreg.lo = urrs.lo / urrt.lo;
			cpu->hireg.lo = urrs.lo % urrt.lo;
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
		res = excRI;
		break;
	case opcMADD:
		utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		multiply(cpu, urrs.lo, urrt.lo, true);
		utmp64 += ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		cpu->hireg.lo = utmp64 >> 32;
		cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff);
		break;
	case opcMADDU:
		utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		multiply(cpu, urrs.lo, urrt.lo, false);
		utmp64 += ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		cpu->hireg.lo = utmp64 >> 32;
		cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff);
		break;
	case opcMSUB:
		utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		multiply(cpu, urrs.lo, urrt.lo, true);
		utmp64 -= ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		cpu->hireg.lo = utmp64 >> 32;
		cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff);
		break;
	case opcMSUBU:
		utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		multiply(cpu, urrs.lo, urrt.lo, false);
		utmp64 -= ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
		cpu->hireg.lo = utmp64 >> 32;
		cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff);
		break;
	case opcMUL:
		utmp64 = ((uint64_t) urrs.lo) * ((uint64_t) urrt.lo);
		cpu->regs[ii.rd].lo = utmp64 & UINT32_C(0xffffffff);
		break;
	case opcMOVN:
		if (urrt.lo != 0)
			cpu->regs[ii.rd].lo = urrs.lo;
		break;
	case opcMOVZ:
		if (urrt.lo == 0)
			cpu->regs[ii.rd].lo = urrs.lo;
		break;
	case opcMULT:
		multiply(cpu, urrs.lo, urrt.lo, true);
		break;
	case opcMULTU:
		multiply(cpu, urrs.lo, urrt.lo, false);
		break;
	case opcNOR:
		cpu->regs[ii.rd].lo = ~(urrs.lo | urrt.lo);
		break;
	case opcOR:
		cpu->regs[ii.rd].lo = urrs.lo | urrt.lo;
		break;
	case opcORI:
		cpu->regs[ii.rt].lo = urrs.lo | (ii.imm & UINT32_C(0x0000ffff));
		break;
	case opcSLL:
		cpu->regs[ii.rd].lo = urrt.lo << ii.shift;
		break;
	case opcSLLV:
		cpu->regs[ii.rd].lo = urrt.lo << (urrs.lo & UINT32_C(0x0000001f));
		break;
	case opcSLT:
		cpu->regs[ii.rd].lo = ((int32_t) urrs.lo) < ((int32_t) urrt.lo);
		break;
	case opcSLTI:
		cpu->regs[ii.rt].lo = ((int32_t) urrs.lo) < ((int32_t) ii.imm);
		break;
	case opcSLTIU:
		cpu->regs[ii.rt].lo = urrs.lo < ii.imm;
		break;
	case opcSLTU:
		cpu->regs[ii.rd].lo = urrs.lo < urrt.lo;
		break;
	case opcSRA:
		cpu->regs[ii.rd].lo = (uint32_t) (((int32_t) urrt.lo) >> ii.shift);
		break;
	case opcSRAV:
		cpu->regs[ii.rd].lo = (uint32_t) (((int32_t) urrt.lo) >> (urrs.lo & UINT32_C(0x0000001f)));
		break;
	case opcSRL:
		cpu->regs[ii.rd].lo = urrt.lo >> ii.shift;
		break;
	case opcSRLV:
		cpu->regs[ii.rd].lo = urrt.lo >> (urrs.lo & UINT32_C(0x0000001f));
		break;
	case opcSUB:
		utmp32 = urrs.lo - urrt.lo;
		
		if (((urrs.lo ^ urrt.lo) & SBIT32) && ((urrs.lo ^ utmp32) & SBIT32)) {
			res = excOv;
			break;
		}
		
		cpu->regs[ii.rd].lo = utmp32;
		break;
	case opcSUBU:
		cpu->regs[ii.rd].lo = urrs.lo - urrt.lo;
		break;
	case opcXOR:
		cpu->regs[ii.rd].lo = urrs.lo ^ urrt.lo;
		break;
	case opcXORI:
		cpu->regs[ii.rt].lo = urrs.lo ^ (ii.imm & UINT32_C(0x0000ffff));
		break;
	
	/*
	 * Branches and jumps
	 */
	case opcBC0FL:
	case opcBC1FL:
	case opcBC2FL:
	case opcBC3FL:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))) {
			/* Ignore - always false */
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0F:
	case opcBC1F:
	case opcBC2F:
	case opcBC3F:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))) {
			/* Ignore - always false */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0TL:
	case opcBC1TL:
	case opcBC2TL:
	case opcBC3TL:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))) {
			/* Ignore - always true */
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBC0T:
	case opcBC1T:
	case opcBC2T:
	case opcBC3T:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))) {
			/* Ignore - always true */
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcBEQ:
		if (urrs.lo == urrt.lo) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBEQL:
		if (urrs.lo == urrt.lo) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcBGEZAL:
		cpu->regs[31].val = cpu->pc.ptr + 8;
		/* no break */
	case opcBGEZ:
		if (!(urrs.lo & SBIT32)) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBGEZALL:
		cpu->regs[31].val = cpu->pc.ptr + 8;
		/* no break */
	case opcBGEZL:
		if (!(urrs.lo & SBIT32)) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcBGTZ:
		if (((int32_t) urrs.lo) > 0) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBGTZL:
		if (((int32_t) urrs.lo) > 0) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcBLEZ:
		if (((int32_t) urrs.lo) <= 0) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBLEZL:
		if (((int32_t) urrs.lo) <= 0) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcBLTZAL:
		cpu->regs[31].val = cpu->pc_next.ptr + 4;
		/* no break */
	case opcBLTZ:
		if (urrs.lo & SBIT32) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBLTZALL:
		cpu->regs[31].val = cpu->pc_next.ptr + 4;
		/* no break */
	case opcBLTZL:
		if (urrs.lo & SBIT32) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcBNE:
		if (urrs.lo != urrt.lo) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		}
		break;
	case opcBNEL:
		if (urrs.lo != urrt.lo) {
			pca.ptr = cpu->pc_next.ptr + (((int32_t) ii.imm) << TARGET_SHIFT);
			cpu->branch = BRANCH_COND;
		} else {
			cpu->pc_next.ptr += 4;
			pca.ptr = cpu->pc_next.ptr + 4;
		}
		break;
	case opcJAL:
		cpu->regs[31].val = cpu->pc_next.ptr + 4;
		/* no break */
	case opcJ:
		pca.ptr = (cpu->pc_next.ptr & TARGET_COMB) | (ii.imm << TARGET_SHIFT);
		cpu->branch = BRANCH_COND;
		break;
	case opcJALR:
		cpu->regs[ii.rd].val = cpu->pc_next.ptr + 4;
		/* no break */
	case opcJR:
		pca.ptr = urrs.val;
		cpu->branch = BRANCH_COND;
		break;
	
	/*
	 * Load, store
	 */
	case opcLB:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem8(cpu, addr, &utmp8, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = (utmp8 & 0x80U) ?
			    ((uint64_t) utmp8 | UINT64_C(0xffffffffffffff00)) : utmp8;
		break;
	case opcLBU:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem8(cpu, addr, &utmp8, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = utmp8;
		break;
	case opcLD:
	case opcLDL:
	case opcLDR:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcLH:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem16(cpu, addr, &utmp16, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = (utmp16 & 0x8000U) ?
			    ((uint64_t) utmp16 | UINT64_C(0xffffffffffff0000)) : utmp16;
		break;
	case opcLHU:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem16(cpu, addr, &utmp16, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = utmp16;
		break;
	case opcLL:
		/* Compute virtual target address
		   and issue read operation */
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {  /* If the read operation has been successful */
			/* Store the value */
			cpu->regs[ii.rt].val = utmp32;
			
			/* Since we need physical address to track, issue the
			   address conversion. It can't fail now. */
			ptr36_t phys;
			convert_addr(cpu, addr, &phys, false, false);
			
			/* Register address for tracking. */
			register_sc(cpu);
			cpu->llbit = true;
			cpu->lladdr = phys;
		} else {
			/* Invalid address; Cancel the address tracking */
			unregister_sc(cpu);
			cpu->llbit = false;
		}
		break;
	case opcLLD:
		/* 64-bit instruction */
		res = excRI;
		break;
	case opcLUI:
		cpu->regs[ii.rt].lo = ii.imm << 16;
		break;
	case opcLW:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = utmp32;
		break;
	case opcLWL:
		addr.ptr = (urrs.val + ((int32_t) ii.imm)) & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = (urrs.val + ((int32_t) ii.imm)) & 0x03U;
			
			cpu->regs[ii.rt].lo &= shift_tab_left[index].mask;
			cpu->regs[ii.rt].lo |= utmp32 << shift_tab_left[index].shift;
		}
		break;
	case opcLWR:
		addr.ptr = (urrs.val + ((int32_t) ii.imm)) & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = (urrs.val + ((int32_t) ii.imm)) & 0x03U;
			
			cpu->regs[ii.rt].lo &= shift_tab_right[index].mask;
			cpu->regs[ii.rt].lo |= (utmp32 >> shift_tab_right[index].shift)
			    & (~shift_tab_right[index].mask);
		}
		break;
	case opcLWU:
		res = excRI;
		break;
	case opcSB:
		addr.ptr = urrs.val + ((int32_t) ii.imm);
		res = cpu_write_mem8(cpu, addr, (uint8_t) cpu->regs[ii.rt].val, true);
		break;
	case opcSC:
		if (!cpu->llbit) {
			/* If we are not tracking LL-SC,
			   then SC has to fail */
			cpu->regs[ii.rt].val = 0;
		} else {
			/* We do track LL-SC address */
			
			/* Compute trarget address */
			addr.ptr = urrs.val + ((int32_t) ii.imm);
			
			/* Perform the write operation */
			res = cpu_write_mem32(cpu, addr, cpu->regs[ii.rt].lo, true);
			if (res == excNone) {
				/* The operation has been successful,
				   write the result, but... */
				cpu->regs[ii.rt].val = 1;
				
				/* ...we are too polite if LL and SC addresses differ.
				   In such a case, the behaviour of SC is undefined.
				   Let's check that. */
				ptr36_t phys;
				convert_addr(cpu, addr, &phys, false, false);
				
				/* sc_addr now contains physical target address */
				if (phys != cpu->lladdr) {
					/* LL and SC addresses do not match ;( */
					if (errors)
						alert("R4000: LL/SC addresses do not match");
				}
			} else {
				/* Error writing the target */
			}
			
			/* SC always stops LL-SC address tracking */
			unregister_sc(cpu);
			cpu->llbit = false;
		}
		break;
	case opcSCD:
	case opcSD:
	case opcSDL:
	case opcSDR:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcSH:
		addr.ptr = urrs.val + ((int16_t) ii.imm);
		res = cpu_write_mem16(cpu, addr, (uint16_t) cpu->regs[ii.rt].val, true);
		break;
	case opcSW:
		addr.ptr = urrs.val + ((int16_t) ii.imm);
		res = cpu_write_mem32(cpu, addr, cpu->regs[ii.rt].lo, true);
		break;
	case opcSWL:
		addr.ptr = (urrs.val + ((int32_t) ii.imm)) & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = (urrs.val + ((int32_t) ii.imm)) & 0x03U;
			
			utmp32 &= shift_tab_left_store[index].mask;
			utmp32 |= (cpu->regs[ii.rt].lo >> shift_tab_left_store[index].shift)
			    & (~shift_tab_left_store[index].mask);
			
			res = cpu_write_mem32(cpu, addr, utmp32, true);
		}
		break;
	case opcSWR:
		addr.ptr = (urrs.val + ((int32_t) ii.imm)) & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = (urrs.val + ((int32_t) ii.imm)) & 0x03U;
			
			utmp32 &= shift_tab_right_store[index].mask;
			utmp32 |= cpu->regs[ii.rt].lo << shift_tab_right_store[index].shift;
			
			res = cpu_write_mem32(cpu, addr, utmp32, true);
		}
		break;
	
	/*
	 * Traps
	 */
	case opcTEQ:
		TRAP(urrs.lo == urrt.lo);
		break;
	case opcTEQI:
		TRAP(urrs.lo == ii.imm);
		break;
	case opcTGE:
		TRAP(((int32_t) urrs.lo) >= ((int32_t) urrt.lo));
		break;
	case opcTGEI:
		TRAP(((int32_t) urrs.lo) >= ((int32_t) ii.imm));
		break;
	case opcTGEIU:
		TRAP(urrs.lo >= ii.imm);
		break;
	case opcTGEU:
		TRAP(urrs.lo >= urrt.lo);
		break;
	case opcTLT:
		TRAP(((int32_t) urrs.lo) < ((int32_t) urrt.lo));
		break;
	case opcTLTI:
		TRAP(((int32_t) urrs.lo) < ((int32_t) ii.imm));
		break;
	case opcTLTIU:
		TRAP(urrs.lo < ii.imm);
		break;
	case opcTLTU:
		TRAP(urrs.lo < urrt.lo);
		break;
	case opcTNE:
		TRAP(urrs.lo != urrt.lo);
		break;
	case opcTNEI:
		TRAP(urrs.lo != ii.imm);
		break;
	
	/*
	 * Special instructions
	 */
	case opcCFC0:
		/* This instruction is not valid */
		break;
	case opcCFC1:
		if (cp0_status_cu1(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu1;
		}
		break;
	case opcCFC2:
		if (cp0_status_cu2(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu2;
		}
		break;
	case opcCFC3:
		if (cp0_status_cu3(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu3;
		}
		break;
	case opcCTC0:
		/* This instruction is not valid */
		break;
	case opcCTC1:
		if (cp0_status_cu1(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu1;
		}
		break;
	case opcCTC2:
		if (cp0_status_cu2(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu2;
		}
		break;
	case opcCTC3:
		if (cp0_status_cu3(cpu)) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu3;
		}
		break;
	case opcERET:
		if ((cp0_status_cu0(cpu))
		    || (!cp0_status_ksu(cpu))
		    || (cp0_status_exl(cpu))
		    || (cp0_status_erl(cpu))) {
			/* ERET breaks LL-SC address tracking */
			cpu->llbit = false;
			unregister_sc(cpu);
			
			/* Delay slot test */
			if ((cpu->branch != BRANCH_NONE) && (errors))
				alert("R4000: ERET in a branch delay slot");
			
			if (cp0_status_erl(cpu)) {
				/* Error level */
				cpu->pc_next.ptr = cp0_errorepc(cpu).val;
				pca.ptr = cpu->pc_next.ptr + 4;
				cp0_status(cpu).val &= ~cp0_status_erl_mask;
			} else {
				/* Exception level */
				cpu->pc_next.ptr = cp0_epc(cpu).val;
				pca.ptr = cpu->pc_next.ptr + 4;
				cp0_status(cpu).val &= ~cp0_status_exl_mask;
			}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcMFC0:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1))) {
			cpu->regs[ii.rt] = cpu->cp0[ii.rd];
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcMFHI:
		cpu->regs[ii.rd] = cpu->hireg;
		break;
	case opcMFLO:
		cpu->regs[ii.rd] = cpu->loreg;
		break;
	case opcMTC0:
		if ((cp0_status_cu0(cpu) == 1)
		    || ((cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu))
		    || (cp0_status_erl(cpu)))) {
			switch (ii.rd) {
			/* 0 */
			case cp0_Index:
				cp0_index(cpu).val = urrt.val & UINT32_C(0x003f);
				break;
			case cp0_Random:
				/* Ignored, read-only */
				break;
			case cp0_EntryLo0:
				cp0_entrylo0(cpu).val = urrt.val & UINT32_C(0x3fffffff);
				break;
			case cp0_EntryLo1:
				cp0_entrylo1(cpu).val = urrt.val & UINT32_C(0x3fffffff);
				break;
			case cp0_Context:
				cp0_context(cpu).val = urrt.val & UINT32_C(0xfffffff0);
				break;
			case cp0_PageMask:
				cp0_pagemask(cpu).val = 0;
				if ((urrt.val == UINT32_C(0x0))
				    || (urrt.val == UINT32_C(0x6000))
				    || (urrt.val == UINT32_C(0x1e000))
				    || (urrt.val == UINT32_C(0x7e000))
				    || (urrt.val == UINT32_C(0x1fe000))
				    || (urrt.val == UINT32_C(0x7fe000))
				    || (urrt.val == UINT32_C(0x1ffe000)))
					cp0_pagemask(cpu).val = urrt.val & cp0_pagemask_mask_mask;
				else if (errors)
					alert("R4000: Invalid value for PageMask (MTC0)");
				break;
			case cp0_Wired:
				cp0_random(cpu).val = 47;
				cp0_wired(cpu).val = urrt.val & UINT32_C(0x003f);
				if (cp0_wired(cpu).val > 47)
					alert("R4000: Invalid value for Wired (MTC0)");
				break;
			case cp0_Res1:
				/* Ignored, reserved */
				break;
			/* 8 */
			case cp0_BadVAddr:
				/* Ignored, read-only */
				break;
			case cp0_Count:
				cp0_count(cpu).val = urrt.val;
				break;
			case cp0_EntryHi:
				cp0_entryhi(cpu).val = urrt.val & UINT32_C(0xfffff0ff);
				break;
			case cp0_Compare:
				cp0_compare(cpu).val = urrt.val;
				cp0_cause(cpu).val &= ~(1 << cp0_cause_ip7_shift);
				break;
			case cp0_Status:
				cp0_status(cpu).val = urrt.val & UINT32_C(0xff77ff1f);
				break;
			case cp0_Cause:
				cp0_cause(cpu).val &= ~(cp0_cause_ip0_mask | cp0_cause_ip1_mask);
				cp0_cause(cpu).val |= urrt.val & (cp0_cause_ip0_mask | cp0_cause_ip1_mask);
				break;
			case cp0_EPC:
				cp0_epc(cpu).val = urrt.val;
				break;
			case cp0_PRId:
				/* Ignored, read-only */
				break;
			/* 16 */
			case cp0_Config:
				/* Ignored for simulation */
				cp0_config(cpu).val = urrt.val & UINT32_C(0xffffefff);
				break;
			case cp0_LLAddr:
				cp0_lladdr(cpu).val = urrt.val;
				break;
			case cp0_WatchLo:
				cp0_watchlo(cpu).val = urrt.val & (~cp0_watchlo_res_mask);
				cpu->waddr = cp0_watchhi_paddr1(cpu);
				cpu->waddr <<= (32 - cp0_watchlo_paddr0_shift);
				cpu->waddr |= cp0_watchlo_paddr0(cpu);
				break;
			case cp0_WatchHi:
				cp0_watchhi(cpu).val = urrt.val & (~cp0_watchhi_res_mask);
				cpu->waddr = cp0_watchhi_paddr1(cpu);
				cpu->waddr <<= (32 - cp0_watchlo_paddr0_shift);
				cpu->waddr |= cp0_watchlo_paddr0(cpu);
				break;
			case cp0_XContext:
				/* Ignored in 32bit MIPS */
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
				cp0_ecc(cpu).val = (urrt.val & cp0_ecc_ecc_mask) << cp0_ecc_ecc_shift;
				break;
			case cp0_CacheErr:
				/* Ignored, read-only */
				break;
			case cp0_TagLo:
				cp0_taglo(cpu).val = urrt.val;
				break;
			case cp0_TagHi:
				cp0_taghi(cpu).val = urrt.val;
				break;
			case cp0_ErrorEPC:
				cp0_errorepc(cpu).val = urrt.val;
				break;
			case cp0_Res7:
				/* Ignored */
				break;
			}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcMTC1:
		if (cp0_status_cu1(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu1;
		}
		break;
	case opcMTC2:
		if (cp0_status_cu2(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu2;
		}
		break;
	case opcMTC3:
		if (cp0_status_cu3(cpu) == 1) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu3;
		}
		break;
	case opcSDC1:
	case opcSDC2:
		/* 64-bit instructions */
		res = excRI;
		break;
	case opcMTHI:
		cpu->hireg.val = urrs.val;
		break;
	case opcMTLO:
		cpu->loreg.val = urrs.val;
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
		if ((cp0_status_cu0(cpu) == 1)
		    || (cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1)) {
			cp0_index(cpu).val = 1 << cp0_index_p_shift;
			uint32_t xvpn2 = cp0_entryhi(cpu).val & cp0_entryhi_vpn2_mask;
			uint32_t xasid = cp0_entryhi(cpu).val & cp0_entryhi_asid_mask;
			unsigned int i;
			
			for (i = 0; i < TLB_ENTRIES; i++) {
				if ((cpu->tlb[i].vpn2 == xvpn2) &&
				    ((cpu->tlb[i].global) || (cpu->tlb[i].asid == xasid))) {
					cp0_index(cpu).val = i;
					break;
				}
			}
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcTLBR:
		if ((cp0_status_cu0(cpu) == 1)
		    || (cp0_status_ksu(cpu) == 0)
		    || (cp0_status_exl(cpu) == 1)
		    || (cp0_status_erl(cpu) == 1)) {
			uint32_t i = cp0_index_index(cpu);
			
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
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		}
		break;
	case opcTLBWI:
		TLBW(cpu, false, &res);
		break;
	case opcTLBWR:
		TLBW(cpu, true, &res);
		break;
	case opcBREAK:
		res = excBp;
		break;
	case opcWAIT:
		cpu->pc_next.ptr = cpu->pc.ptr;
		cpu->stdby = true;
		break;
	case opcNOP:
		break;
	
	/*
	 * Machine debugging instructions
	 */
	case opcDVAL:
		alert("Debug: Value %#" PRIx64 " = %" PRIu64 " (%" PRId64 ")",
		    cpu->regs[4].val, cpu->regs[4].val, cpu->regs[4].val);
		break;
	case opcDTRC:
		if (!totrace) {
			reg_view(cpu);
			printf("\n");
		}
		cpu_update_debug(cpu);
		totrace = true;
		break;
	case opcDTRO:
		totrace = false;
		break;
	case opcDRV:
		alert("Debug: register view");
		reg_view(cpu);
		printf("\n");
		break;
	case opcDHLT:
		if (totrace)
			alert("Debug: Machine halt");
		tohalt = true;
		break;
	case opcDINT:
		interactive = true;
		break;
	
	/*
	 * Unimplemented instructions
	 */
	case opcSPECIAL:
	case opcBCOND:
	case opcSPECIAL2:
	case opcCACHE:
	case opcCOP0:
	case opcCOP1:
	case opcCOP2:
	case opcCOP3:
	case opcDMFC0:
	case opcDMFC1:
	case opcDMFC2:
	case opcDMFC3:
	case opcDMTC0:
	case opcDMTC1:
	case opcDMTC2:
	case opcDMTC3:
	case opcLDC1:
	case opcLDC2:
	case opcLWC1:
	case opcLWC2:
	case opcMFC1:
	case opcMFC2:
	case opcMFC3:
	case opcSWC1:
	case opcSWC2:
	case opcUNIMP:
	case opcIllegal:
	case opcBC:
	case opcC0:
		res = excRI;
		break;
	}
	
	/* Branch test */
	if ((cpu->branch == BRANCH_COND) || (cpu->branch == BRANCH_NONE))
		cpu->excaddr.ptr = cpu->pc.ptr;
	
	/* PC update */
	if (res == excNone) {
		cpu->pc.ptr = cpu->pc_next.ptr;
		cpu->pc_next.ptr = pca.ptr;
	}
	
	/* Register 0 is hardwired zero */
	cpu->regs[0].val = 0;
	
	return res;
}

/** Change the processor state according to the exception type
 *
 */
static void handle_exception(cpu_t *cpu, exc_t res)
{
	ASSERT(cpu != NULL);
	
	bool tlb_refill = false;
	
	/* Convert TLB Refill exceptions */
	if ((res == excTLBLR) || (res == excTLBSR)) {
		tlb_refill = true;
		if (res == excTLBLR)
			res = excTLBL;
		else
			res = excTLBS;
	}
	
	cpu->stdby = false;
	
	/* User info and register fill */
	if (totrace)
		alert("Raised exception: %s", txt_exc[res]);
	
	cp0_cause(cpu).val &= ~cp0_cause_exccode_mask;
	cp0_cause(cpu).val |= res << cp0_cause_exccode_shift;
	
	/* Exception branch control */
	cp0_cause(cpu).val &= ~cp0_cause_bd_mask;
	if (cpu->branch == BRANCH_PASSED)
		cp0_cause(cpu).val |= cp0_cause_bd_mask;
	
	if (!cp0_status_exl(cpu)) {
		cp0_epc(cpu).val = cpu->excaddr.ptr;
		if ((res == excInt) && (cpu->branch != BRANCH_COND))
			cp0_epc(cpu).val = cpu->pc.ptr;
	}
	
	ptr64_t exc_pc;
	/* Exception vector base address */
	if (cp0_status_bev(cpu)) {
		/* Boot time */
		if (res != excReset)
			exc_pc.ptr = UINT64_C(0xffffffffbfc00200);
		else
			exc_pc.ptr = UINT64_C(0xffffffffbfc00000);
	} else {
		/* Normal time */
		if (res != excReset)
			exc_pc.ptr = UINT64_C(0xffffffff80000000);
		else
			exc_pc.ptr = UINT64_C(0xffffffffbfc00000);
	}
	
	/* Exception vector offsets */
	if ((cp0_status_exl(cpu)) || (!tlb_refill))
		exc_pc.ptr += EXCEPTION_OFFSET;
	
	cpu_set_pc(cpu, exc_pc);
	
	/* Switch to kernel mode */
	cp0_status(cpu).val |= cp0_status_exl_mask;
}

/** React on interrupt requests, updates internal timer, random register
 *
 */
static void manage(cpu_t *cpu, exc_t res)
{
	ASSERT(cpu != NULL);
	
	/* Test for interrupt request */
	if ((res == excNone)
	    && (!cp0_status_exl(cpu))
	    && (!cp0_status_erl(cpu))
	    && (cp0_status_ie(cpu))
	    && ((cp0_cause(cpu).val & cp0_status(cpu).val) & cp0_cause_ip_mask) != 0)
		res = excInt;
	
	/* Exception control */
	if (res != excNone)
		handle_exception(cpu, res);
	
	/* Increase counter */
	cp0_count(cpu).val++;
	
	/* Decrease random register */
	if (cp0_random(cpu).val-- == 0)
		cp0_random(cpu).val = 47;
	
	if (cp0_random(cpu).val < cp0_wired(cpu).val)
		cp0_random(cpu).val = 47;
	
	/* Timer control */
	if (cp0_count(cpu).val == cp0_compare(cpu).val)
		/* Generate interrupt request */
		cp0_cause(cpu).val |= 1 << cp0_cause_ip7_shift;
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
	ASSERT(cpu != NULL);
	
	/* Fetch instruction */
	instr_info_t ii;
	*res = cpu_read_ins(cpu, cpu->pc, &ii.icode, true);
	
	if (*res == excNone) {
		/* Decode instruction */
		decode_instr(&ii);
		
		/* Execute instruction */
		ptr64_t old_pc = cpu->pc;
		*res = execute(cpu, ii);
		
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
	ASSERT(cpu != NULL);
	
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
		if (CPU_KERNEL_MODE(cpu))
			cpu->k_cycles++;
		else
			cpu->u_cycles++;
	}
	
	/* Branch delay slot control */
	if (cpu->branch > BRANCH_NONE)
		cpu->branch--;
}
