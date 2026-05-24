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

#define XLEN 32

#include <stdbool.h>
#include <stdint.h>

#include "../../../main.h"
#include "../riscv_rv_ima/csr.h"
#include "../riscv_rv_ima/types.h"
#include "tlb.h"

#define RV_REG_COUNT 32

#define RV_INSTR_CACHE_SIZE 1024

struct rv_tlb;

/** Main processor structure */
typedef struct rv32_cpu {
    /** Non privileged registers */
    uint32_t regs[RV_REG_COUNT];

    /** Control and status registers */
    rv_csr_t csr;

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
    rv32_tlb_t tlb;

} rv32_cpu_t;

/** Basic CPU routines */
extern void rv32_cpu_init(rv32_cpu_t *cpu, unsigned int procno);
extern void rv32_cpu_done(rv32_cpu_t *cpu);
extern void rv32_cpu_set_pc(rv32_cpu_t *cpu, uint32_t value);
extern void rv32_cpu_step(rv32_cpu_t *cpu);

/** Interrupts */
extern void rv32_interrupt_up(rv32_cpu_t *cpu, unsigned int no);
extern void rv32_interrupt_down(rv32_cpu_t *cpu, unsigned int no);

/** Memory operations */
extern rv_exc_t rv32_convert_addr(rv32_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);
extern bool rv32_sc_access(rv32_cpu_t *cpu, ptr36_t phys, int size);

#endif // RISCV_RV32IMA_CPU_H_
