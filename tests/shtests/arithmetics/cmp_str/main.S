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
    mov.l       val_1, r0
    mov.l       val_2, r1
    mov.l       val_2, r2
    mov.l       val_3, r3
    cmp/str     r1, r2
    cpu_dump
    cmp/str     r0, r1
    cpu_dump
    cmp/str     r2, r3
    cpu_dump
    ehalt


.align 2
val_1:
    .long   0x12345678

val_2:
    .long   0xFFFFFFFF

val_3:
    .long   0xAAFFAAAA