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
    mov     #8, r0
    mov     #4, r1
    // 4 * 8 = 32
    mul.l   r0, r1
    cpu_dump
    mov.l   val, r0
    mov.l   val, r1
    // 0x00010000 * 0x00010000 should result in the lower 32-bits being all zeroes
    mul.l   r0, r1
    cpu_dump
    ehalt

.align 2
val:
    .long 0x00010000
