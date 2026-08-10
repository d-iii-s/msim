#define illegal .word 0xffff
#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x10
    .long 0x90000000     /* Illegal instruction handler */

.section .text
    .org 0x400
_start:
    illegal
    cpu_dump
    ehalt