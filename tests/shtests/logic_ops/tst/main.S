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
    mov     #0, r0
    mov     #1, r1
    mov     #-1, r2
    tst     r0, r1
    cpu_dump
    tst     r1, r2
    cpu_dump
    ehalt
