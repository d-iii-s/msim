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
    mov     #2, r0
    mov     #16, r1
    //  Should not set the T-bit
    div0s   r0, r1
    cpu_dump
    mov     #-16, r1
    // Should set the T-bit and the Q-bit
    div0s   r0, r1
    cpu_dump
    ehalt