#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x2F0
    .long 0x90000000     /* CMT 0 interrupt handler */

    .org 0x300
    .long 0x91000000     /* CMT 1 interrupt handler */

.section .text
    .org 0x400
_start:
    ldc         r0, sr

    mov.l       cmt_cmstr_address, r0
    mov         #3, r1
    mov.w       r1, @r0

    mov.l       cmt_cmcsr0_address, r0
    /* Period is 8 cycles */
    mov         #0x40, r1
    mov.w       r1, @r0

    mov.l       cmt_cmcsr1_address, r0
    /* Period is 128 cycles */
    mov         #0x42, r1
    mov.w       r1, @r0

loop:
    cmp/pl      r9
    bf          loop

    cpu_dump
    ehalt

.align 2
cmt_cmstr_address:
    .long       0xFFFFF710

cmt_cmcsr0_address:
    .long       0xFFFFF712

cmt_cmcsr1_address:
    .long       0xFFFFF718


