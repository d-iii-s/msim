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

// TODO: CSR

typedef enum {
	
	/********************************
	 * Unprivileged Counters/Timers *
	 ********************************/
	
	/* Low word */
	csr_cycle 		   =	0xC00,
	csr_time 		   = 	0xC01,
	csr_instret 	   =	0xC02,
	csr_hpmcounter3    = 	0xC03,
	csr_hpmcounter4    = 	0xC04,
	csr_hpmcounter5    = 	0xC05,
	csr_hpmcounter6    = 	0xC06,
	csr_hpmcounter7    = 	0xC07,
	csr_hpmcounter8    = 	0xC08,
	csr_hpmcounter9    = 	0xC09,
	csr_hpmcounter10   = 	0xC0A,
	csr_hpmcounter11   = 	0xC0B,
	csr_hpmcounter12   = 	0xC0C,
	csr_hpmcounter13   = 	0xC0D,
	csr_hpmcounter14   = 	0xC0E,
	csr_hpmcounter15   = 	0xC0F,
	csr_hpmcounter16   = 	0xC10,
	csr_hpmcounter17   = 	0xC11,
	csr_hpmcounter18   = 	0xC12,
	csr_hpmcounter19   = 	0xC13,
	csr_hpmcounter20   = 	0xC14,
	csr_hpmcounter21   = 	0xC15,
	csr_hpmcounter22   = 	0xC16,
	csr_hpmcounter23   = 	0xC17,
	csr_hpmcounter24   = 	0xC18,
	csr_hpmcounter25   = 	0xC19,
	csr_hpmcounter26   = 	0xC1A,
	csr_hpmcounter27   = 	0xC1B,
	csr_hpmcounter28   = 	0xC1C,
	csr_hpmcounter29   = 	0xC1D,
	csr_hpmcounter30   = 	0xC1E,
	csr_hpmcounter31   = 	0xC1F,

	/* High word*/
	csr_cycleh 		   =	0xC80,
	csr_timeh 		   = 	0xC81,
	csr_instreth 	   =	0xC82,
	csr_hpmcounter3h   = 	0xC83,
	csr_hpmcounter4h   = 	0xC84,
	csr_hpmcounter5h   = 	0xC85,
	csr_hpmcounter6h   = 	0xC86,
	csr_hpmcounter7h   = 	0xC87,
	csr_hpmcounter8h   = 	0xC88,
	csr_hpmcounter9h   = 	0xC89,
	csr_hpmcounter10h  =	0xC8A,
	csr_hpmcounter11h  = 	0xC8B,
	csr_hpmcounter12h  = 	0xC8C,
	csr_hpmcounter13h  = 	0xC8D,
	csr_hpmcounter14h  = 	0xC8E,
	csr_hpmcounter15h  = 	0xC8F,
	csr_hpmcounter16h  = 	0xC90,
	csr_hpmcounter17h  = 	0xC91,
	csr_hpmcounter18h  = 	0xC92,
	csr_hpmcounter19h  = 	0xC93,
	csr_hpmcounter20h  = 	0xC94,
	csr_hpmcounter21h  = 	0xC95,
	csr_hpmcounter22h  = 	0xC96,
	csr_hpmcounter23h  = 	0xC97,
	csr_hpmcounter24h  = 	0xC98,
	csr_hpmcounter25h  = 	0xC99,
	csr_hpmcounter26h  = 	0xC9A,
	csr_hpmcounter27h  = 	0xC9B,
	csr_hpmcounter28h  = 	0xC9C,
	csr_hpmcounter29h  = 	0xC9D,
	csr_hpmcounter30h  = 	0xC9E,
	csr_hpmcounter31h  = 	0xC9F,

	/*************************
	 * Supervisor level CSRs *
	 *************************/
	
	/* Trap Setup */
	csr_sstatus 	   = 	0x100,
	csr_sie 		   = 	0x104,
	csr_stvec 		   = 	0x105,
	csr_stcounteren	   = 	0x106,

	/* Configuration */
	csr_senvcfg 	   = 	0x10A,

	/* Trap Handling */
	csr_sscratch 	   = 	0x140,
	csr_sepc		   = 	0x141,
	csr_scause 	 	   = 	0x142,
	csr_stval          =    0x143,
	csr_sip            =    0x144,

	/* Address Translation and Protection */
	csr_satp           =    0x180,

	/* Debug/Trace */
	csr_scontext       =    0x5A8,

	/**********************
	 * Machine level CSRs *
	 **********************/

	/* Machine information */
	csr_mvendorid      =    0xF11,
	csr_marchid        =    0xF12,
	csr_mimpid         =    0xF13,
	csr_mhartid        =    0xF14,
	csr_mconfigptr     =    0xF15,

	/* Trap Setup */
	csr_mstatus        =    0x300,
	csr_misa           =    0x301,
	csr_medeleg        =    0x302,
	csr_mideleg        =    0x303,
	csr_mie            =    0x304,
	csr_mtvec          =    0x305,
	csr_mcounteren     =    0x306,
	csr_mstatush       =    0x310,

	/* Trap Handling */
	csr_mscratch       =    0x340,
	csr_mepc           =    0x341,
	csr_mcause         =    0x342,
	csr_mtval          =    0x343,
	csr_mip            =    0x344,
	csr_mtinst         =    0x34A,
	csr_mtval2         =    0x34B,

	/* Machine Configuration */
	csr_menvcfg        =    0x30A,
	csr_mevncfgh       =    0x31A,
	csr_mseccfg        =    0x747,
	csr_mseccfg        =    0x757,

	/* Memory Protection */
	csr_pmpcfg0        =    0x3A0,
	csr_pmpcfg1        =    0x3A1,
	csr_pmpcfg2        =    0x3A2,
	csr_pmpcfg3        =    0x3A3,
	csr_pmpcfg4        =    0x3A4,
	csr_pmpcfg5        =    0x3A5,
	csr_pmpcfg6        =    0x3A6,
	csr_pmpcfg7        =    0x3A7,
	csr_pmpcfg8        =    0x3A8,
	csr_pmpcfg9        =    0x3A9,
	csr_pmpcfg10       =    0x3AA,
	csr_pmpcfg11       =    0x3AB,
	csr_pmpcfg12       =    0x3AC,
	csr_pmpcfg13       =    0x3AD,
	csr_pmpcfg14       =    0x3AE,
	csr_pmpcfg15       =    0x3AF,

	csr_pmpaddr0       =    0x3B0,
    csr_pmpaddr1       =    0x3B1,
    csr_pmpaddr2       =    0x3B2,
    csr_pmpaddr3       =    0x3B3,
    csr_pmpaddr4       =    0x3B4,
    csr_pmpaddr5       =    0x3B5,
    csr_pmpaddr6       =    0x3B6,
    csr_pmpaddr7       =    0x3B7,
    csr_pmpaddr8       =    0x3B8,
    csr_pmpaddr9       =    0x3B9,
    csr_pmpaddr10      =    0x3BA,
    csr_pmpaddr11      =    0x3BB,
    csr_pmpaddr12      =    0x3BC,
    csr_pmpaddr13      =    0x3BD,
    csr_pmpaddr14      =    0x3BE,
    csr_pmpaddr15      =    0x3BF,
    csr_pmpaddr16      =    0x3C0,
    csr_pmpaddr17      =    0x3C1,
    csr_pmpaddr18      =    0x3C2,
    csr_pmpaddr19      =    0x3C3,
    csr_pmpaddr20      =    0x3C4,
    csr_pmpaddr21      =    0x3C5,
    csr_pmpaddr22      =    0x3C6,
    csr_pmpaddr23      =    0x3C7,
    csr_pmpaddr24      =    0x3C8,
    csr_pmpaddr25      =    0x3C9,
    csr_pmpaddr26      =    0x3CA,
    csr_pmpaddr27      =    0x3CB,
    csr_pmpaddr28      =    0x3CC,
    csr_pmpaddr29      =    0x3CD,
    csr_pmpaddr30      =    0x3CE,
    csr_pmpaddr31      =    0x3CF,
    csr_pmpaddr32      =    0x3D0,
    csr_pmpaddr33      =    0x3D1,
    csr_pmpaddr34      =    0x3D2,
    csr_pmpaddr35      =    0x3D3,
    csr_pmpaddr36      =    0x3D4,
    csr_pmpaddr37      =    0x3D5,
    csr_pmpaddr38      =    0x3D6,
    csr_pmpaddr39      =    0x3D7,
    csr_pmpaddr40      =    0x3D8,
    csr_pmpaddr41      =    0x3D9,
    csr_pmpaddr42      =    0x3DA,
    csr_pmpaddr43      =    0x3DB,
    csr_pmpaddr44      =    0x3DC,
    csr_pmpaddr45      =    0x3DD,
    csr_pmpaddr46      =    0x3DE,
    csr_pmpaddr47      =    0x3DF,
    csr_pmpaddr48      =    0x3E0,
    csr_pmpaddr49      =    0x3E1,
    csr_pmpaddr50      =    0x3E2,
    csr_pmpaddr51      =    0x3E3,
    csr_pmpaddr52      =    0x3E4,
    csr_pmpaddr53      =    0x3E5,
    csr_pmpaddr54      =    0x3E6,
    csr_pmpaddr55      =    0x3E7,
    csr_pmpaddr56      =    0x3E8,
    csr_pmpaddr57      =    0x3E9,
    csr_pmpaddr58      =    0x3EA,
    csr_pmpaddr59      =    0x3EB,
    csr_pmpaddr60      =    0x3EC,
    csr_pmpaddr61      =    0x3ED,
    csr_pmpaddr62      =    0x3EE,
    csr_pmpaddr63      =    0x3EF,

	/* Counters/Timers */
	/* Low word */
	csr_mcycle 		   =	0xB00,
	csr_minstret 	   =	0xB02,
	csr_mhpmcounter3   = 	0xB03,
	csr_mhpmcounter4   = 	0xB04,
	csr_mhpmcounter5   = 	0xB05,
	csr_mhpmcounter6   = 	0xB06,
	csr_mhpmcounter7   = 	0xB07,
	csr_mhpmcounter8   = 	0xB08,
	csr_mhpmcounter9   = 	0xB09,
	csr_mhpmcounter10  = 	0xB0A,
	csr_mhpmcounter11  = 	0xB0B,
	csr_mhpmcounter12  = 	0xB0C,
	csr_mhpmcounter13  = 	0xB0D,
	csr_mhpmcounter14  = 	0xB0E,
	csr_mhpmcounter15  = 	0xB0F,
	csr_mhpmcounter16  = 	0xB10,
	csr_mhpmcounter17  = 	0xB11,
	csr_mhpmcounter18  = 	0xB12,
	csr_mhpmcounter19  = 	0xB13,
	csr_mhpmcounter20  = 	0xB14,
	csr_mhpmcounter21  = 	0xB15,
	csr_mhpmcounter22  = 	0xB16,
	csr_mhpmcounter23  = 	0xB17,
	csr_mhpmcounter24  = 	0xB18,
	csr_mhpmcounter25  = 	0xB19,
	csr_mhpmcounter26  = 	0xB1A,
	csr_mhpmcounter27  = 	0xB1B,
	csr_mhpmcounter28  = 	0xB1C,
	csr_mhpmcounter29  = 	0xB1D,
	csr_mhpmcounter30  = 	0xB1E,
	csr_mhpmcounter31  = 	0xB1F,

	/* High word*/
	csr_mcycleh        =	0xB80,
	csr_minstreth 	   =	0xB82,
	csr_mhpmcounter3h  = 	0xB83,
	csr_mhpmcounter4h  = 	0xB84,
	csr_mhpmcounter5h  = 	0xB85,
	csr_mhpmcounter6h  = 	0xB86,
	csr_mhpmcounter7h  = 	0xB87,
	csr_mhpmcounter8h  = 	0xB88,
	csr_mhpmcounter9h  = 	0xB89,
	csr_mhpmcounter10h =	0xB8A,
	csr_mhpmcounter11h = 	0xB8B,
	csr_mhpmcounter12h = 	0xB8C,
	csr_mhpmcounter13h = 	0xB8D,
	csr_mhpmcounter14h = 	0xB8E,
	csr_mhpmcounter15h = 	0xB8F,
	csr_mhpmcounter16h = 	0xB90,
	csr_mhpmcounter17h = 	0xB91,
	csr_mhpmcounter18h = 	0xB92,
	csr_mhpmcounter19h = 	0xB93,
	csr_mhpmcounter20h = 	0xB94,
	csr_mhpmcounter21h = 	0xB95,
	csr_mhpmcounter22h = 	0xB96,
	csr_mhpmcounter23h = 	0xB97,
	csr_mhpmcounter24h = 	0xB98,
	csr_mhpmcounter25h = 	0xB99,
	csr_mhpmcounter26h = 	0xB9A,
	csr_mhpmcounter27h = 	0xB9B,
	csr_mhpmcounter28h = 	0xB9C,
	csr_mhpmcounter29h = 	0xB9D,
	csr_mhpmcounter30h = 	0xB9E,
	csr_mhpmcounter31h = 	0xB9F,

	/* Counter Setup */
	csr_mcountinhibit  =    0x320,
	csr_mhmpevent3     =	0x323,
	csr_mhmpevent4     =	0x324,
	csr_mhmpevent5     =	0x325,
	csr_mhmpevent6     =	0x326,
	csr_mhmpevent7     =	0x327,
	csr_mhmpevent8     =	0x328,
	csr_mhmpevent9     =	0x329,
	csr_mhmpevent10    =	0x32A,
	csr_mhmpevent11    =	0x32B,
	csr_mhmpevent12    =	0x32C,
	csr_mhmpevent13    =	0x32D,
	csr_mhmpevent14    =	0x32E,
	csr_mhmpevent15    =	0x32F,
	csr_mhmpevent16    =	0x330,
	csr_mhmpevent17    =	0x331,
	csr_mhmpevent18    =	0x332,
	csr_mhmpevent19    =	0x333,
	csr_mhmpevent20    =	0x334,
	csr_mhmpevent21    =	0x335,
	csr_mhmpevent22    =	0x336,
	csr_mhmpevent23    =	0x337,
	csr_mhmpevent24    =	0x338,
	csr_mhmpevent25    =	0x339,
	csr_mhmpevent26    =	0x33A,
	csr_mhmpevent27    =	0x33B,
	csr_mhmpevent28    =	0x33C,
	csr_mhmpevent29    =	0x33D,
	csr_mhmpevent30    =	0x33E,
	csr_mhmpevent31    =	0x33F,

	/* Debug/Trace */
	csr_tselect        =    0x7A0,
	csr_tdata1         =    0x7A1,
	csr_tdata2         =    0x7A2,
	csr_tdata3         =    0x7A3,
	csr_mcontext       =    0x7A8,
	
	/* Debug Mode */
	csr_dcsr           =    0x7B0,
	csr_dpc            =    0x7B1,
	csr_dscratch0      =    0x7B2,
	csr_dscratch1      =    0x7B3

} csr_regs_t;


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
