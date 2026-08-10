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
    // 1 - 2 - T = -1 (T bit is cleared before and should be set after instruction)
    subc r1, r0
    cpu_dump
    // 0 - 0 - T (which should be set) = -1 (T bit should still be set after this instruction)
    subc r3, r2
    cpu_dump
    ehalt