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
    mov.b       @r0, r2
    cpu_dump
    mov.b       r1, @r0
    mov.b       @r0, r2
    cpu_dump
    ehalt

data_address:
    .long       0xF0000000

val:
    .long       0x7F