/*
 * Copyright (c) 2022 Jan Papesch
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV32IMA simulation
 *
 */

#ifndef RISCV_RV32IMA_CPU_H_
#define RISCV_RV32IMA_CPU_H_


#include <stdint.h>
#include <stdbool.h>
#include "instr.h"
#include "csr.h"
#include "tlb.h"
#include "../../../main.h"

#define RV_REG_COUNT	32
#define RV_START_ADDRESS    UINT32_C(0xF0000000)
#define RV_MTIME_ADDRESS    UINT32_C(0xFF000000)
#define RV_MTIMECMP_ADDRESS UINT32_C(0xFF000008)

#define RV_INTERRUPT_EXC_BITS UINT32_C(0x80000000)
#define RV_EXCEPTION_EXC_BITS UINT32_C(0)
#define RV_EXCEPTION_MASK(exc) (1U << ((exc) & ~RV_INTERRUPT_EXC_BITS))
#define RV_INTERRUPT_NO(interrupt) ((interrupt) & ~RV_INTERRUPT_EXC_BITS)

/**
 * RISC-V exception codes
 * 
 * Includes Interrupt exception codes
 */
typedef enum rv_exc {
	rv_exc_supervisor_software_interrupt = RV_INTERRUPT_EXC_BITS | 1,
	rv_exc_machine_software_interrupt = RV_INTERRUPT_EXC_BITS | 3,
	rv_exc_supervisor_timer_interrupt = RV_INTERRUPT_EXC_BITS | 5,
	rv_exc_machine_timer_interrupt = RV_INTERRUPT_EXC_BITS | 7,
	rv_exc_supervisor_external_interrupt = RV_INTERRUPT_EXC_BITS | 9,
	rv_exc_machine_external_interrupt = RV_INTERRUPT_EXC_BITS | 11,
	rv_exc_instruction_address_misaligned = RV_EXCEPTION_EXC_BITS | 0,
	rv_exc_instruction_access_fault = RV_EXCEPTION_EXC_BITS | 1,
	rv_exc_illegal_instruction = RV_EXCEPTION_EXC_BITS | 2,
	rv_exc_breakpoint = RV_EXCEPTION_EXC_BITS | 3,
	rv_exc_load_address_misaligned = RV_EXCEPTION_EXC_BITS | 4,
	rv_exc_load_access_fault = RV_EXCEPTION_EXC_BITS | 5,
	rv_exc_store_amo_address_misaligned = RV_EXCEPTION_EXC_BITS | 6,
	rv_exc_store_amo_access_fault = RV_EXCEPTION_EXC_BITS | 7,
	rv_exc_umode_environment_call = RV_EXCEPTION_EXC_BITS | 8,
	rv_exc_smode_environment_call = RV_EXCEPTION_EXC_BITS | 9,
	rv_exc_mmode_environment_call = RV_EXCEPTION_EXC_BITS | 11,
	rv_exc_instruction_page_fault = RV_EXCEPTION_EXC_BITS | 12,
	rv_exc_load_page_fault = RV_EXCEPTION_EXC_BITS | 13,
	rv_exc_store_amo_page_fault = RV_EXCEPTION_EXC_BITS | 15,
	rv_exc_none = RV_EXCEPTION_EXC_BITS | 24	/** Custom exception code, with the meaning that no exception has been raised */
} rv_exc_t;

/** Bitmask that has a bit set on every position that symbolizes an existing exception */
#define RV_EXCEPTIONS_MASK (									\
	RV_EXCEPTION_MASK(rv_exc_instruction_address_misaligned) |	\
	RV_EXCEPTION_MASK(rv_exc_instruction_access_fault)       |  \
	RV_EXCEPTION_MASK(rv_exc_illegal_instruction)            |  \
	RV_EXCEPTION_MASK(rv_exc_breakpoint)                     |  \
	RV_EXCEPTION_MASK(rv_exc_load_address_misaligned)        |  \
	RV_EXCEPTION_MASK(rv_exc_load_access_fault)              |  \
	RV_EXCEPTION_MASK(rv_exc_store_amo_address_misaligned)   |  \
	RV_EXCEPTION_MASK(rv_exc_store_amo_access_fault)         |  \
	RV_EXCEPTION_MASK(rv_exc_umode_environment_call)         |  \
	RV_EXCEPTION_MASK(rv_exc_smode_environment_call)         |  \
	RV_EXCEPTION_MASK(rv_exc_mmode_environment_call)         |  \
	RV_EXCEPTION_MASK(rv_exc_instruction_page_fault)         |  \
	RV_EXCEPTION_MASK(rv_exc_load_page_fault)                |  \
	RV_EXCEPTION_MASK(rv_exc_store_amo_page_fault)				\
)
/** Bitmask that has a bit set on every position that symbolizes an existing interrupt */
#define RV_INTERRUPTS_MASK (\
	RV_EXCEPTION_MASK(rv_exc_supervisor_software_interrupt) |   \
	RV_EXCEPTION_MASK(rv_exc_machine_software_interrupt)    |   \
	RV_EXCEPTION_MASK(rv_exc_supervisor_external_interrupt) |   \
	RV_EXCEPTION_MASK(rv_exc_machine_external_interrupt)    |   \
	RV_EXCEPTION_MASK(rv_exc_supervisor_timer_interrupt)    |   \
	RV_EXCEPTION_MASK(rv_exc_machine_timer_interrupt)           \
)

/** Privilege modes */
typedef enum rv_priv_mode {
	rv_umode = 0b00,
	rv_smode = 0b01,
	rv_rmode = 0b10, // RESERVED
	rv_mmode = 0b11
} rv_priv_mode_t;

struct rv_tlb;

/** Main processor structure */
typedef struct rv_cpu {
	/** Non privileged registers */
	uint32_t regs[RV_REG_COUNT];

	/** Control and status registers */
	csr_t csr;

	/** Program counter */
	uint32_t pc;

	/** The next value of PC
	 *  Used for implementing jumps, branches and traps
	 */
	uint32_t pc_next;

	/** Current privilege mode */
	rv_priv_mode_t priv_mode;

	// LR and SC
	bool reserved_valid; /** Is the current LR reservation valid */
	ptr36_t reserved_addr; /** physical address of the last LR */

	/** Tells if the processor is executing or waiting */
	bool stdby;

	/** Translation Lookaside Buffer used for caching translated addresses */
	rv_tlb_t tlb;

} rv_cpu_t;


/** Basic CPU routines */
extern void rv_cpu_init(rv_cpu_t *cpu, unsigned int procno);
extern void rv_cpu_done(rv_cpu_t *cpu);
extern void rv_cpu_set_pc(rv_cpu_t *cpu, uint32_t value);
extern void rv_cpu_step(rv_cpu_t *cpu);

/** Interrupts */
extern void rv_interrupt_up(rv_cpu_t *cpu, unsigned int no);
extern void rv_interrupt_down(rv_cpu_t *cpu, unsigned int no);

/** Memory operations */
extern rv_exc_t rv_convert_addr(rv_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);
extern rv_exc_t rv_read_mem8(rv_cpu_t *cpu, uint32_t virt, uint8_t *value, bool noisy);
extern rv_exc_t rv_read_mem16(rv_cpu_t *cpu, uint32_t virt, uint16_t *value, bool fetch, bool noisy);
extern rv_exc_t rv_read_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t *value, bool fetch, bool noisy);
extern rv_exc_t rv_write_mem8(rv_cpu_t *cpu, uint32_t virt, uint8_t value, bool noisy);
extern rv_exc_t rv_write_mem16(rv_cpu_t *cpu, uint32_t virt, uint16_t value, bool noisy);
extern rv_exc_t rv_write_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t value, bool noisy);
extern bool rv_sc_access(rv_cpu_t *cpu, ptr36_t phys, int size);

#endif //RISCV_RV32IMA_CPU_H_
