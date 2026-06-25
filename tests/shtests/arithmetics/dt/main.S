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
    mov     #2, r0
    clrt
    // r0 := 2 - 1 (T-bit not set)
    dt      r0
    cpu_dump
    // r0 := 1 - 1 (T-bit set)
    dt      r0
    cpu_dump
    ehalt
