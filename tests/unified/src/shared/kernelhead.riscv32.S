.section .handlers

.macro rd_dump_and_quit
    .word 0x8C100073
    .word 0x8C000073
.endm rd_dump_and_quit

.balign 4
machine_trap_handler:
    rd_dump_and_quit

.section .head

.org 0x200

/* Initial code after handover from bootloader. */

    /* Setup stack */
    la sp, 0x8D000000

    /* Reset bare-mode */
    csrw satp, zero

    /* Jump to C */
    jal ra, kernel_wrap
    nop

