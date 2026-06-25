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
    mov.l   val_1, r0
    or     #0xFF, r0
    cpu_dump
    ehalt

.align 2
val_1:
    .long   0xF0F0F0F0