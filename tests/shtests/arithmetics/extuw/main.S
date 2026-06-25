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
    mov.l   pos_val, r0
    extu.w  r0, r1
    cpu_dump
    mov.l   neg_val, r0
    extu.w  r0, r1
    cpu_dump
    ehalt

.align 2
pos_val:
    .long   0x00007FFF

neg_val:
    .long   0x12348000