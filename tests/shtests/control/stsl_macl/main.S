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
    mov.l       data_address, r0
    mov.l       val, r1
    lds         r1, macl
    sts.l       macl, @-r0
    mov.l       @r0, r2
    cpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000004

val:
    .long       0x12345678