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
    mov #0, r1
    clrt
    // 0 - 1 - T (which is not set) = -1 (should set the T bit)
    negc r0, r0
    cpu_dump
    // 0 - 0 - T (which is set) = -1 (T bit should still be set after this)
    negc r1, r1
    cpu_dump
    ehalt