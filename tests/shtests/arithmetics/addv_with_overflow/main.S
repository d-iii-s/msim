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
    mov.l val, r1
    // r1 should have 0x80000000 after the addv insn and T bit should be set
    addv r0, r1
    cpu_dump
    ehalt

.align 2
val:
    .long 0x7FFFFFFF