/*
 * Copyright (c) 2022 Jan Papesch
 * Copyright (c) 2025 Martin Rosenberg
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  RISCV RV64IMA simulation
 *
 */

#ifndef RISCV_RV64IMA_CPU_H_
#define RISCV_RV64IMA_CPU_H_

#define XLEN 64

#include <stdbool.h>
#include <stdint.h>

#include "../../../main.h"
#include "../riscv_rv_ima/csr.h"
#include "../riscv_rv_ima/types.h"
#include "tlb.h"

#define RV64_REG_COUNT 32

struct rv64_tlb;

/** Main processor structure */
typedef struct rv64_cpu {
    /** Non privileged registers */
    uint64_t regs[RV64_REG_COUNT];

    /** Control and status registers */
    rv_csr_t csr;

    /** Program counter */
    uint64_t pc;

    /** The next value of PC
     *  Used for implementing jumps, branches and traps
     */
    uint64_t pc_next;

    /** Current privilege mode */
    rv_priv_mode_t priv_mode;

    // LR and SC
    bool reserved_valid; /** Is the current LR reservation valid */
    ptr36_t reserved_addr; /** physical address of the last LR */

    /** Tells if the processor is executing or waiting */
    bool stdby;

    /** Translation Lookaside Buffer used for caching translated addresses */
    rv64_tlb_t tlb;

} rv64_cpu_t;

/** Basic CPU routines */
extern void rv64_cpu_init(rv64_cpu_t *cpu, unsigned int procno);
extern void rv64_cpu_done(rv64_cpu_t *cpu);
extern void rv64_cpu_set_pc(rv64_cpu_t *cpu, virt_t value);
extern void rv64_cpu_step(rv64_cpu_t *cpu);

/** Interrupts */
extern void rv64_interrupt_up(rv64_cpu_t *cpu, unsigned int no);
extern void rv64_interrupt_down(rv64_cpu_t *cpu, unsigned int no);

/** Memory and address conversion */
extern rv_exc_t rv64_convert_addr(rv64_cpu_t *cpu, virt_t virt, ptr36_t *phys, bool wr, bool fetch, bool noisy);
extern bool rv64_sc_access(rv64_cpu_t *cpu, ptr36_t phys, int size);

#endif // RISCV_RV64IMA_CPU_H_
