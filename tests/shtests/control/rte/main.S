#define ehalt .word 0x8200
#define cpu_dump .word 0x8201

.section .vectors, "a"
    .org 0x0
    .long _start         /* Power on reset - PC */

    .org 0x4
    .long 0xFFFF8000     /* Power on reset - SP */

    .org 0x80
    .long 0x480          /* TRAPA vector number 1 */

.section .text
    .org 0x400
_start:
    trapa       #0x20
    nop
    cpu_dump
    ehalt

.section .text
    .org 0x480
    mov         #1, r0
    rte
    nop

