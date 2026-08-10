#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

.section .text
    .org 0x400
_start:
    mov.l   subroutine_addr, r1
    jsr     @r1
    // Executed because of the delayed branch
    mov     #-1, r0
    mov     #1, r2
    cpu_dump
    ehalt

subroutine:
    mov     #1, r3
    rts
    // Executed because of the delayed branch
    mov     #-1, r4
    // Skipped
    mov     #-2, r4


.align 2
subroutine_addr:
    .long   subroutine
