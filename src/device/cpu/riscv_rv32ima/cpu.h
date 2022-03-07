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

#define RV32IMA_REG_COUNT	32

// TODO: CSR
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

typedef union {

} instr_t;

/** Basic CPU routines */
extern void cpu_init(rv32ima_cpu_t *cpu, unsigned int procno);
extern void cpu_set_pc(rv32ima_cpu_t *cpu, uint32_t value);
extern void cpu_step(rv32ima_cpu_t *cpu);

/** Interrupts */
extern void cpu_interrupt_up(rv32ima_cpu_t *cpu, unsigned int no);
extern void cpu_interrupt_down(rv32ima_cpu_t *cpu, unsigned int no);

#endif //RISCV_RV32IMA_CPU_H_
