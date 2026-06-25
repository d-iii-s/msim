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
    mov     #16, r0
    mov     #2, r1
    // Should not set the Q-bit nor the T-bit
    div0s   r1, r0
    cpu_dump
    // Should set the T-bit
    div1    r1, r0
    cpu_dump
    ehalt