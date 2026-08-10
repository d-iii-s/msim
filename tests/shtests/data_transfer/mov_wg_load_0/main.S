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
    mov.l       data_address, r1
    ldc         r1, gbr
    mov.w       @(2,GBR), r0
    cpu_dump
    mov.w       @(4,GBR), r0
    cpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000000