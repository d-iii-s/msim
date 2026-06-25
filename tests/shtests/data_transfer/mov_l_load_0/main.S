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
    mov         #4, r0
    mov.l       data_address, r1
    mov.l       @(r0,r1), r2
    cpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000000