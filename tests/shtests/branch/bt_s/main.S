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
    clrt
    bt/s        should_skip
    sett
    bt/s        should_take
    // Executed because of the delayed branch
    mov         #1, r2

should_skip:
    mov         #1, r0

should_take:
    mov         #-1, r1
    cpu_dump
    ehalt