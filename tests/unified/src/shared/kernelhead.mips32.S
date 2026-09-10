.text
.set noreorder

/* Dump registers and halt. */
.macro rd_dump_and_quit
    .insn
    .word 0x37
    .insn
    .word 0x0e
    .insn
    .word 0x28
.endm rd_dump_and_quit

/* Put the hardwired code to a special input section ".excvec". */
.section .excvec, "ax"

.org 0x0000
.ent exception_tlb_refill
exception_tlb_refill:
    rd_dump_and_quit
.end exception_tlb_refill


.org 0x0100
.ent exception_cache_error
exception_cache_error:
    rd_dump_and_quit
.end exception_cache_error


.org 0x0180
.ent exception_general
exception_general:
    rd_dump_and_quit
.end exception_general

.org 0x200
.ent start
start:
    /* Setup stack */
    la $sp, 0x8D000000

    /* Leave BEV and disable interrupts globally. */
    la $k0, 0xff00
    mtc0 $k0, $12

    /* Jump to the actual main now */
    jal kernel_main
    nop
    .insn
    simulator_halt
.end start

