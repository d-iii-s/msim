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
    mov.l   val, r0
    mov.l   val_2, r1
    clrt
    // T-bit should be set as the lowest bit is 1 and the output value should be 0x7FFFFFFF
    rotcr   r0
    cpu_dump
    // T-bit should be cleared and the output value should be 0xFFFFFFFF
    rotcr   r1
    cpu_dump
    ehalt

.align 2
val:
    .long   0xFFFFFFFF

val_2:
    .long   0xFFFFFFFE