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
    mov     #-1, r0
    mov     #2, r1
    // -1 * 2 = -2
    dmuls.l  r0, r1
    cpu_dump
    mov.l   val, r0
    mov.l   val, r1
    // (-2147483648)^2 = 4611686018427387904
    dmuls.l  r0, r1
    cpu_dump
    ehalt

.align 2
val:
    .long 0x80000000
