// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/** MSIM-specific instruction for dumping general registers. */
#define machine_dump_rd_asm .word 0x8C100073

/** MSIM-specific instruction for halting the simulation (power-off). */
#define machine_halt_asm .word 0x8C000073

/** MSIM-specific instruction for entering interactive mode. */
#define machine_enter_debugger_asm ebreak

/** MSIM-specific instruction for dumping CSR indexed by t0. */
#define machine_dump_csr_t0_asm .word 0x8C4002F3

/** The CSR number of the MEPC (machine exception program counter) CSR */
#define CSR_MEPC 0x341

/** The CSR number of the MCAUSE (machine cause) CSR */
#define CSR_MCAUSE 0x342

/** The CSR number of the MTVAL (machine trap value) CSR */
#define CSR_MTVAL 0x343

/** Bitmask that signifies that the MPP
 *  (machine previous privilege) is set to S (supervisor mode) in MSTATUS CSR
 */
#define CSR_MSTATUS_MPP_S 0x800

.section .handlers

/*
 * Machine-mode trap handler.
 *
 * Services any exceptions/interrupts not delegated to supervisor mode.
 *
 * This handler runs in M-mode and simply dumps general purpose registers
 * together with 'mepc', 'mcause' and 'mtval' CSRs. Then it enters MSIM
 * interactive mode to allow the user to inspect registers, memory or
 * anything else of interest. Finally, it halts the simulator, because
 * there is nothing else to be done.
 */
.balign 4
machine_trap_handler:
    // Dump general registers.
    machine_dump_rd_asm

    // Dump relevant Control and Status Registers (CSRs).
    li t0, CSR_MEPC
    machine_dump_csr_t0_asm
    li t0, CSR_MCAUSE
    machine_dump_csr_t0_asm
    li t0, CSR_MTVAL
    machine_dump_csr_t0_asm

    // Switch MSIM to interactive mode first, then halt it.
    machine_enter_debugger_asm
    machine_halt_asm


.section .head

/*
 * Kernel entry point (the boot loader code at 0xF0000000 jumps here).
 *
 * The Machine Trap-Vector Base-Address (mtvec) is set to addresses of the trap
 * handler. Because the lowest two bits of the handler addresses will be zero
 * (due to 4-byte alignment), the vector MODE field is set to 'direct', which
 * means that all exceptions cause the processor to jump to the trap handler
 * BASE address.
 *
 * The MODE field in the Supervisor Address Translation and Protection (satp)
 * register is set to zero (`bare` mode), which disables address translation
 * in supervisor mode, thus providing identity mapping without page tables.
 *
 * The Machine Previous Privilege (MPP) field in the Machine Status (mstatus)
 * register is set to level 1 ('supervisor' mode) so that the kernel starts
 * in S-mode.
 *
 * Prior to jumping to kernel C code, the stack pointer is initialized so that
 * the area immediately preceding the 'start' entry point becomes an initial
 * 4-KiB stack (which should be enough for everybody).
 *
 * The Machine Exception Program Counter (mepc) register is set to the address
 * of the kernel C entry point (the 'kernel_main' function). The final 'mret'
 * instruction jumps to the kernel C code while switching the processor to the
 * privilege mode stored in the 'mstatus.MPP' field.
 */
.org 0x1000
start:
    // Set M-mode trap handler base, use 'direct' vector mode.
    la t0, machine_trap_handler
    csrw mtvec, t0

    // Enable 'bare' mode to disable address translation.
    // Requires all other bits in 'satp' to be zero.
    csrw satp, zero

    // Switch the processor to 'supervisor' mode with 'mret'.
    li t0, CSR_MSTATUS_MPP_S
    csrs mstatus, t0

    // Setup initial stack and jump to kernel C code in 'kernel_main'.
    la sp, start

    la ra, kernel_main
    csrw mepc, ra

    mret
    // Paranoia: this is unreachable, but just in case...
    machine_halt_asm
    loop:
        j loop
