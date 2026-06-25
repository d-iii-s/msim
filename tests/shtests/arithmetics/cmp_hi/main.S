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
    mov     #2, r2
    mov     #3, r3
    cmp/hi  r3, r2
    cpu_dump
    cmp/hi  r1, r2
    cpu_dump
    cmp/hi  r0, r2
    cpu_dump
    mov     #1, r0
    cmp/hi  r0, r2
    cpu_dump
    ehalt