// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * Kernel bootstrap code.
 *
 * This code will be loaded at (physical) address 0.
 */

/*
 * MSIM-specific macro for halting the simulator.
 */
.macro halt
    .insn
    .word 0x28

    /*
     * Loop forever in case the above instruction is not
     * properly recognized by the simulator.
     * As a matter of fact, if the instruction is not recognized,
     * exception should have been raised but we are a bit paranoid
     * here ;-).
     */

    0:
        j 0b
        nop

.endm halt

/*
 * MSIM-specific macro for entering interactive mode.
 */
.macro msim_stop
    .insn
    .word 0x29
.endm msim_stop

/*
 * For now, dump registers and enter interactive mode on any exception.
 */
.macro announce_exception
    .insn
    .word 0x37
    msim_stop
.endm announce_exception

/*
 * Set status register to a sane value.
 *
 * This includes switching to normal exception vector (stop using BEV)
 * and enabling all interrupts (IM0-IM7).
 *
 * Note: Status register is 12.
 *
 * Note: we know when this will be called so we can safely use $k0.
 */
.macro set_status_register
    la $k0, 0xff01
    mtc0 $k0, $12
    nop
.endm set_status_register



/*
 * We know what whare are doing here so do not let
 * assembler change our code in any way.
 */

.set noat
.set noreorder

/*
 * Put the hardwired code to a special input section ".excvec".
 * This section is then placed by the linker (as instructed by the kernel
 * linker script) at the very beginning of the kernel image (address 0).
 */

.section .excvec, "ax"

/*
 * Debugging-only exception handling: enter interactive mode.
 */
.org   0x000
.globl exception_tlb_refill
.ent   exception_tlb_refill
exception_tlb_refill:
    announce_exception
.end exception_tlb_refill

.org   0x100
.globl exception_cache_error
.ent   exception_cache_error
exception_cache_error:
    announce_exception
.end exception_cache_error

.org   0x180
.globl exception_general
.ent   exception_general
exception_general:
    announce_exception
.end exception_general

/*
 * Kernel entry point.
 * The bootstrap loader (0x1FC00000) jumps here.
 * We only jump to the C code from here.
 */

.org   0x400
.globl start
.ent   start

start:
    la $sp, 0x80000400
    la $gp, 0x80000000
    set_status_register
    jal kernel_main
    nop

    /*
     * Again, paranoid style: this is unreachable anyway.
     */
    halt
.end start

/*
 * Rest of code.
 */
.text

