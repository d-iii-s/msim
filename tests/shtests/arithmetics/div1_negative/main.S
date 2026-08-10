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
    mov     #-16, r0
    mov     #2, r1
    // Should set both the Q-bit and the T-bit
    div0s   r1, r0
    cpu_dump
    // Should clear the T-bit and set the Q-bit
    div1    r1, r0
    cpu_dump
    ehalt