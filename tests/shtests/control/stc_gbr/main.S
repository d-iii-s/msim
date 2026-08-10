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
    mov.l       val, r0
    ldc         r0, gbr
    stc         gbr, r1
    cpu_dump
    ehalt

.align 2
val:
    .long       0x12345678