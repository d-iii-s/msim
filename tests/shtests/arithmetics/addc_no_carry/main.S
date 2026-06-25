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
    mov #1, r0
    mov #2, r1
    clrt
    // 1 + 2 + T (which should be cleared) = 3 (and T bit should still be cleared)
    addc r1, r0
    cpu_dump
    ehalt