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
#include "../../../main.h"

#define RV32IMA_REG_COUNT	32

#define RV_INTERRUPT_EXC_BITS UINT32_C(0x80000000)
#define RV_EXCEPTION_EXC_BITS UINT32_C(0)

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
	rv_exc_none = RV_EXCEPTION_EXC_BITS | 24
} rv_exc_t;

// TODO: prev regs for debug
// TODO: instruction decoding

/** Main processor structure */
typedef struct rv_cpu 

{
	/* procno: in csr*/
	bool stdby;
	// struct frame - for holding cached decoded instructions

	uint32_t regs[RV32IMA_REG_COUNT];
	
	// CSR
	csr_t csr;

	// f regs - no

	uint32_t pc;
	// Used to implement jumps and branches easily
	uint32_t pc_next;

	// tlb - virtual memory is done using page tables
	// but if this would be really slow, some caching could be a nice optimalization
	
	// old registers - todo

	// excaddr ???
	// branch - again, no branch delay

	// LR and SC - TODO

	// watch ?? is this a gdb or a mips thing?
	// TODO: look into this

	// statistics - in CSR
	// maybe define some of the custom counters for these statistics

	// tlb - not in risc-v

	// intr??? what is this

	// breakpoints: TODO

} rv_cpu_t;


/** Basic CPU routines */
extern void rv_cpu_init(rv_cpu_t *cpu, unsigned int procno);
extern void rv_cpu_set_pc(rv_cpu_t *cpu, uint32_t value);
extern void rv_cpu_step(rv_cpu_t *cpu);

/** Interrupts */
extern void rv_cpu_interrupt_up(rv_cpu_t *cpu, unsigned int no);
extern void rv_cpu_interrupt_down(rv_cpu_t *cpu, unsigned int no);

/** Memory operations */
extern rv_exc_t rv_convert_addr(rv_cpu_t *cpu, uint32_t virt, ptr36_t *phys, bool wr, bool noisy);
extern rv_exc_t rv_read_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t *value, bool noisy);
extern rv_exc_t rv_write_mem32(rv_cpu_t *cpu, uint32_t virt, uint32_t value, bool noisy);

#endif //RISCV_RV32IMA_CPU_H_
