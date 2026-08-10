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
    mov.l   data_addr_1, r0
    // the byte is 0xFF, so the result should be 0xF0
    xor.b   #0xF, @(r0, gbr)
    mov     #1, r1
    ldc     r1, gbr
    // the byte is 0xFF, so the result should be 0xF0
    xor.b   #0xF, @(r0, gbr)
    mov     #2, r1
    ldc     r1, gbr
    // the byte is 0xFF, so the result should be 0xF0
    xor.b   #0xF, @(r0, gbr)
    mov     #3, r1
    ldc     r1, gbr
    // the byte is 0xFF, so the result should be 0xF0
    xor.b   #0xF, @(r0, gbr)
    mov.l   @r0, r2
    cpu_dump
    ehalt

.align 2
data_addr_1:
    .long   0xF0000000