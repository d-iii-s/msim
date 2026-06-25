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
    mov.l       val_1, r1
    mov.w       r1, @-r0
    mov.l       val_2, r1
    mov.w       r1, @-r0
    mov.w       @r0+, r2
    mov.w       @r0+, r3
    cpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000004

val_1:
    .long       0xAAAA

val_2:
    .long       0x1234