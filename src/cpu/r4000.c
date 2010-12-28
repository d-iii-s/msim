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
#include "../assert.h"
#include "../endian.h"
#include "../env.h"
#include "../fault.h"
#include "../main.h"
#include "../text.h"
#include "../utils.h"
#include "instr.h"
#include "r4000.h"

/** Physical memory management
 *
 */

#define PTL2_COUNT  4096
#define PTL1_COUNT  4096

#define PTL2_SHIFT  12
#define PTL1_SHIFT  24

#define PTL2_MASK   0x0FFFU
#define PTL1_MASK   0x0FFFU

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
	uint32_t start;
	
	/* Number of physical frames */
	uint32_t count;
	
	/* Memory content */
	unsigned char *data;
} physmem_area_t;

/** Instruction implementation */
typedef exc_t (*instr_fnc_t)(cpu_t *, instr_t);

typedef struct {
	/* Physical memory area containing the frame */
	physmem_area_t *area;
	
	/* Decoded instructions */
	instr_fnc_t *decoded;
} frame_t;

typedef frame_t *ptl1_t[PTL2_COUNT];
typedef ptl1_t *ptl0_t[PTL1_COUNT];

static ptl0_t ptl0;

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

#define KSSEG_MASK  SSEG_MASK
#define KSSEG_BITS  SSEG_BITS
#define CKSSEG_MASK  CSSEG_MASK
#define CKSSEG_BITS  CSSEG_BITS

#define KSEG3_MASK  UINT32_C(0xe0000000)
#define KSEG3_BITS  UINT32_C(0xe0000000)
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
	cp0_context(cpu).val |= (addr.ptr >> cp0_context_addr_shift)
	    & ~cp0_context_res1_mask;
	
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

/** Search through TLB and generates apropriate exception (32 bits)
 *
 */
static exc_t tlb_hit32(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr,
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

/** Search through TLB and generates apropriate exception (64 bits)
 *
 */
static exc_t tlb_hit64(cpu_t *cpu, ptr64_t virt, ptr36_t *phys, bool wr,
    bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	
	ASSERT(false);
	
	return excNone;
}

/** The user mode address conversion (32 bits)
 *
 */
static exc_t convert_addr_user32(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_USER_MODE(cpu));
	
	if ((virt.lo & USEG_MASK) == USEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return excAddrError;
}

/** The user mode address conversion (64 bits)
 *
 */
static exc_t convert_addr_user64(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_USER_MODE(cpu));
	
	if ((virt.ptr & XUSEG_MASK) == XUSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return excAddrError;
}

/** The supervisor mode address conversion (32 bits)
 *
 */
static exc_t convert_addr_supervisor32(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
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
	return excAddrError;
}

/** The supervisor mode address conversion (64 bits)
 *
 */
static exc_t convert_addr_supervisor64(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
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
	return excAddrError;
}

/** The kernel mode address conversion (32 bits)
 *
 */
static exc_t convert_addr_kernel32(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_KERNEL_MODE(cpu));
	
	if ((virt.lo & KUSEG_MASK) == KUSEG_BITS) {
		if (!cp0_status_erl(cpu))
			return tlb_hit32(cpu, virt, phys, wr, noisy);
		
		return excNone;
	}
	
	if ((virt.lo & KSEG0_MASK) == KSEG0_BITS) {
		*phys = virt.lo - UINT32_C(0x80000000);
		return excNone;
	}
	
	if ((virt.lo & KSEG1_MASK) == KSEG1_BITS) {
		*phys = virt.lo - UINT32_C(0xa0000000);
		return excNone;
	}
	
	if ((virt.lo & KSSEG_MASK) == KSSEG_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	if ((virt.lo & KSEG3_MASK) == KSEG3_BITS)
		return tlb_hit32(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return excAddrError;
}

/** The kernel mode address conversion (64 bits)
 *
 */
static exc_t convert_addr_kernel64(cpu_t *cpu, ptr64_t virt, ptr36_t *phys,
    bool wr, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(phys != NULL);
	ASSERT(CPU_KERNEL_MODE(cpu));
	
	if ((virt.ptr & XKSUSEG_MASK) == XKSUSEG_BITS) {
		if (!cp0_status_erl(cpu))
			return tlb_hit64(cpu, virt, phys, wr, noisy);
		
		return excNone;
	}
	
	if ((virt.ptr & XKSSEG_MASK) == XKSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & XKPHYS_MASK) == XKPHYS_BITS)
		ASSERT(false);
	
	if ((virt.ptr & XKSEG_MASK) == XKSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.ptr & CKSEG0_MASK) == CKSEG0_BITS) {
		*phys = virt.ptr - UINT64_C(0x80000000);
		return excNone;
	}
	
	if ((virt.ptr & CKSEG1_MASK) == CKSEG1_BITS) {
		*phys = virt.ptr - UINT64_C(0xa0000000);
		return excNone;
	}
	
	if ((virt.lo & CKSSEG_MASK) == CKSSEG_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	if ((virt.lo & CKSEG3_MASK) == CKSEG3_BITS)
		return tlb_hit64(cpu, virt, phys, wr, noisy);
	
	fill_addr_error(cpu, virt, noisy);
	return excAddrError;
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
	
	if (CPU_64BIT_MODE(cpu)) {
		if (CPU_USER_MODE(cpu))
			return convert_addr_user64(cpu, virt, phys, write, noisy);
		
		if (CPU_SUPERVISOR_MODE(cpu))
			return convert_addr_supervisor64(cpu, virt, phys, write, noisy);
		
		return convert_addr_kernel64(cpu, virt, phys, write, noisy);
	} else {
		if (CPU_USER_MODE(cpu))
			return convert_addr_user32(cpu, virt, phys, write, noisy);
		
		if (CPU_SUPERVISOR_MODE(cpu))
			return convert_addr_supervisor32(cpu, virt, phys, write, noisy);
		
		return convert_addr_kernel32(cpu, virt, phys, write, noisy);
	}
}

/** Test for correct alignment (16 bits)
 *
 * Fill BadVAddr if the alignment is not correct.
 *
 */
static exc_t align_test16(cpu_t *cpu, ptr64_t addr, bool noisy)
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
static exc_t align_test32(cpu_t *cpu, ptr64_t addr, bool noisy)
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
static exc_t align_test64(cpu_t *cpu, ptr64_t addr, bool noisy)
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
static exc_t access_mem(cpu_t *cpu, acc_mode_t mode, ptr64_t virt,
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
static exc_t cpu_read_mem8(cpu_t *cpu, ptr64_t addr, uint8_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	ptr36_t phys;
	exc_t res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read8(cpu, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (16 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static exc_t cpu_read_mem16(cpu_t *cpu, ptr64_t addr, uint16_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	exc_t res = align_test16(cpu, addr, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	default:
		ASSERT(false);
	}
	
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
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read32(cpu, phys, true);
	return res;
}

/** Preform read operation from the virtual memory (64 bits)
 *
 * Does not change the value if an exception occurs.
 *
 */
static exc_t cpu_read_mem64(cpu_t *cpu, ptr64_t addr, uint64_t *val, bool noisy)
{
	ASSERT(cpu != NULL);
	ASSERT(val != NULL);
	
	exc_t res = align_test64(cpu, addr, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_READ, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdEL;
	case excTLB:
		return excTLBL;
	case excTLBR:
		return excTLBLR;
	default:
		ASSERT(false);
	}
	
	*val = physmem_read64(cpu, phys, true);
	return res;
}

/** Perform write operation to the virtual memory (8 bits)
 *
 */
static exc_t cpu_write_mem8(cpu_t *cpu, ptr64_t addr, uint8_t value, bool noisy)
{
	ASSERT(cpu != NULL);
	
	ptr36_t phys;
	exc_t res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	case excTLB:
		return excTLBS;
	case excTLBR:
		return excTLBSR;
	case excMod:
		return excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write8(cpu, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (16 bits)
 *
 */
static exc_t cpu_write_mem16(cpu_t *cpu, ptr64_t addr, uint16_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	exc_t res = align_test16(cpu, addr, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	case excTLB:
		return excTLBS;
	case excTLBR:
		return excTLBSR;
	case excMod:
		return excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write16(cpu, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (32 bits)
 *
 */
static exc_t cpu_write_mem32(cpu_t *cpu, ptr64_t addr, uint32_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	exc_t res = align_test32(cpu, addr, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	case excTLB:
		return excTLBS;
	case excTLBR:
		return excTLBSR;
	case excMod:
		return excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write32(cpu, phys, value, true);
	return res;
}

/** Perform write operation to the virtual memory (64 bits)
 *
 */
static exc_t cpu_write_mem64(cpu_t *cpu, ptr64_t addr, uint64_t value,
    bool noisy)
{
	ASSERT(cpu != NULL);
	
	exc_t res = align_test64(cpu, addr, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	default:
		ASSERT(false);
	}
	
	ptr36_t phys;
	res = access_mem(cpu, AM_WRITE, addr, &phys, noisy);
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	case excTLB:
		return excTLBS;
	case excTLBR:
		return excTLBSR;
	case excMod:
		return excMod;
	default:
		ASSERT(false);
	}
	
	physmem_write64(cpu, phys, value, true);
	return res;
}

/** Probe TLB entry
 *
 */
static exc_t TLBP(cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	if (CP0_USABLE(cpu)) {
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
		
		return excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

/** Read entry from the TLB
 *
 */
static exc_t TLBR(cpu_t *cpu)
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
		
		return excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

/** Write a new entry into the TLB
 *
 * The entry index is determined by either
 * the random register (TLBWR) or index (TLBWI).
 *
 */
static exc_t TLBW(cpu_t *cpu, bool random)
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
		
		return excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

/** SC-LL tracking
 *
 */

typedef struct {
	item_t item;
	cpu_t *cpu;
} sc_item_t;

static list_t sc_list = LIST_INITIALIZER;

/** Register current processor in LL-SC tracking list
 *
 */
static void sc_register(cpu_t *cpu)
{
	/* Ignore if already registered. */
	sc_item_t *sc_item;
	
	for_each(sc_list, sc_item, sc_item_t) {
		if (sc_item->cpu == cpu)
			return;
	}
	
	sc_item = safe_malloc_t(sc_item_t);
	item_init(&sc_item->item);
	sc_item->cpu = cpu;
	list_append(&sc_list, &sc_item->item);
}

/** Remove current processor from the LL-SC tracking list
 *
 */
static void sc_unregister(cpu_t *cpu)
{
	sc_item_t *sc_item;
	
	for_each(sc_list, sc_item, sc_item_t) {
		if (sc_item->cpu == cpu) {
			list_remove(&sc_list, &sc_item->item);
			safe_free(sc_item);
			break;
		}
	}
}

/** Implementation of instructions of R4000
 *
 * Include those instructions which are supported
 * by R4000.
 *
 */

#include "instr/_reserved.c"
#include "instr/_warning.c"
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
	instr__reserved,  /* opcSPECIAL */
	instr__reserved,  /* opcREGIMM */
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
	instr__reserved,  /* opcCOP0 */
	instr__reserved,  /* opcCOP1 */
	instr__reserved,  /* opcCOP2 */
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
	instr__reserved,  /* unused */
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
	
	instr__reserved,  /* unused */
	instr__reserved,  /* unused */
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
	instr__reserved,  /* unused */
	instr_tne,
	instr__reserved,  /* unused */
	
	instr_dsll,
	instr__reserved,  /* unused */
	instr_dsrl,
	instr_dsra,
	instr_dsll32,
	instr__reserved,  /* unused */
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
	switch (res) {
	case excNone:
		break;
	case excAddrError:
		return excAdES;
	default:
		ASSERT(false);
	}
	
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
		ASSERT(false);
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
static void physmem_breakpoint_find(ptr36_t addr, len36_t size,
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

static physmem_area_t* find_physmem_area(ptr36_t addr)
{
	ptl1_t *ptl1 = ptl0[(addr >> PTL1_SHIFT) & PTL1_MASK];
	if (ptl1 != NULL) {
		frame_t *frame = (*ptl1)[(addr >> PTL2_SHIFT) & PTL2_MASK];
		if (frame != NULL) {
			physmem_area_t *area = frame->area;
			if (area != NULL)
				return area;
		}
	}
	
	return NULL;
}

static uint8_t devmem_read8(cpu_t *cpu, ptr36_t addr)
{
	uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read32)
			dev->type->read32(cpu, dev, addr, &val);
	
	return val;
}

static uint16_t devmem_read16(cpu_t *cpu, ptr36_t addr)
{
	uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read32)
			dev->type->read32(cpu, dev, addr, &val);
	
	return val;
}

static uint32_t devmem_read32(cpu_t *cpu, ptr36_t addr)
{
	uint32_t val = (uint32_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read32)
			dev->type->read32(cpu, dev, addr, &val);
	
	return val;
}

static uint64_t devmem_read64(cpu_t *cpu, ptr36_t addr)
{
	uint64_t val = (uint64_t) DEFAULT_MEMORY_VALUE;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL))
		if (dev->type->read64)
			dev->type->read64(cpu, dev, addr, &val);
	
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
	    (area->data + (addr - FRAME2ADDR(area->start)))));
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
	    (area->data + (addr - FRAME2ADDR(area->start)))));
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
	    (area->data + (addr - FRAME2ADDR(area->start)))));
}

/** Physical memory read (64 bits)
 *
 * Read 64 bits from memory. At first try to read from configured memory
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
uint64_t physmem_read64(cpu_t *cpu, ptr36_t addr, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/*
	 * No memory area found, try to read the value
	 * from appropriate device or return the default value.
	 */
	if (area == NULL)
		return devmem_read64(cpu, addr);
	
	/* Check for memory read breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 8, ACCESS_READ);
	
	return convert_uint64_t_endian(*((uint64_t *)
	    (area->data + (addr - FRAME2ADDR(area->start)))));
}

/** Load Linked and Store Conditional control
 *
 */
static void sc_control(ptr36_t addr)
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

static bool devmem_write8(cpu_t *cpu, ptr36_t addr, uint8_t val)
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

static bool devmem_write16(cpu_t *cpu, ptr36_t addr, uint16_t val)
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

static bool devmem_write32(cpu_t *cpu, ptr36_t addr, uint32_t val)
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

static bool devmem_write64(cpu_t *cpu, ptr36_t addr, uint64_t val)
{
	bool written = false;
	
	/* List for each device */
	device_t *dev = NULL;
	while (dev_next(&dev, DEVICE_FILTER_ALL)) {
		if (dev->type->write64) {
			dev->type->write64(cpu, dev, addr, val);
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
	
	*((uint8_t *) (area->data + (addr - FRAME2ADDR(area->start)))) =
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
	
	*((uint16_t *) (area->data + (addr - FRAME2ADDR(area->start)))) =
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
	
	*((uint32_t *) (area->data + (addr - FRAME2ADDR(area->start)))) =
	    convert_uint32_t_endian(val);
	
	return true;
}

/** Physical memory write (64 bits)
 *
 * Write 64 bits of data to memory at given address. At first try to find
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
bool physmem_write64(cpu_t *cpu, ptr36_t addr, uint64_t val, bool protected)
{
	physmem_area_t *area = find_physmem_area(addr);
	
	/* No region found, try to write the value to appropriate device */
	if (area == NULL)
		return devmem_write64(cpu, addr, val);
	
	/* Writting to ROM? */
	if ((!area->writable) && (protected))
		return false;
	
	sc_control(addr);
	
	/* Check for memory write breakpoints */
	if (protected)
		physmem_breakpoint_find(addr, 8, ACCESS_WRITE);
	
	*((uint64_t *) (area->data + (addr - FRAME2ADDR(area->start)))) =
	    convert_uint64_t_endian(val);
	
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

/** Decode MIPS R4000 instruction
 *
 * @return Instruction implementation.
 *
 */
static instr_fnc_t decode(instr_t instr)
{
	instr_fnc_t fnc;
	
	/*
	 * Basic opcode decoding based
	 * on the opcode field.
	 */
	switch (instr.r.opcode) {
	case opcSPECIAL:
		/*
		 * SPECIAL opcode decoding based
		 * on the func field.
		 */
		fnc = func_map[instr.r.func];
		break;
	case opcREGIMM:
		/*
		 * REGIMM opcode decoding based
		 * on the rt field.
		 */
		fnc = rt_map[instr.r.rt];
		break;
	case opcCOP0:
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
			fnc = cop0_rt_map[instr.r.rt];
			break;
		case cop0rsCO:
			/*
			 * COP0/CO opcode decoding
			 * based on the 8-bit func field.
			 */
			fnc = cop0_func_map[instr.cop.func];
			break;
		default:
			fnc = cop0_rs_map[instr.r.rs];
		}
		break;
	case opcCOP1:
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
			fnc = cop1_rt_map[instr.r.rt];
			break;
		default:
			fnc = cop1_rs_map[instr.r.rs];
		}
		break;
	case opcCOP2:
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
			fnc = cop2_rt_map[instr.r.rt];
			break;
		default:
			fnc = cop2_rs_map[instr.r.rs];
		}
		break;
	default:
		fnc = opcode_map[instr.r.opcode];
	}
	
	return fnc;
}

/** Execute the instruction specified by the opcode
 *
 */
static exc_t execute(cpu_t *cpu, instr_t instr)
{
	ASSERT(cpu != NULL);
	
	instr_fnc_t fnc = decode(instr);
	exc_t res = fnc(cpu, instr);
	
	/* Branch test */
	if ((cpu->branch == BRANCH_COND) || (cpu->branch == BRANCH_NONE))
		cpu->excaddr.ptr = cpu->pc.ptr;
	
	/* PC update */
	switch (res) {
	case excJump:
		/*
		 * Execute the instruction in the branch
		 * delay slot. The jump target is stored
		 * in pc_next.
		 */
		res = excNone;
		cpu->pc.ptr += 4;
		break;
	case excLikely:
		/*
		 * Nullify the instruction in the branch
		 * delay slot by ignoring it completely.
		 */
		res = excNone;
		cpu->pc_next.ptr += 4;
		/* No break */
	default:
		/*
		 * Advance to the next instruction
		 * as usual.
		 */
		cpu->pc.ptr = cpu->pc_next.ptr;
		cpu->pc_next.ptr += 4;
		break;
	}
	
	/* Register 0 contains a hardwired zero value */
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
	if (machine_trace) {
		ASSERT(res <= excVCED);
		alert("Raised exception %u: %s", res, txt_exc[res]);
	}
	
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
			exc_pc.ptr = EXCEPTION_BOOT_BASE_ADDRESS;
		else
			exc_pc.ptr = EXCEPTION_BOOT_RESET_ADDRESS;
	} else {
		/* Normal time */
		if (res != excReset)
			exc_pc.ptr = EXCEPTION_NORMAL_BASE_ADDRESS;
		else
			exc_pc.ptr = EXCEPTION_NORMAL_RESET_ADDRESS;
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
}

/* Simulate one instruction
 *
 */
static void instruction(cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	/* Fetch instruction */
	instr_t instr;
	exc_t res = cpu_read_ins(cpu, cpu->pc, &instr.val, true);
	
	if (res == excNone) {
		/* Execute instruction */
		ptr64_t old_pc = cpu->pc;
		res = execute(cpu, instr);
		
		/* Tracing output */
		if (machine_trace)
			idump(cpu, old_pc, instr, true);
	}
	
	/* Manage CPU state */
	manage(cpu, res);
}

/* Simulate 4096 cycles of the processor
 *
 * This is just one instruction.
 *
 */
void cpu_step4k(cpu_t *cpu)
{
	ASSERT(cpu != NULL);
	
	unsigned int i;
	
	for (i = 0; i < 4096; i++) {
		/*
		 * Execute one instruction and update
		 * CPU cycle accounting.
		 */
		if (!cpu->stdby) {
			instruction(cpu);
			
			if (CPU_KERNEL_MODE(cpu))
				cpu->k_cycles++;
			else
				cpu->u_cycles++;
		} else
			cpu->w_cycles++;
	}
}
