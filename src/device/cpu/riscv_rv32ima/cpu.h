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

#define RV32IMA_REG_COUNT	32


// TODO: prev regs for debug
// TODO: instruction decoding

/** Main processor structure */
typedef struct {
	/* procno: in csr*/
	bool stdby;
	// struct frame - for holding cached decoded instructions

	uint32_t regs[RV32IMA_REG_COUNT];
	// CSR
	// f regs - no

	uint32_t pc;
	// pc_next - unneccesary, because risc-v does not have a delay slot

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

} rv32ima_cpu_t;


/** Basic CPU routines */
extern void rv32ima_cpu_init(rv32ima_cpu_t *cpu, unsigned int procno);
extern void rv32ima_cpu_set_pc(rv32ima_cpu_t *cpu, uint32_t value);
extern void rv32ima_cpu_step(rv32ima_cpu_t *cpu);

/** Interrupts */
extern void rv32ima_cpu_interrupt_up(rv32ima_cpu_t *cpu, unsigned int no);
extern void rv32ima_cpu_interrupt_down(rv32ima_cpu_t *cpu, unsigned int no);

#endif //RISCV_RV32IMA_CPU_H_
