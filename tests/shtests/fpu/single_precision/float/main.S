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
    mov         #-1, r0
    lds         r0, fpul
    float       fpul, fr0
    fpu_dump
    ehalt
