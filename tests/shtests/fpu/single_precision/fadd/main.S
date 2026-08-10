#define ehalt .word 0x8200
#define fpu_dump .word 0x8202

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

.section .text
    .org 0x400
_start:
    mov.l       data_address, r0
    fmov.s      @r0, fr0
    fldi1       fr1
    fadd        fr0,fr1
    fpu_dump
    ehalt

.align 2
data_address:
    .long       0xF0000000