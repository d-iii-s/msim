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
    mov.l       data_address, r1
    mov.l       data_address, r2
    add         #4, r2
    ldc         r1, gbr
    mov.l       @r2, r3
    cpu_dump
    mov.l       r0,@(4,gbr)
    mov.l       @r2, r3
    cpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000000

val:
    .long       0x98a908ca

