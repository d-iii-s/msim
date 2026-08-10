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
    mov.l   data_address_1, r0
    mov.l   data_address_2, r1
    mac.l   @r0+, @r1+
    cpu_dump
    ehalt

.align 2
data_address_1:
    .long 0xF0000000

data_address_2:
    .long 0xF0000004