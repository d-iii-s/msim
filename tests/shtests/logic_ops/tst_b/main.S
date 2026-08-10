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
    mov     #2, r1
    ldc     r1, gbr
    // the byte in memory should be 0x0F, so the T-bit should be set after this
    tst.b   #0xF0, @(r0, gbr)
    cpu_dump
    mov     #3, r1
    ldc     r1, gbr
    // the byte in memory should be 0xFF, so the T bit should not be set after this
    tst.b   #0xFF, @(r0, gbr)
    cpu_dump
    ehalt

.align 2
data_addr_1:
    .long   0xF0000000